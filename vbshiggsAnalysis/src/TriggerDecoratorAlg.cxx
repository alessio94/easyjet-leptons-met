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

    // NOTE: This change would allow non-di-lepton events to pass to the lepton triggers
    // This means that events that have less than two leptons can pass the trigger algorithm
    // However, for the FullLep version, if we require PASS_EXACTLY_TWO_LEPTONS in addition to PASS_TRIGGER then this issue goes away
    // And this allows us to use the trigger algorithm for one lepton signal events for the SemiLep version
    if (electrons->size() == 1 && muons->size() == 0) {
      ele0 = electrons->at(0);
    }

    if (electrons->size() == 0 && muons->size() == 1) {
      mu0 = muons->at(0);
    }

    if (ele0 || mu0) evaluateSingleLeptonTrigger(event, runBoolDecos, ele0, mu0, sys);
    if (ele1 || mu1) evaluateSingleLeptonTrigger(event, runBoolDecos, ele1, mu1, sys);
    if ((ele0 && ele1) || (mu0 && mu1))
      evaluateDiLeptonTrigger(event, runBoolDecos, ele0, ele1, mu0, mu1, sys);
    if (ele0 && mu0) evaluateAsymmetricLeptonTrigger(event, ele0, mu0, sys);
  }

  void TriggerDecoratorAlg::evaluateSingleLeptonTrigger
  (const xAOD::EventInfo *event, const runBoolReadDecoMap& runBoolDecos,
   const xAOD::Electron *ele, const xAOD::Muon *mu,
   const CP::SystematicSet& sys)
  {

    // Check single electron triggers
    std::vector<std::string> single_ele_paths;
    int year = m_year.get(*event, sys);
    getSingleEleTriggers(year, event, runBoolDecos, single_ele_paths);

    bool trigPassed_SET = false;
    if(ele){
      for(const auto& trig : single_ele_paths){
        bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
        if (pass){
          bool match = m_matchingTool->match(*ele, trig);
          trigPassed_SET |= match;
        }
      }
      trigPassed_SET &= ele->pt() > m_pt_threshold[VBSHIGGS::SLT][VBSHIGGS::ele];
    }

    // Check single muon triggers
    std::vector<std::string> single_mu_paths;
    getSingleMuTriggers(year, event, runBoolDecos, single_mu_paths);

    bool trigPassed_SMT = false;
    if (mu){
      for(const auto& trig : single_mu_paths){
        bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
        if (pass){
          bool match = m_matchingTool->match(*mu, trig);
          trigPassed_SMT |= match;
        }
      }
      trigPassed_SMT &= mu->pt() > m_pt_threshold[VBSHIGGS::SLT][VBSHIGGS::mu];
    }

    if(trigPassed_SET || trigPassed_SMT){
      m_trig_bools.at(VBSHIGGS::SLT) = true;
    }
  }

  void TriggerDecoratorAlg::evaluateDiLeptonTrigger
  (const xAOD::EventInfo *event, const runBoolReadDecoMap& runBoolDecos,
   const xAOD::Electron *ele0, const xAOD::Electron *ele1,
   const xAOD::Muon *mu0, const xAOD::Muon *mu1,
   const CP::SystematicSet& sys)
  {
    std::vector<std::string> di_ele_paths;

    int year = m_year.get(*event, sys);
    getDiEleTriggers(year, event, runBoolDecos, di_ele_paths);

    bool trigPassed_DET = false;
    if (ele0 && ele1) {
      for (const auto &trig : di_ele_paths){
        bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
        if (pass) {
          bool match = m_matchingTool->match({ele0, ele1}, trig);
          trigPassed_DET |= match;
        }
      }
      trigPassed_DET &= ele0->pt() > m_pt_threshold[VBSHIGGS::DLT][VBSHIGGS::leadingele];
      trigPassed_DET &= ele1->pt() > m_pt_threshold[VBSHIGGS::DLT][VBSHIGGS::subleadingele];
    }

    // Check di-muon triggers
    std::vector<std::string> di_mu_paths;
    getDiMuTriggers(year, di_mu_paths);

    bool trigPassed_DMT = false;
    if (mu0 && mu1) {
      for (const auto &trig : di_mu_paths){
        bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
        if (pass) {
          bool match = m_matchingTool->match({mu0, mu1}, trig);
          trigPassed_DMT |= match;
        }
      }
      trigPassed_DMT &= mu0->pt() > m_pt_threshold[VBSHIGGS::DLT][VBSHIGGS::leadingmu];
      trigPassed_DMT &= mu1->pt() > m_pt_threshold[VBSHIGGS::DLT][VBSHIGGS::subleadingmu];
    }

    if(trigPassed_DET || trigPassed_DMT){
      m_trig_bools.at(VBSHIGGS::DLT) = true;
    }
  }

  void TriggerDecoratorAlg::evaluateAsymmetricLeptonTrigger
  (const xAOD::EventInfo *event,
   const xAOD::Electron *ele, const xAOD::Muon *mu,
   const CP::SystematicSet& sys)
  {
    int year = m_year.get(*event, sys);

    bool trigPassed_ASLT1_em = false;
    bool trigPassed_ASLT1_me = false;
    bool trigPassed_ASLT2 = false;
    if (ele && mu) {

      std::vector<std::string> asym_lepton_paths;
      getAsymLep2Triggers(year, asym_lepton_paths);

      for(const auto& trig : asym_lepton_paths){
        bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
        if (pass){
          bool match = m_matchingTool->match(*ele, trig) && m_matchingTool->match(*mu, trig);
          trigPassed_ASLT2 |= match;
        }
      }
      trigPassed_ASLT2 &= ele->pt() > m_pt_threshold[VBSHIGGS::ASLT2][VBSHIGGS::leadingele];
      trigPassed_ASLT2 &= mu->pt() > m_pt_threshold[VBSHIGGS::ASLT2][VBSHIGGS::leadingmu];
      if (ele->pt() > mu->pt()) {
        asym_lepton_paths = {};
        getAsymLep1emTriggers(year, asym_lepton_paths);
        
        for(const auto& trig : asym_lepton_paths){
          bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
          if (pass){
            bool match = m_matchingTool->match(*ele, trig) && m_matchingTool->match(*mu, trig);
            trigPassed_ASLT1_em |= match;
          }
        }
        trigPassed_ASLT1_em &= ele->pt() > m_pt_threshold[VBSHIGGS::ASLT1_em][VBSHIGGS::leadingele];
        trigPassed_ASLT1_em &= mu->pt() > m_pt_threshold[VBSHIGGS::ASLT1_em][VBSHIGGS::leadingmu];
      } 
      else {
        asym_lepton_paths = {};
        getAsymLep1meTriggers(year, asym_lepton_paths);

        for(const auto& trig : asym_lepton_paths){
          bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
          if (pass){
            bool match = m_matchingTool->match(*ele, trig) && m_matchingTool->match(*mu, trig);
            trigPassed_ASLT1_me |= match;
          }
        }
        trigPassed_ASLT1_me &= ele->pt() > m_pt_threshold[VBSHIGGS::ASLT1_me][VBSHIGGS::leadingele];
        trigPassed_ASLT1_me &= mu->pt() > m_pt_threshold[VBSHIGGS::ASLT1_me][VBSHIGGS::leadingmu];
      }
    }

    if(trigPassed_ASLT1_em){
      m_trig_bools.at(VBSHIGGS::ASLT1_em) = true;
    }
    if(trigPassed_ASLT1_me){
      m_trig_bools.at(VBSHIGGS::ASLT1_me) = true;
    }
    if(trigPassed_ASLT2){
      m_trig_bools.at(VBSHIGGS::ASLT2) = true;
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
    else if(year<=2016 && year<=2018)
      m_pt_threshold[VBSHIGGS::SLT][VBSHIGGS::mu] = 27. * Athena::Units::GeV;
    else
      m_pt_threshold[VBSHIGGS::SLT][VBSHIGGS::mu] = 25. * Athena::Units::GeV;

    //Di-lepton triggers
    //ee
    if(year==2015) {
      m_pt_threshold[VBSHIGGS::DLT][VBSHIGGS::leadingele] = 13. * Athena::Units::GeV;
      m_pt_threshold[VBSHIGGS::DLT][VBSHIGGS::subleadingele] = 13. * Athena::Units::GeV;
    }
    // prescaled periods B5-B8
    // https://twiki.cern.ch/twiki/bin/view/Atlas/TrigEgammaRecommendedTriggers2017
    else if(runBoolDecos.at(VBSHIGGS::is17_periodB5_B8)(*event)) {
      m_pt_threshold[VBSHIGGS::DLT][VBSHIGGS::leadingele] = 25. * Athena::Units::GeV;
      m_pt_threshold[VBSHIGGS::DLT][VBSHIGGS::subleadingele] = 25. * Athena::Units::GeV;
    } else {
      m_pt_threshold[VBSHIGGS::DLT][VBSHIGGS::leadingele] = 18. * Athena::Units::GeV;
      m_pt_threshold[VBSHIGGS::DLT][VBSHIGGS::subleadingele] = 18. * Athena::Units::GeV;
    }

    //mm
    if(year==2015) {
      m_pt_threshold[VBSHIGGS::DLT][VBSHIGGS::leadingmu] = 19. * Athena::Units::GeV;
      m_pt_threshold[VBSHIGGS::DLT][VBSHIGGS::subleadingmu] = 10. * Athena::Units::GeV;
    }
    else if(year<=2016 && year<=2018) {
      m_pt_threshold[VBSHIGGS::DLT][VBSHIGGS::leadingmu] = 24. * Athena::Units::GeV;
      m_pt_threshold[VBSHIGGS::DLT][VBSHIGGS::subleadingmu] = 10. * Athena::Units::GeV;
    } else {
      m_pt_threshold[VBSHIGGS::DLT][VBSHIGGS::leadingmu] = 15. * Athena::Units::GeV;
      m_pt_threshold[VBSHIGGS::DLT][VBSHIGGS::subleadingmu] = 15. * Athena::Units::GeV;
    }

    m_pt_threshold[VBSHIGGS::ASLT1_em][VBSHIGGS::leadingele] = 27. * Athena::Units::GeV;
    m_pt_threshold[VBSHIGGS::ASLT1_em][VBSHIGGS::leadingmu] = 9. * Athena::Units::GeV;

    m_pt_threshold[VBSHIGGS::ASLT1_me][VBSHIGGS::leadingmu] = 26. * Athena::Units::GeV;
    m_pt_threshold[VBSHIGGS::ASLT1_me][VBSHIGGS::leadingele] = 9. * Athena::Units::GeV;

    //Configuration 2
    m_pt_threshold[VBSHIGGS::ASLT2][VBSHIGGS::leadingele] = 18. * Athena::Units::GeV;
    m_pt_threshold[VBSHIGGS::ASLT2][VBSHIGGS::leadingmu] = 15. * Athena::Units::GeV;
  }
}
