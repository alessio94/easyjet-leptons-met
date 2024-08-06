/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TauSelectorAlg.h"

namespace Easyjet
{
  TauSelectorAlg::TauSelectorAlg(const std::string &name,
                                       ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator) { }

  StatusCode TauSelectorAlg::initialize()
  {

    // Read syst-aware input/output handles
    ATH_CHECK (m_inHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK (m_outHandle.initialize(m_systematicsList));

    if(m_keepAntiTaus){
      m_IDTau = CP::SysReadDecorHandle<char>("isIDTau", this);
      m_antiTau = CP::SysReadDecorHandle<char>("isAntiTau", this);
    }
    ATH_CHECK (m_IDTau.initialize(m_systematicsList, m_inHandle, SG::AllowEmpty));
    ATH_CHECK (m_antiTau.initialize(m_systematicsList, m_inHandle, SG::AllowEmpty));

    if (m_tauAmount > 0)
    {
      for (int i = 0; i < m_tauAmount; i++)
      {
        std::string index = std::to_string(i + 1);
        CP::SysWriteDecorHandle<bool> whandle{"isTau" + index + "_%SYS%", this};
        m_leadBranches.emplace("isTau" + index, whandle);
        ATH_CHECK(m_leadBranches.at("isTau" + index).initialize(m_systematicsList, m_inHandle));
      };
    }

    ANA_CHECK (m_isSelectedTau.initialize(m_systematicsList, m_inHandle));

    ATH_CHECK (m_passesOR.initialize(m_systematicsList, m_inHandle));

    if(m_tightTauWP.empty()) m_tightTauWP = m_looseTauWP;

    if(m_isMC){
      m_tau_recoSF = CP::SysReadDecorHandle<float>("tau_Reco_effSF_"+m_tightTauWP+"_%SYS%", this);
      m_tau_IDSF = CP::SysReadDecorHandle<float>("tau_ID_effSF_"+m_tightTauWP+"_%SYS%", this);
      if(m_tightTauWP.value().find("noeleid")==std::string::npos){
        m_tau_eVetoFakeTauSF = CP::SysReadDecorHandle<float>("tau_EvetoFakeTau_effSF_"+m_tightTauWP+"_%SYS%", this);
        m_tau_eVetoTrueTauSF = CP::SysReadDecorHandle<float>("tau_EvetoTrueTau_effSF_"+m_tightTauWP+"_%SYS%", this);
      }
      m_tau_SF_out = CP::SysWriteDecorHandle<float>("tau_effSF_"+m_tightTauWP+"_%SYS%", this);
    }
    ATH_CHECK (m_tau_recoSF.initialize(m_systematicsList, m_inHandle, SG::AllowEmpty));
    ATH_CHECK (m_tau_IDSF.initialize(m_systematicsList, m_inHandle, SG::AllowEmpty));
    ATH_CHECK (m_tau_eVetoFakeTauSF.initialize(m_systematicsList, m_inHandle, SG::AllowEmpty));
    ATH_CHECK (m_tau_eVetoTrueTauSF.initialize(m_systematicsList, m_inHandle, SG::AllowEmpty));
    ATH_CHECK (m_tau_SF_out.initialize(m_systematicsList, m_outHandle, SG::AllowEmpty));

    for(const auto& trig : m_tauTrigSF){
      m_tauTriggerSF_in.emplace_back("tau_trigEffSF_"+trig+"_%SYS%", this);
      m_tauTriggerSF_out.emplace_back("tau_trigEffSF_"+trig+"_%SYS%", this);
    }

    for(auto& handle : m_tauTriggerSF_in)
      ATH_CHECK(handle.initialize(m_systematicsList, m_inHandle));
    for(auto& handle : m_tauTriggerSF_out)
      ATH_CHECK(handle.initialize(m_systematicsList, m_outHandle));

    m_select_loose_in = CP::SysReadDecorHandle<char>("baselineSelection_"+m_looseTauWP+"_%SYS%", this);
    m_select_tight_in = CP::SysReadDecorHandle<char>("baselineSelection_"+m_tightTauWP+"_%SYS%", this);
    m_select_out = CP::SysWriteDecorHandle<char>("baselineSelection_"+m_tightTauWP+"_%SYS%", this);
    ATH_CHECK (m_select_loose_in.initialize(m_systematicsList, m_inHandle));
    ATH_CHECK (m_select_tight_in.initialize(m_systematicsList, m_inHandle));
    ATH_CHECK (m_select_out.initialize(m_systematicsList, m_outHandle));

    // Initialise syst-aware input/output decorators 
    ATH_CHECK (m_nSelPart.initialize(m_systematicsList, m_eventHandle));

    // Initialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());    

    return StatusCode::SUCCESS;
  }

  StatusCode TauSelectorAlg::execute()
  {

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector()) {

      // Retrive inputs
      const xAOD::TauJetContainer *inContainer = nullptr;
      ANA_CHECK (m_inHandle.retrieve (inContainer, sys));      

      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      // Setup output 
      auto workContainer =
        std::make_unique<ConstDataVector<xAOD::TauJetContainer> >(
            SG::VIEW_ELEMENTS);
    
      // loop over taus 
      for (const xAOD::TauJet *tau : *inContainer) {

        // selected taus for systematics
        m_isSelectedTau.set(*tau, false, sys);

        if(!m_select_loose_in.get(*tau,sys)) continue;

        // If not ID tau nor anti tau, skip
        if(m_keepAntiTaus){
          bool keep = m_IDTau.get(*tau, sys) || m_antiTau.get(*tau, sys);
          if( !keep ) continue;
        }

        // If not passing OR, skip
        if( m_checkOR ){
          bool passesOR = m_passesOR.get(*tau, sys);
          if ( !passesOR ) continue;
        }
	
        if (tau->pt() < m_minPt)
          continue;
    
        float this_tau_eta_abs = std::abs(tau->eta());
        if ((this_tau_eta_abs > m_minEtaVeto &&
            this_tau_eta_abs < m_maxEtaVeto) ||
            (this_tau_eta_abs > m_maxEta))
          continue;

        // For some reason this decoration needs to be explicitly copied
        if(m_isMC){
          float SF = m_tau_recoSF.get(*tau,sys);
          SF *=  m_tau_IDSF.get(*tau,sys);
          if(m_tightTauWP.value().find("noeleid")==std::string::npos){
            SF *= m_tau_eVetoFakeTauSF.get(*tau,sys);
            SF *= m_tau_eVetoTrueTauSF.get(*tau,sys);
          }
          m_tau_SF_out.set(*tau, SF, sys);
          for(unsigned int i=0; i<m_tauTrigSF.size(); i++){
            m_tauTriggerSF_out[i].set(*tau, m_tauTriggerSF_in[i].get(*tau, sys), sys);
          }
        }
        m_select_out.set(*tau, m_select_tight_in.get(*tau,sys), sys);

        // If cuts are passed, save the object
        workContainer->push_back(tau);
        m_isSelectedTau.set(*tau, true, sys);
      }

      int nTaus = workContainer->size();      
      m_nSelPart.set(*event, nTaus, sys);
      
      // if we have less than the requested nr, empty the workcontainer to write
      // defaults/return empty container
      if (nTaus < m_minimumAmount) {
        workContainer->clear();
        nTaus = 0;
      }
      
      // sort and truncate
      int nKeep;
      if (nTaus < m_truncateAtAmount) nKeep = nTaus;
      else nKeep = m_truncateAtAmount;
      
      if (m_pTsort) {
        // if we give -1, sort the whole container
        if (m_truncateAtAmount == -1)  nKeep = nTaus;
    
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

      //lead/sublead tau
      if(m_tauAmount > 0){
        int nTau = 0;
        for (const xAOD::TauJet *tau : *workContainer) {
          nTau++;
          m_leadBranches.at("isTau"+std::to_string(nTau)).set(*tau, true, sys);
          if ( nTau == m_tauAmount ) break;
        }
      }
    
      // Write to eventstore
      ATH_CHECK(m_outHandle.record(std::move(workContainer), sys));   
    }

    return StatusCode::SUCCESS;
  }
}

