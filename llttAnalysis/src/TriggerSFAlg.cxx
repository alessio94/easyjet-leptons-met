/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TriggerSFAlg.h"

#include "AthContainers/AuxElement.h"
#include <AthContainers/ConstDataVector.h>

#include "TLorentzVector.h"

namespace HLLTT
{
  TriggerSFAlg::TriggerSFAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {

  }

  StatusCode TriggerSFAlg::initialize()
  {

    // Read syst-aware input handles
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    ATH_CHECK (m_pass_trigger_SLT.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_pass_trigger_DLT.initialize(m_systematicsList, m_eventHandle));
    
    ATH_CHECK (m_selected_el.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK (m_selected_mu.initialize(m_systematicsList, m_muonHandle));

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

    ATH_CHECK(m_year.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK(m_eventTriggerSF.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode TriggerSFAlg::execute()
  {
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {

      // Retrieve inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

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
	  if(!mu0) {
	    mu0 = muon;
	    break;
	  }
	}
      }
      computeTriggerSF(event, sys, ele0, mu0);
    }

    return StatusCode::SUCCESS;
  }

  void TriggerSFAlg::computeTriggerSF
  (const xAOD::EventInfo *event, const CP::SystematicSet& sys,
   const xAOD::Electron* ele0, const xAOD::Muon* mu0){
    float triggerSF = 1.;
    int year = m_year.get(*event, sys);
    std::vector<std::string> single_ele_path;
    std::string single_ele_SF_path;
    std::vector<std::string> single_mu_path;
    std::string single_mu_SF_path;
    if(m_pass_trigger_SLT.get(*event, sys)){
      if(ele0&&mu0){
	if(ele0->pt()>mu0->pt()){
	  getSingleEleTriggers(year, single_ele_path, single_ele_SF_path);
	  if(!single_ele_SF_path.empty())
	    triggerSF *= m_eleTriggerSF.at(single_ele_SF_path).get(*ele0, sys);
	}
	else{
	  getSingleMuTriggers(year, single_mu_path, single_mu_SF_path);
	  if(!single_mu_SF_path.empty())
	    triggerSF *= m_muonTriggerSF.at(single_mu_SF_path).get(*mu0, sys);
	}
      }	  
      else if(ele0){
        getSingleEleTriggers(year, single_ele_path, single_ele_SF_path);
        if(!single_ele_SF_path.empty())
          triggerSF *= m_eleTriggerSF.at(single_ele_SF_path).get(*ele0, sys);
      }
      else if(mu0){
        getSingleMuTriggers(year, single_mu_path, single_mu_SF_path);
        if(!single_mu_SF_path.empty()){
          triggerSF *= m_muonTriggerSF.at(single_mu_SF_path).get(*mu0, sys);
	}
      }
    }
    m_eventTriggerSF.set(*event, triggerSF, sys);
  }

  void TriggerSFAlg::getSingleMuTriggers(int year,
			   std::vector<std::string>& single_mu_paths,
			   std::string& single_mu_SF_path){
    if(year==2015){
      single_mu_paths = {"HLT_mu20_iloose_L1MU15", "HLT_mu50"};
      single_mu_SF_path = "mu20_iloose_L1MU15_OR_mu40";
    }
    else if(2016<=year && year<=2018){
      single_mu_paths = {"HLT_mu26_ivarmedium", "HLT_mu50"};
      single_mu_SF_path = "mu26_ivarmedium_OR_mu50";
    }
    else if(2022<=year && year<=2023){
      single_mu_paths = {
	"HLT_mu24_ivarmedium_L1MU14FCH", "HLT_mu50_L1MU14FCH"};
      single_mu_SF_path = "mu24_ivarmedium_L1MU14FCH_OR_mu50_L1MU14FCH";
    }
  }

  void TriggerSFAlg::getSingleEleTriggers(int year,
			    std::vector<std::string>& single_ele_paths,
			    std::string& single_ele_SF_path){
    if(year==2015){
      single_ele_paths = {
	"HLT_e24_lhmedium_L1EM20VH", "HLT_e60_lhmedium",
	"HLT_e120_lhloose"
      };
      single_ele_SF_path = "e24_lhmedium_L1EM20VH_OR_e60_lhmedium_OR_e120_lhloose";
    }
    else if(2016<=year && year<=2018){
      single_ele_paths = {
	"HLT_e26_lhtight_nod0_ivarloose", "HLT_e60_lhmedium_nod0",
	"HLT_e140_lhloose_nod0"
      };
      single_ele_SF_path = "e26_lhtight_nod0_ivarloose_OR_e60_lhmedium_nod0_OR_e140_lhloose_nod0";
    }
    else if(year==2022){
      single_ele_paths = {
	"HLT_e26_lhtight_ivarloose_L1EM22VHI", "HLT_e60_lhmedium_L1EM22VHI",
	"HLT_e140_lhloose_L1EM22VHI", "HLT_e300_etcut_L1EM22VHI"
      };
    }
    else if(year==2023){
      single_ele_paths = {
	"HLT_e26_lhtight_ivarloose_L1eEM26M", "HLT_e60_lhmedium_L1eEM26M",
	"HLT_e140_lhloose_L1eEM26M", "HLT_e140_lhloose_noringer_L1eEM26M",
	"HLT_e300_etcut_L1eEM26M"
      };
    }
  }
  
}

