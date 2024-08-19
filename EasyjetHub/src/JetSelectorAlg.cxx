/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Frederic Renner

#include "JetSelectorAlg.h"

namespace Easyjet
{
  JetSelectorAlg ::JetSelectorAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator) { }

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
    if (!m_PCBT.empty()) {
      ATH_CHECK (m_PCBT.initialize(m_systematicsList, m_inHandle));
    }

    for(int i=0; i<m_bjetAmount; i++){
      std::string index = std::to_string(i+1);
      CP::SysWriteDecorHandle<bool> whandle{"isbjet"+index+"_%SYS%", this};
      m_leadBranches.emplace("isbjet"+index, whandle);
      ATH_CHECK(m_leadBranches.at("isbjet"+index).initialize(m_systematicsList, m_inHandle));
    }

    ANA_CHECK (m_isSelectedJet.initialize (m_systematicsList, m_inHandle));

    if(m_checkOR) ATH_CHECK (m_passesOR.initialize(m_systematicsList, m_inHandle));

    ATH_CHECK (m_relativeDeltaRToVRJet.initialize(m_systematicsList, m_inHandle));

    if(m_useJVT) ATH_CHECK (m_jvtselection.initialize(m_systematicsList, m_inHandle));
    if(m_useFJVT) ATH_CHECK (m_fjvtselection.initialize(m_systematicsList, m_inHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    // check that m_PCBTsort and m_PCBT are both set or not set
    if (m_PCBTsort && m_PCBT.empty()) {
      ATH_MSG_ERROR("PCBT sorting configured but no PCBT decorator given!");
      return StatusCode::FAILURE;
    }

    // check that pTsort and PCBTSort are not both set
    if(m_pTsort && m_PCBTsort){
      ATH_MSG_ERROR("pT sorting and PCBT sorting are configured simultaneously!");
      return StatusCode::FAILURE;
    }

    // check if m_isBtag is empty and m_selectBjet is true 
    if(m_isBtag.empty() && m_selectBjet){
      ATH_MSG_ERROR("btag wp is empty but selectBjet is true");
      return StatusCode::FAILURE;
    }

    // check if if m_bjetAmount is positive and there is no WPgiven
    if(m_bjetAmount>0 && m_isBtag.empty()){
      ATH_MSG_ERROR("required bjet amount is positive but btag wp is empty");
      return StatusCode::FAILURE;
    }

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
      bool PCBTaggiven = !m_PCBT.empty();

      // define jets_pcbt as a map of jets to their pcbt
      std::map<const xAOD::IParticle *, int> workContainer_pcbt;
      
      // loop over jets
      for (const xAOD::Jet *jet : *inContainer)
      {
        // jvt selection
        if(m_useJVT){
          bool jvt = m_jvtselection.get(*jet, sys);
          if(m_useFJVT) jvt &= m_fjvtselection.get(*jet, sys);
          if ( !jvt ) continue;
        }

        // skip OR jets
        if( m_checkOR ){
          bool passesOR = m_passesOR.get(*jet, sys);
          if ( !passesOR ) continue;
        }

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
          continue;

        bool isSelected = false;
        // select btagging wp if given and select_bjet flag is on. if not given always push back
	// check if want to apply btagging
        if (!m_selectBjet || (m_isBtag.get(*jet, sys) && std::abs(jet->eta())<2.5)) 
	{
	  workContainer->push_back(jet);
	  isSelected = true;
	}
        if (PCBTaggiven) workContainer_pcbt[jet] = m_PCBT.get(*jet, sys);
	m_isSelectedJet.set(*jet, isSelected, sys);
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
      if (nJets < m_truncateAtAmount) nKeep = nJets;
      else nKeep = m_truncateAtAmount;
      
      if (m_pTsort || m_PCBTsort) {
        // if we give -1, sort the whole container
        if (m_truncateAtAmount == -1) nKeep = nJets;
        std::partial_sort(
          workContainer->begin(), // Iterator from which to start sorting
          workContainer->begin() + nKeep, // Use begin + N to sort first N
          workContainer->end(), // Iterator marking the end of range to sort
            [&](const xAOD::IParticle *left, const xAOD::IParticle *right) {
              // sort by pT if pTsort or as tiebreaker if PCBTSort
              if (m_pTsort || (m_PCBTsort && workContainer_pcbt[left] == workContainer_pcbt[right]))
                return left->pt() > right->pt();
              // else sort by PCBT
              else
                return workContainer_pcbt[left] > workContainer_pcbt[right];
            });
        // keep only the requested amount
        workContainer->erase(workContainer->begin() + nKeep, workContainer->end());
      }

      //lead/sublead bjet
      if(m_bjetAmount > 0){
         int njet = 0;
         for (const xAOD::Jet *jet : *workContainer) {
            if (WPgiven &&  m_isBtag.get(*jet, sys) && std::abs(jet->eta())<2.5){
               njet++;
               m_leadBranches.at("isbjet"+std::to_string(njet)).set(*jet, true, sys);
            }
            if ( njet == m_bjetAmount ) break;
         }
      }

      // Write to eventstore
      ATH_CHECK(m_outHandle.record(std::move(workContainer), sys));   
    }

    return StatusCode::SUCCESS;
  }
}
