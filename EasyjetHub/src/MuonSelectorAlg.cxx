/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Minori Fujimoto

#include "MuonSelectorAlg.h"
#include "AthContainers/AuxElement.h"
#include <xAODMuon/MuonContainer.h>
#include "FourMomUtils/xAODP4Helpers.h"

namespace Easyjet
{
  MuonSelectorAlg::MuonSelectorAlg(const std::string &name,
                                       ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("minPt", m_minPt);
    declareProperty("maxEta", m_maxEta);
    declareProperty("minimumAmount", m_minimumAmount);
    declareProperty("truncateAtAmount", m_truncateAtAmount);
    declareProperty("pTsort", m_pTsort);
  }

  StatusCode MuonSelectorAlg::initialize()
  {

    // Read syst-aware input/output handles
    ATH_CHECK (m_inHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK (m_outHandle.initialize(m_systematicsList));

    // Intialise syst-aware input/output decorators    
    ATH_CHECK (m_nSelPart.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());    
    return StatusCode::SUCCESS;
  }

  StatusCode MuonSelectorAlg::execute()
  {
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector()) {

      // Retrive inputs
      const xAOD::MuonContainer *inContainer = nullptr;
      ANA_CHECK (m_inHandle.retrieve (inContainer, sys));      

      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      // Setup output 
      auto workContainer =
        std::make_unique<ConstDataVector<xAOD::MuonContainer> >(
            SG::VIEW_ELEMENTS);
     
      // loop over muons 
      for (const xAOD::Muon *muon : *inContainer)
	{
	  // cuts
	  if (muon->pt() < m_minPt || std::abs(muon->eta()) > m_maxEta)
	    continue;
	  
	  // If cuts are passed, save the object
	  workContainer->push_back(muon);
	}
      
      int nMuons = workContainer->size();      
      m_nSelPart.set(*event, nMuons, sys);

      // if we have less than the requested nr, empty the workcontainer to write
      // defaults/return empty container
      if (nMuons < m_minimumAmount)
	{
	  workContainer->clear();
	  nMuons = 0;
	}
      
      // sort and truncate
      int nKeep;
      if (nMuons < m_truncateAtAmount)
	{
	  nKeep = nMuons;
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
	      nKeep = nMuons;
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
    
      // Write to eventstore
      ATH_CHECK(m_outHandle.record(std::move(workContainer), sys));   
    }

    return StatusCode::SUCCESS;
  }
}

