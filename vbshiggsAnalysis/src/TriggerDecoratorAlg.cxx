/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration   
*/

#include "TriggerDecoratorAlg.h"
#include "TriggerUtils.h"
#include <AthenaKernel/Units.h>

namespace VBSHIGGS
{
  TriggerDecoratorAlg::TriggerDecoratorAlg(const std::string &name,
					   ISvcLocator *pSvcLocator)
      : EL::AnaAlgorithm(name, pSvcLocator){}

  StatusCode TriggerDecoratorAlg::initialize()
  {
    ATH_CHECK(m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK(m_year.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK(m_muonHandle.initialize(m_systematicsList));

    ////////////////// Trigger ///////////////////
    
    // make trigger decorators
    for (auto trig : m_triggers){
      CP::SysReadDecorHandle<bool> deco {this, "trig"+trig, trig, "Name of trigger"};
      m_triggerdecos.emplace(trig, deco);
      ATH_CHECK(m_triggerdecos.at(trig).initialize(m_systematicsList, m_eventHandle));
    }

    // make trigger flags
    for (const auto& [channel, name] : m_triggerChannels) {
      m_trig_bools.emplace(channel, false);
      CP::SysWriteDecorHandle<bool> whandle{"pass_trigger_"+name+"_%SYS%", this};
      m_trig_branches.emplace(channel, whandle);
      ATH_CHECK(m_trig_branches.at(channel).initialize(m_systematicsList, m_eventHandle));
    }

    // make bool for trig paths
    for(const auto& [runBool, name] : m_runBooleans) {
      SG::ReadDecorHandleKey<xAOD::EventInfo> deco;
      deco = "EventInfo." + name;
      m_runBooleans_key.emplace(runBool, deco);
      ATH_CHECK(m_runBooleans_key.at(runBool).initialize());
    }

    if(m_saveHighLevelVariables) {
      ANA_CHECK(ele0_passSET_decor.initialize(m_systematicsList, m_eventHandle));
      ANA_CHECK(ele1_passSET_decor.initialize(m_systematicsList, m_eventHandle));
      ANA_CHECK(mu0_passSMT_decor.initialize(m_systematicsList, m_eventHandle));
      ANA_CHECK(mu1_passSMT_decor.initialize(m_systematicsList, m_eventHandle));

      ANA_CHECK(ele0_trigPassed_decor.initialize(m_systematicsList, m_eventHandle));
      ANA_CHECK(ele1_trigPassed_decor.initialize(m_systematicsList, m_eventHandle));
      ANA_CHECK(mu0_trigPassed_decor.initialize(m_systematicsList, m_eventHandle));
      ANA_CHECK(mu1_trigPassed_decor.initialize(m_systematicsList, m_eventHandle));

      ANA_CHECK(ele0_trigMatched_decor.initialize(m_systematicsList, m_eventHandle));
      ANA_CHECK(ele1_trigMatched_decor.initialize(m_systematicsList, m_eventHandle));
      ANA_CHECK(mu0_trigMatched_decor.initialize(m_systematicsList, m_eventHandle));
      ANA_CHECK(mu1_trigMatched_decor.initialize(m_systematicsList, m_eventHandle));
    }

    // Trigger SF tool
    for(const auto& trig : m_eleTrigSF){
      m_eleTriggerSF.emplace
        (trig, CP::SysReadDecorHandle<float>("el_trigEffSF_"+trig+"_%SYS%", this));
      ATH_CHECK (m_eleTriggerSF.at(trig).initialize(m_systematicsList, m_electronHandle));
    }

    for(const auto& trig : m_muTrigSF){
      m_muTriggerSF.emplace
        (trig, CP::SysReadDecorHandle<float>("muon_trigEffSF_"+trig+"_%SYS%", this));
      ATH_CHECK (m_muTriggerSF.at(trig).initialize(m_systematicsList, m_muonHandle));
    }

    ATH_CHECK(m_ele0TriggerSF.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_ele1TriggerSF.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_mu0TriggerSF.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_mu1TriggerSF.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_eventTriggerSF.initialize(m_systematicsList, m_eventHandle));

    ////////////////////////////////

    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode TriggerDecoratorAlg::execute()
  {
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector()){
      // Retrive inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      runBoolReadDecoMap runBoolDecos;
      for (const auto& [runBool, key] : m_runBooleans_key){
        runBoolDecos.emplace(runBool, key);
      }

      for (const auto& [channel, name] : m_triggerChannels) m_trig_bools.at(channel) = false;

      setThresholds(event, runBoolDecos, sys);
      evaluateTriggerCuts(event, runBoolDecos, electrons, muons, sys);

      for (auto& [channel, var] : m_trig_bools) {
        m_trig_branches.at(channel).set(*event, var, sys);
      }

      ele0_trigPassed.clear();
      ele1_trigPassed.clear();
      mu0_trigPassed.clear();
      mu1_trigPassed.clear();

      ele0_trigMatched.clear();
      ele1_trigMatched.clear();
      mu0_trigMatched.clear();
      mu1_trigMatched.clear();

    }
    return StatusCode::SUCCESS;
  }

  void TriggerDecoratorAlg::evaluateTriggerCuts(const xAOD::EventInfo *event, const runBoolReadDecoMap& runBoolDecos,
                                                const xAOD::ElectronContainer *electrons , const xAOD::MuonContainer *muons,
                                                const CP::SystematicSet& sys) {

    //Leptons
    const xAOD::Electron* ele0 = nullptr;
    const xAOD::Electron* ele1 = nullptr;
    
    const xAOD::Muon* mu0 = nullptr;
    const xAOD::Muon* mu1 = nullptr;
    
    if (electrons->size() >= 2) {
      ele0 = electrons->at(0);
      ele1 = electrons->at(1);
    }
    
    if (muons->size() >= 2) {
      mu0 = muons->at(0);
      mu1 = muons->at(1);
    }
    
    if (electrons->size() == 1 && muons->size() == 1) {
      ele0 = electrons->at(0);
      mu0 = muons->at(0);
    }

    if (electrons->size() == 1 && muons->size() == 0) {
      ele0 = electrons->at(0);
    }

    if (electrons->size() == 0 && muons->size() == 1) {
      mu0 = muons->at(0);
    }

    float ele0_trigSF = 1.0;
    float ele1_trigSF = 1.0;
    float mu0_trigSF = 1.0;
    float mu1_trigSF = 1.0;
    float eventTriggerSF = 1.0;
    
    // Based on decision of group, only SLT will remain in the analysis code
    if (ele0 || mu0) evaluateSingleLeptonTrigger(event, runBoolDecos, ele0, mu0, sys, ele0_trigPassed, mu0_trigPassed, ele0_trigMatched, mu0_trigMatched, ele0_passSET, mu0_passSMT, ele0_trigSF, mu0_trigSF);
    if (ele1 || mu1) evaluateSingleLeptonTrigger(event, runBoolDecos, ele1, mu1, sys, ele1_trigPassed, mu1_trigPassed, ele1_trigMatched, mu1_trigMatched, ele1_passSET, mu1_passSMT, ele1_trigSF, mu1_trigSF);

    // For now, turn of muon trigger SF as there are things I don't understand here
    if (ele0_passSET) eventTriggerSF *= ele0_trigSF;
    if (mu0_passSMT)  eventTriggerSF *= mu0_trigSF;
    if (ele1_passSET) eventTriggerSF *= ele1_trigSF;
    if (mu1_passSMT)  eventTriggerSF *= mu1_trigSF;

    m_ele0TriggerSF.set(*event, ele0_trigSF, sys);
    m_ele1TriggerSF.set(*event, ele1_trigSF, sys);
    m_mu0TriggerSF.set(*event, mu0_trigSF, sys);
    m_mu1TriggerSF.set(*event, mu1_trigSF, sys);
    m_eventTriggerSF.set(*event, eventTriggerSF, sys);

    if(m_saveHighLevelVariables) {
      // Store the output as decorators which will be written into the nTuples
      ele0_passSET_decor.set(*event, ele0_passSET, sys);
      ele1_passSET_decor.set(*event, ele1_passSET, sys);
      mu0_passSMT_decor.set(*event, mu0_passSMT, sys);
      mu1_passSMT_decor.set(*event, mu1_passSMT, sys);

      ele0_trigPassed_decor.set(*event, ele0_trigPassed, sys);
      ele1_trigPassed_decor.set(*event, ele1_trigPassed, sys);
      mu0_trigPassed_decor.set(*event, mu0_trigPassed, sys);
      mu1_trigPassed_decor.set(*event, mu1_trigPassed, sys);

      ele0_trigMatched_decor.set(*event, ele0_trigMatched, sys);
      ele1_trigMatched_decor.set(*event, ele1_trigMatched, sys);
      mu0_trigMatched_decor.set(*event, mu0_trigMatched, sys);
      mu1_trigMatched_decor.set(*event, mu1_trigMatched, sys);
    }
  }

  void TriggerDecoratorAlg::evaluateSingleLeptonTrigger
  (const xAOD::EventInfo *event, const runBoolReadDecoMap& runBoolDecos,
   const xAOD::Electron *ele, const xAOD::Muon *mu,
   const CP::SystematicSet& sys,
   std::vector<std::string>& ele_trigPassed, std::vector<std::string>& mu_trigPassed,
   std::vector<std::string>& ele_trigMatched, std::vector<std::string>& mu_trigMatched,
   bool& ele_passSET, bool& mu_passSMT, float& ele_trigSF, float& mu_trigSF)
  {
    // Check single electron triggers
    std::vector<std::string> single_ele_paths;
    std::string single_ele_SF_path;
    int year = m_year.get(*event, sys);
    getSingleEleTriggers(year, event, runBoolDecos, single_ele_paths, single_ele_SF_path);

    bool trigPassed_SET = false;
    if(ele){
      for(const auto& trig : single_ele_paths){
        bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
        if (pass){
          if(m_saveHighLevelVariables) ele_trigPassed.push_back(trig); // Record trigger name if it is passed for the electron
          bool match = m_matchingTool->match(*ele, trig);
          if (match && m_saveHighLevelVariables) ele_trigMatched.push_back(trig); // Record trigger name if it is matched for the electron
          trigPassed_SET |= match;
        }
      }
      trigPassed_SET &= ele->pt() > m_pt_threshold[VBSHIGGS::SLT][VBSHIGGS::ele];
      ele_passSET = trigPassed_SET;

      // If electron is matched to one of the single lepton triggers, then get the SF for the OR SLT chain
      if(trigPassed_SET) {
        if (!single_ele_SF_path.empty() && m_eleTriggerSF.contains(single_ele_SF_path)) ele_trigSF *= m_eleTriggerSF.at(single_ele_SF_path).get(*ele, sys);
        else if (!single_ele_SF_path.empty()) ATH_MSG_WARNING("Missing trigger SF handle: " + single_ele_SF_path);
      }
    }

    // Check single muon triggers
    std::vector<std::string> single_mu_paths;
    std::string single_mu_SF_path;
    getSingleMuTriggers(year, event, runBoolDecos, single_mu_paths, single_mu_SF_path);

    bool trigPassed_SMT = false;
    if (mu){
      for(const auto& trig : single_mu_paths){
        bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
        if (pass){
          if(m_saveHighLevelVariables) mu_trigPassed.push_back(trig); // Record trigger name if it is passed for the muon
          bool match = m_matchingTool->match(*mu, trig);
          if (match && m_saveHighLevelVariables) mu_trigMatched.push_back(trig); // Record trigger name if it is matched for the muon
          trigPassed_SMT |= match;
        }
      }
      trigPassed_SMT &= mu->pt() > m_pt_threshold[VBSHIGGS::SLT][VBSHIGGS::mu];
      mu_passSMT = trigPassed_SMT;

      // If muon is matched to one of the single lepton triggers, then get the SF for the OR SLT chain
      if(trigPassed_SMT) {
        if (!single_mu_SF_path.empty() && m_muTriggerSF.contains(single_mu_SF_path)) mu_trigSF *= m_muTriggerSF.at(single_mu_SF_path).get(*mu, sys);
        else if (!single_mu_SF_path.empty()) ATH_MSG_WARNING("Missing trigger SF handle: " + single_mu_SF_path);
      }
    }

    if(trigPassed_SET || trigPassed_SMT){
      m_trig_bools.at(VBSHIGGS::SLT) = true;
    }
  }

  void TriggerDecoratorAlg::setThresholds(const xAOD::EventInfo* event,
					const runBoolReadDecoMap& runBoolDecos, const CP::SystematicSet& sys) {
    
    int year = m_year.get(*event, sys);

    // Single-lepton triggers
    if(year==2015)
      m_pt_threshold[VBSHIGGS::SLT][VBSHIGGS::ele] = 25. * Athena::Units::GeV;
    // 2022 75 bunches
    else if(runBoolDecos.at(VBSHIGGS::is22_75bunches)(*event))
      m_pt_threshold[VBSHIGGS::SLT][VBSHIGGS::ele] = 18. * Athena::Units::GeV;
    else
      m_pt_threshold[VBSHIGGS::SLT][VBSHIGGS::ele] = 27. * Athena::Units::GeV;

    if(year==2015)
      m_pt_threshold[VBSHIGGS::SLT][VBSHIGGS::mu] = 21. * Athena::Units::GeV;
    else if(year>=2016 && year<=2018)
      m_pt_threshold[VBSHIGGS::SLT][VBSHIGGS::mu] = 27. * Athena::Units::GeV;
    else
      m_pt_threshold[VBSHIGGS::SLT][VBSHIGGS::mu] = 25. * Athena::Units::GeV;
  }
}
