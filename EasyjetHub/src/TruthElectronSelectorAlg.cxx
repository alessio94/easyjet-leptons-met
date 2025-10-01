/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "TruthElectronSelectorAlg.h"
#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteHandle.h>
#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

namespace Easyjet
{
  TruthElectronSelectorAlg::TruthElectronSelectorAlg(const std::string &name,
                                       ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator) { }

  StatusCode TruthElectronSelectorAlg::initialize()
  {
    // Read input/output handles
    ATH_CHECK (m_inHandleKey.initialize());
    ATH_CHECK (m_eventHandleKey.initialize());
    ATH_CHECK (m_outHandleKey.initialize());

    // Intialise input/output decorators    
    ATH_CHECK (m_nSelPartKey.initialize());

    ATH_CHECK (m_isSelectedElectronKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode TruthElectronSelectorAlg::execute()
  {

    // Retrieve inputs
    SG::ReadHandle<xAOD::TruthParticleContainer> inContainer(m_inHandleKey);
    SG::ReadHandle<xAOD::TruthEventContainer> events(m_eventHandleKey);

    // Setup output 
    auto workContainer =
    std::make_unique<ConstDataVector<xAOD::TruthParticleContainer> >(
        SG::VIEW_ELEMENTS);

        
    SG::WriteDecorHandle<xAOD::TruthParticleContainer, bool> m_isSelectedElectron(m_isSelectedElectronKey);
    SG::WriteDecorHandle<xAOD::TruthEventContainer, int> m_nSelPart(m_nSelPartKey);

    // loop over electrons 
    for (const xAOD::TruthParticle *electron : *inContainer)
    {
      m_isSelectedElectron(*electron) = false;


      if (electron->pt() < m_minPt || std::abs(electron->eta()) > m_maxEta)
          continue;
      
      // If cuts are passed, save the object
      workContainer->push_back(electron);
      m_isSelectedElectron(*electron) = true;
    }
    
    int nElectrons = workContainer->size();
    const xAOD::TruthEvent *event = events->at(0);
    m_nSelPart(*event) = nElectrons;

    // if we have less than the requested nr, empty the workcontainer to write
    // defaults/return empty container
    if (nElectrons < m_minimumAmount)
    {
    workContainer->clear();
    nElectrons = 0;
    }
    
    // sort and truncate
    int nKeep;
    if (nElectrons < m_truncateAtAmount || m_truncateAtAmount == -1) nKeep = nElectrons; // if we give -1, sort the whole container
    else nKeep = m_truncateAtAmount;
    
    if (m_pTsort){
      std::partial_sort(
          workContainer->begin(), // Iterator from which to start sorting
          workContainer->begin() + nKeep, // Use begin + N to sort first N
          workContainer->end(), // Iterator marking the end of range to sort
          [](const xAOD::TruthParticle *left, const xAOD::TruthParticle *right)
          { return left->pt() > right->pt(); }); // lambda function here just
                                                  // handy, could also be another
                                                  // function that returns bool

      // keep only the requested amount
      workContainer->erase(workContainer->begin() + nKeep,
              workContainer->end());
    }
    
    // Write to eventstore
    SG::WriteHandle<ConstDataVector<xAOD::TruthParticleContainer>> m_outHandle(m_outHandleKey);
    ATH_CHECK(m_outHandle.record(std::move(workContainer)));   
    
    return StatusCode::SUCCESS;
  }
}