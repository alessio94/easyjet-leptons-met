/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Minori Fujimoto

#include "MuonSelectorAlg.h"

namespace Easyjet
{
  MuonSelectorAlg::MuonSelectorAlg(const std::string &name,
                                       ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator) { }

  StatusCode MuonSelectorAlg::initialize()
  {

    // Read syst-aware input/output handles
    ATH_CHECK (m_inHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK (m_outHandle.initialize(m_systematicsList));

    // Intialise syst-aware input/output decorators    
    ATH_CHECK (m_nSelPart.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK (m_passesOR.initialize(m_systematicsList, m_inHandle));

    for(const auto& wp : m_muWPNames){
      // Scale factors
      m_mu_recoSF.emplace_back(m_isMC ? "muon_reco_effSF_"+wp+"_%SYS%" : "", this);
      m_mu_isoSF.emplace_back((m_isMC && wp.find("NonIso")==std::string::npos) ?
            "muon_isol_effSF_"+wp+"_%SYS%" : "", this);
      m_mu_TTVASF.emplace_back((m_isMC && m_doTTVA) ?
            "muon_TTVA_effSF_"+wp+"_%SYS%" : "", this);
      m_mu_SF.emplace_back(m_isMC ? "muon_effSF_"+wp+"_%SYS%" : "", this);

      // Select flags
      m_select_in.emplace_back("baselineSelection_"+wp+"_%SYS%", this);
      m_select_out.emplace_back("baselineSelection_"+wp+"_%SYS%", this);
    }

    for(auto& handle : m_mu_recoSF)
      ATH_CHECK(handle.initialize(m_systematicsList, m_inHandle, SG::AllowEmpty));
    for(auto& handle : m_mu_isoSF)
      ATH_CHECK(handle.initialize(m_systematicsList, m_inHandle, SG::AllowEmpty));
    for(auto& handle : m_mu_TTVASF)
      ATH_CHECK(handle.initialize(m_systematicsList, m_inHandle, SG::AllowEmpty));
    for(auto& handle : m_mu_SF)
      ATH_CHECK(handle.initialize(m_systematicsList, m_outHandle, SG::AllowEmpty));
    for(auto& handle : m_select_in)
      ATH_CHECK(handle.initialize(m_systematicsList, m_inHandle, SG::AllowEmpty));
    for(auto& handle : m_select_out)
      ATH_CHECK(handle.initialize(m_systematicsList, m_outHandle, SG::AllowEmpty));


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
        // skip OR muons
        if ( m_checkOR ){
          bool passesOR = m_passesOR.get(*muon, sys);
          if ( !passesOR ) continue;
        }

        // pT and eta cuts
        if (muon->pt() < m_minPt || std::abs(muon->eta()) > m_maxEta)
          continue;

        // For some reason this decoration needs to be explicitly copied
        for(unsigned int i=0; i<m_muWPNames.size(); i++){
          std::string wp = m_muWPNames[i];
          if(m_isMC){
            float SF = m_mu_recoSF[i].get(*muon,sys);
            if(wp.find("NonIso")==std::string::npos) SF *= m_mu_isoSF[i].get(*muon,sys);
            if(m_doTTVA) SF *= m_mu_TTVASF[i].get(*muon,sys);
            m_mu_SF[i].set(*muon, SF, sys);
          }
          m_select_out[i].set(*muon, m_select_in[i].get(*muon,sys), sys);
        }

        // If cuts are passed, save the object
        workContainer->push_back(muon);
      }
      
      int nMuons = workContainer->size();      
      m_nSelPart.set(*event, nMuons, sys);

      // if we have less than the requested nr, empty the workcontainer to write
      // defaults/return empty container
      if (nMuons < m_minimumAmount) {
        workContainer->clear();
        nMuons = 0;
      }
      
      // sort and truncate
      int nKeep;
      if (nMuons < m_truncateAtAmount) nKeep = nMuons;
      else nKeep = m_truncateAtAmount;
      
      if (m_pTsort) {
        // if we give -1, sort the whole container
        if (m_truncateAtAmount == -1) nKeep = nMuons;
        
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

