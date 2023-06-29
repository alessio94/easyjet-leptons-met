/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Minori Fujimoto

#include "ElectronSelectorAlg.h"
#include "AthContainers/AuxElement.h"
#include <AthContainers/ConstDataVector.h>
#include <xAODEgamma/ElectronContainer.h>
#include "FourMomUtils/xAODP4Helpers.h"

namespace Easyjet
{
  ElectronSelectorAlg::ElectronSelectorAlg(const std::string &name,
                                       ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("minPt", m_minPt);
    declareProperty("maxEta", m_maxEta);
    declareProperty("minimumAmount", m_minimumAmount);
    declareProperty("truncateAtAmount", m_truncateAtAmount);
    declareProperty("pTsort", m_pTsort);
  }

  StatusCode ElectronSelectorAlg::initialize()
  {
    ATH_CHECK(m_containerInKey.initialize());
    ATH_CHECK(m_EventInfoKey.initialize());
    ATH_CHECK(m_containerOutKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode ElectronSelectorAlg::execute()
  {
    // container we read in
    SG::ReadHandle<xAOD::ElectronContainer> inContainer(m_containerInKey);
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    ATH_CHECK(inContainer.isValid());
    ATH_CHECK(eventInfo.isValid());

    // make some accessors and decorators

    SG::AuxElement::Decorator<unsigned int> nSelectedParticles_dec(
        m_containerOutKey.key() + "_n");

    // fill workContainer with "views" of the inContainer
    // see TJ's tutorial for this

    auto workContainer =
        std::make_unique<ConstDataVector<xAOD::ElectronContainer> >(
            SG::VIEW_ELEMENTS);
    
     
    // loop over electrons 
    for (const xAOD::Electron *electron : *inContainer)
    {
      // cuts
      if (electron->pt() < m_minPt || std::abs(electron->eta()) > m_maxEta)
      continue;

      // If cuts are passed, save the object
      workContainer->push_back(electron);
    }

    int nElectrons = workContainer->size();

    // decorate nr of selected particles to the eventinfo
    nSelectedParticles_dec(*eventInfo) = nElectrons;

    // if we have less than the requested nr, empty the workcontainer to write
    // defaults/return empty container
    if (nElectrons < m_minimumAmount)
    {
      workContainer->clear();
      nElectrons = 0;
    }

    // sort and truncate
    int nKeep;
    if (nElectrons < m_truncateAtAmount)
    {
      nKeep = nElectrons;
    }
    else
    {
      nKeep = m_truncateAtAmount;
    }

    if (m_pTsort)
    {
      // if we give -1, sort the whole container
      if (m_truncateAtAmount == -1)
      {
        nKeep = nElectrons;
      }
      std::partial_sort(
          workContainer->begin(), // Iterator from which to start sorting
          workContainer->begin() + nKeep, // Use begin + N to sort first N
          workContainer->end(), // Iterator marking the end of range to sort
          [](const xAOD::IParticle *left, const xAOD::IParticle *right)
      { return left->pt() > right->pt(); }); // lambda function here just
                                             // handy, could also be another
                                             // function that returns bool

      // keep only the requested amount
      workContainer->erase(workContainer->begin() + nKeep,
                           workContainer->end());
    }
    
    // write to eventstore
    SG::WriteHandle<ConstDataVector<xAOD::ElectronContainer> > Writer(
        m_containerOutKey);
    ATH_CHECK(Writer.record(std::move(workContainer)));

    return StatusCode::SUCCESS;
  }
}

