/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "TruthMuonSelectorAlg.h"
#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteHandle.h>
#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

namespace Easyjet
{
  TruthMuonSelectorAlg::TruthMuonSelectorAlg(const std::string &name,
                                       ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator) { }

  StatusCode TruthMuonSelectorAlg::initialize()
  {
    // Read input/output handles
    ATH_CHECK (m_inHandleKey.initialize());
    ATH_CHECK (m_eventHandleKey.initialize());
    ATH_CHECK (m_outHandleKey.initialize());

    // Intialise input/output decorators    
    ATH_CHECK (m_nSelPartKey.initialize());
    ATH_CHECK (m_isSelectedMuonKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode TruthMuonSelectorAlg::execute()
  {

    // Retrieve inputs
    SG::ReadHandle<xAOD::TruthParticleContainer> inContainer(m_inHandleKey);
    SG::ReadHandle<xAOD::TruthEventContainer> events(m_eventHandleKey);

    ATH_CHECK(events.isValid());
    ATH_CHECK(inContainer.isValid());

    // Setup output 
    auto workContainer =
    std::make_unique<ConstDataVector<xAOD::TruthParticleContainer> >(
        SG::VIEW_ELEMENTS);

        
    SG::WriteDecorHandle<xAOD::TruthParticleContainer, bool> m_isSelectedMuon(m_isSelectedMuonKey);
    SG::WriteDecorHandle<xAOD::TruthEventContainer, int> m_nSelPart(m_nSelPartKey);


    // loop over muons 
    for (const xAOD::TruthParticle *muon : *inContainer)
    {
      m_isSelectedMuon(*muon) = false;

      if (muon->pt() < m_minPt || std::abs(muon->eta()) > m_maxEta)
          continue;
      
      // If cuts are passed, save the object
      workContainer->push_back(muon);
      m_isSelectedMuon(*muon) = true;
    }
    
    int nMuons = workContainer->size();
    if (events->empty()) {
      ATH_MSG_ERROR("TruthEventContainer is empty!");
      return StatusCode::FAILURE;
    }
    const xAOD::TruthEvent *event = events->at(0);
    m_nSelPart(*event) = nMuons;

    // if we have less than the requested nr, empty the workcontainer to write
    // defaults/return empty container
    if (nMuons < m_minimumAmount)
    {
    workContainer->clear();
    nMuons = 0;
    }
    
    // sort and truncate
    int nKeep;
    if (nMuons < m_truncateAtAmount || m_truncateAtAmount == -1) nKeep = nMuons;     // if we give -1, sort the whole container
    else nKeep = m_truncateAtAmount;
    
    if (m_pTsort) {
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