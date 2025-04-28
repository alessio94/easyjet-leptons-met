/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TriggerSFAlg.h"

#include "AthContainers/AuxElement.h"
#include <AthContainers/ConstDataVector.h>

#include "TriggerUtils.h"

#include "TLorentzVector.h"

namespace HHBBTT
{
  TriggerSFAlg::TriggerSFAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {

  }

  StatusCode TriggerSFAlg::initialize()
  {

    // Read syst-aware input handles
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_tauHandle.initialize(m_systematicsList));
    ATH_CHECK (m_bbttTauHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_bbttEleHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_bbttMuonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    for(const auto& trig : m_eleTrigSF){
      m_eleTriggerSF.emplace
	(trig, CP::SysReadDecorHandle<float>("el_trigEffSF_"+trig+"_%SYS%", this));
      ATH_CHECK (m_eleTriggerSF.at(trig).initialize(m_systematicsList, m_electronHandle));
    }

    for(const auto& trig : m_muonTrigSF){
      m_muonTriggerSF.emplace
	(trig, CP::SysReadDecorHandle<float>("muon_trigEffSF_"+trig+"_%SYS%", this));
      ATH_CHECK (m_muonTriggerSF.at(trig).initialize(m_systematicsList, m_muonHandle));
    }

    for(const auto& trig : m_tauTrigSF){
      m_tauTriggerSF.emplace
	(trig, CP::SysReadDecorHandle<float>("tau_trigEffSF_"+trig+"_%SYS%", this));
      ATH_CHECK (m_tauTriggerSF.at(trig).initialize(m_systematicsList, m_tauHandle));
    }

    ATH_CHECK (m_selected_el.initialize(m_systematicsList, m_bbttEleHandle));
    ATH_CHECK (m_selected_mu.initialize(m_systematicsList, m_bbttMuonHandle));
    ATH_CHECK (m_selected_tau.initialize(m_systematicsList, m_bbttTauHandle));

    ATH_CHECK(m_year.initialize(m_systematicsList, m_eventHandle));

    for(const auto& [runBool, name] : m_runBooleans) {
      SG::ReadDecorHandleKey<xAOD::EventInfo> deco;
      deco = "EventInfo." + name;
      m_runBooleans_key.emplace(runBool, deco);
      ATH_CHECK(m_runBooleans_key.at(runBool).initialize());
    }

    for (auto& [key, value] : m_boolnames) {
      CP::SysReadDecorHandle<bool> whandle{value+"_%SYS%", this};
      m_categoryBranches.emplace(key, whandle);
      ATH_CHECK(m_categoryBranches.at(key).initialize(m_systematicsList, m_eventHandle));
    };

    ATH_CHECK(m_eventTriggerSF.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode TriggerSFAlg::execute()
  {

    runBoolReadDecoMap runBoolDecos;
    for (const auto& [runBool, key] : m_runBooleans_key){
      runBoolDecos.emplace(runBool, key);
    }

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {

      // Retrieve inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_bbttMuonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_bbttEleHandle.retrieve (electrons, sys));

      const xAOD::TauJetContainer *taus = nullptr;
      ANA_CHECK (m_bbttTauHandle.retrieve (taus, sys));

      m_eventTriggerSF.set(*event, -999., sys);

      // selected leptons ;
      const xAOD::Electron* ele0 = nullptr;

      for(const xAOD::Electron* electron : *electrons) {
        if (m_selected_el.get(*electron, sys)){
          if(!ele0){
            ele0 = electron;
            break;
          }
        }
      }

      const xAOD::Muon* mu0 = nullptr;
      for(const xAOD::Muon* muon : *muons) {
        if (m_selected_mu.get(*muon, sys)){
          if(!mu0){
            mu0 = muon;
            break;
          }
        }
      }

      //selected tau
      const xAOD::TauJet* tau0 = nullptr;
      const xAOD::TauJet* tau1 = nullptr;

      for(const xAOD::TauJet* tau : *taus) {
        if (m_selected_tau.get(*tau, sys)){
          if(!tau0) tau0 = tau;
          else if(!tau1){
            tau1 = tau;
            break;
          }
        }
      }
      std::vector<const xAOD::TauJet*> sel_taus = {tau0, tau1};

      computeTriggerSF(event, sys, runBoolDecos, ele0, mu0, tau0, tau1);
    }

    return StatusCode::SUCCESS;
  }

  void TriggerSFAlg::computeTriggerSF
  (const xAOD::EventInfo *event, const CP::SystematicSet& sys,
   const runBoolReadDecoMap& runBoolDecos,
   const xAOD::Electron* ele0, const xAOD::Muon* mu0,
   const xAOD::TauJet* tau0, const xAOD::TauJet* tau1){
    float triggerSF = 1.;
    int year = m_year.get(*event, sys);

    if(m_categoryBranches.at(HHBBTT::pass_baseline_SLT).get(*event, sys)){
      if(ele0){
	std::string single_ele_SF_path;
	getSingleEleTriggers(year, event, runBoolDecos, single_ele_SF_path);
	if(!single_ele_SF_path.empty())
	  triggerSF *= m_eleTriggerSF.at(single_ele_SF_path).get(*ele0, sys);
      }
      else if(mu0){
	std::string single_mu_SF_path;
	getSingleMuTriggers(year, event, runBoolDecos, single_mu_SF_path);
	if(!single_mu_SF_path.empty())
	  triggerSF *= m_muonTriggerSF.at(single_mu_SF_path).get(*mu0, sys);
      }
    }

    else if(m_categoryBranches.at(HHBBTT::pass_baseline_LTT).get(*event, sys)){
      if(ele0){
	std::pair<std::string, std::string> ele_tau_SF_path;
	getEleTauTriggers(year, event, runBoolDecos, ele_tau_SF_path);
	if(!ele_tau_SF_path.first.empty())
	  triggerSF *= m_eleTriggerSF.at(ele_tau_SF_path.first).get(*ele0, sys);
	if(!ele_tau_SF_path.second.empty())
	  triggerSF *= m_tauTriggerSF.at(ele_tau_SF_path.second).get(*tau0, sys);
      }
      else if(mu0){
	std::pair<std::string, std::string> mu_tau_SF_path;
	getMuTauTriggers(year, event, runBoolDecos, mu_tau_SF_path);
	if(!mu_tau_SF_path.first.empty())
	  triggerSF *= m_muonTriggerSF.at(mu_tau_SF_path.first).get(*mu0, sys);
	if(!mu_tau_SF_path.second.empty())
	  triggerSF *= m_tauTriggerSF.at(mu_tau_SF_path.second).get(*tau0, sys);
      }
    }

    else if(m_categoryBranches.at(HHBBTT::pass_baseline_STT).get(*event, sys)){
      std::string single_tau_SF_path;
      getSingleTauTriggers(year, event, runBoolDecos, single_tau_SF_path);
      if(!single_tau_SF_path.empty())
	triggerSF *= m_tauTriggerSF.at(single_tau_SF_path).get(*tau0, sys);
    }

    else if(m_categoryBranches.at(HHBBTT::pass_baseline_DTT).get(*event, sys)){
      std::pair<std::string, std::string> di_tau_SF_path;
      getDiTauTriggers(year, event, runBoolDecos, di_tau_SF_path);

      if(!di_tau_SF_path.first.empty())
	triggerSF *= m_tauTriggerSF.at(di_tau_SF_path.first).get(*tau0, sys);
      if(!di_tau_SF_path.second.empty())
	triggerSF *= m_tauTriggerSF.at(di_tau_SF_path.second).get(*tau1, sys);
    }

    m_eventTriggerSF.set(*event, triggerSF, sys);
  }
}
