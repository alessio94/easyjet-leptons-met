/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Frederic Renner

#include "JetSelectorAlg.h"
#include "AthContainers/AuxElement.h"
#include <xAODJet/JetContainer.h>

namespace Easyjet
{
  JetSelectorAlg ::JetSelectorAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("minPt", m_minPt);
    declareProperty("maxEta", m_maxEta);
    declareProperty("minimumAmount", m_minimumAmount);
    declareProperty("maximumAmount", m_maximumAmount=-1);
    declareProperty("truncateAtAmount", m_truncateAtAmount);
    declareProperty("pTsort", m_pTsort);
    declareProperty("removeRelativeDeltaRToVRJet",
                    m_removeRelativeDeltaRToVRJet = false);
  }

  StatusCode JetSelectorAlg ::initialize()
  {
    // Read syst-aware input/output handles
    ATH_CHECK (m_inHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK (m_outHandle.initialize(m_systematicsList));

    // Intialise syst-aware input/output decorators  
    ATH_CHECK (m_nSelPart.initialize(m_systematicsList, m_eventHandle));  
    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_inHandle));
    }
    ATH_CHECK (m_relativeDeltaRToVRJet.initialize(m_systematicsList, m_inHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());    

    return StatusCode::SUCCESS;
  }

  StatusCode JetSelectorAlg ::execute()
  {
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector()) {

      // Retrive inputs
      const xAOD::JetContainer *inContainer = nullptr;
      ANA_CHECK (m_inHandle.retrieve (inContainer, sys));      

      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      // Setup output 
      auto workContainer =
        std::make_unique<ConstDataVector<xAOD::JetContainer> >(
            SG::VIEW_ELEMENTS);

      // check if a btag wp is given
      bool WPgiven = !m_isBtag.empty();
      
      // loop over jets
      for (const xAOD::Jet *jet : *inContainer)
	{
	  // jump out if VR jets overlap
	  // recommended by ftag : Remove the event if any of your signal jets have
	  // relativeDeltaRToVRJet = radius(jet_i)/min(dR(jet_i,jet_j)) < 1.0.
	  // checks if any of the vr jets overlap	  
	  if (m_removeRelativeDeltaRToVRJet && m_relativeDeltaRToVRJet.get(*jet, sys) < 1.0)
	    {
	      workContainer->clear();
	      break;
	    }
	  // cuts
	  if (jet->pt() < m_minPt || std::abs(jet->eta()) > m_maxEta)
	    {
	      continue;
	    }
	  // select btagging wp
	  if (WPgiven)
	    {
	      if (m_isBtag.get(*jet, sys))
		{
		  workContainer->push_back(jet);
		}
	    }
	  // if no btag wp is given take all
	  else
	    {
	      workContainer->push_back(jet);
	    }
	}
      
      int nJets = workContainer->size();
      m_nSelPart.set(*event, nJets, sys);
      
      // if we have more or less than the requested numbers, empty the workcontainer to write
      // defaults/return empty container
      bool over_maximum = m_maximumAmount > 0 && nJets > m_maximumAmount;
      if ( nJets < m_minimumAmount || over_maximum)
	{
	  workContainer->clear();
	  nJets = 0;
	}
      
      // sort and truncate
      int nKeep;
      if (nJets < m_truncateAtAmount)
	{
	  nKeep = nJets;
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
	      nKeep = nJets;
	    }
	  std::partial_sort(
             workContainer->begin(), // Iterator from which to start sorting
	     workContainer->begin() + nKeep, // Use begin + N to sort first N
	     workContainer->end(), // Iterator marking the end of range to sort
	     [](const xAOD::IParticle *left, const xAOD::IParticle *right)
	     {
	       return left->pt() > right->pt();
	     }); // lambda function here just handy, could also be another
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
