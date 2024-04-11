/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "HHbbllSelectorAlg.h"
#include <AthenaKernel/Units.h>

#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>
#include <AthContainers/ConstDataVector.h>
#include <AthenaKernel/Units.h>


namespace HHBBLL
{

  HHbbllSelectorAlg::HHbbllSelectorAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : EL::AnaAlgorithm(name, pSvcLocator)
  {
    declareProperty("passTriggers",m_passTriggers);
  }


  StatusCode HHbbllSelectorAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("     HHbbllSelectorAlg      \n");
    ATH_MSG_INFO("*********************************\n");

    // Initialise global event filter
    ATH_CHECK (m_filterParams.initialize(m_systematicsList));
    
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));    

    ATH_CHECK (m_runNumber.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_rdmRunNumber.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK (m_matchingTool.retrieve());

    for (auto& [key, value] : m_boolnames) {
      m_bools.emplace(key, false);
      CP::SysWriteDecorHandle<bool> whandle{value+"_%SYS%", this};
      m_Bbranches.emplace(key, whandle);
      ATH_CHECK(m_Bbranches.at(key).initialize(m_systematicsList, m_eventHandle));
    }

    // make trigger decorators
    for (auto trig : m_triggers){
      CP::SysReadDecorHandle<bool> deco {this, "trig"+trig, trig, "Name of trigger"};
      m_triggerdecos.emplace(trig, deco);
      ATH_CHECK(m_triggerdecos.at(trig).initialize(m_systematicsList, m_eventHandle));
    }

    //Asymmetric Lepton triggers
    //Configuration 1
    m_pt_threshold[HHBBLL::ASLT1_em][HHBBLL::leadingele] = 27. * Athena::Units::GeV;
    m_pt_threshold[HHBBLL::ASLT1_em][HHBBLL::leadingmu] = 9. * Athena::Units::GeV;

    m_pt_threshold[HHBBLL::ASLT1_me][HHBBLL::leadingmu] = 26. * Athena::Units::GeV;
    m_pt_threshold[HHBBLL::ASLT1_me][HHBBLL::leadingele] = 9. * Athena::Units::GeV;

    //Configuration 2
    m_pt_threshold[HHBBLL::ASLT2][HHBBLL::leadingele] = 18. * Athena::Units::GeV;
    m_pt_threshold[HHBBLL::ASLT2][HHBBLL::leadingmu] = 15. * Athena::Units::GeV;


    // special flag for all cuts
    ATH_CHECK (m_passallcuts.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    std::vector<std::string> boolnameslist;
    for (const auto& [key, value] : m_boolnames) {
      boolnameslist.push_back(value);
    }
    m_bbllCuts.CheckInputCutList(m_inputCutList, boolnameslist);

    m_inputCutKeys.resize(m_inputCutList.size());
    std::vector<bool> inputWasFound (m_inputCutList.size(), false);
    for (const auto& [key, value]: m_boolnames) {
      auto it = std::find(m_inputCutList.begin(), m_inputCutList.end(), value);
      if (it != m_inputCutList.end()) {
        auto index = it - m_inputCutList.begin();
        m_inputCutKeys.at(index) = key;
        inputWasFound.at(index) = true;
      }
    }

    for (unsigned int index = 0; index < inputWasFound.size(); index++) {
      if(inputWasFound.at(index)) continue;
      ATH_MSG_ERROR("Doubled or falsely spelled cuts in CutList (see config file)." + m_inputCutList[index]);
    }

    for (const auto &cut : m_inputCutKeys) {
      m_bbllCuts.add(m_boolnames[cut]);
    }

    //After filling the CutManager, book your histograms.
    const unsigned int nbins = m_bbllCuts.size() + 1; //  need an extra bin for the total num of events.
    ANA_CHECK (book (TEfficiency("AbsoluteEfficiency","Absolute Efficiency of HH->bbll cuts;Cuts;#epsilon",
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("RelativeEfficiency","Relative Efficiency of HH->bbll cuts;Cuts;#epsilon",
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("StandardCutFlow","StandardCutFlow of HH->bbll cuts;Cuts;#epsilon",
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TH1F("EventsPassed_BinLabeling", "Events passed by each cut / Bin labeling", nbins, 0.5, nbins + 0.5)));

    return StatusCode::SUCCESS;
  }


  StatusCode HHbbllSelectorAlg::execute()
  {
    // Global filter originally false
    CP::SysFilterReporterCombiner filterCombiner (m_filterParams, false);

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      CP::SysFilterReporter filter (filterCombiner, sys);

      // Retrive inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));
      
      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      auto nonbjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      for(const xAOD::Jet* jet : *jets) {
        if (WPgiven) {
          if (m_isBtag.get(*jet, sys)) bjets->push_back(jet);
          else nonbjets->push_back(jet);
        }
      }

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));
      
      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      const xAOD::MissingETContainer *metCont = nullptr;
      ANA_CHECK (m_metHandle.retrieve (metCont, sys));
      const xAOD::MissingET* met = (*metCont)["Final"];
      if (!met) {
        ATH_MSG_ERROR("Could not retrieve MET");
        return StatusCode::FAILURE;
      }

      m_bools.at(HHBBLL::Pass_ll) = false;
      m_bools.at(HHBBLL::IS_SF) = false;
      m_bools.at(HHBBLL::IS_ee) = false;
      m_bools.at(HHBBLL::IS_mm) = false;
      m_bools.at(HHBBLL::IS_em) = false;
      m_bools.at(HHBBLL::pass_trigger_SLT) = false;
      m_bools.at(HHBBLL::pass_trigger_DLT) = false;
      m_bools.at(HHBBLL::pass_trigger_ASLT1_em) = false;
      m_bools.at(HHBBLL::pass_trigger_ASLT1_me) = false;
      m_bools.at(HHBBLL::pass_trigger_ASLT2) = false;
      m_bools.at(HHBBLL::EXACTLY_TWO_LEPTONS) = false;
      m_bools.at(HHBBLL::PASS_TRIGGER) = false;
      m_bools.at(HHBBLL::TWO_OPPOSITE_CHARGE_LEPTONS) = false;
      m_bools.at(HHBBLL::EXACTLY_TWO_B_JETS) = false;
      m_bools.at(HHBBLL::DILEPTON_MASS_SR1) = false;
      m_bools.at(HHBBLL::VBFVETO_SR1) = false;
      m_bools.at(HHBBLL::DILEPTON_MASS_SR2) = false;
      m_bools.at(HHBBLL::DIBJET_MASS_SR2) = false;

      unsigned int runNumber = m_isMC ? m_rdmRunNumber.get(*event, sys) :
      m_runNumber.get(*event, sys);

      int year = 0;
      std::unordered_map<HHBBLL::RunBooleans, bool> runBoolMap;
      for(unsigned int i=0; i<=HHBBLL::Count; i++){
        runBoolMap[static_cast<HHBBLL::RunBooleans>(i)] = false;
      }

      setRunNumberQuantities(runNumber, year, runBoolMap);

      // Leptons
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

      evaluateTriggerCuts(year, runBoolMap, event, ele0, ele1, mu0, mu1, m_bbllCuts, sys);
      evaluateLeptonCuts(*electrons, *muons, m_bbllCuts);
      evaluateJetCuts(*bjets, *nonbjets, m_bbllCuts);
      evaluateBJetLeptonCuts(*bjets, *electrons, *muons);
      
      bool passedall = true;
      for (const auto& [key, value] : m_boolnames) {
        auto it = std::find(m_STANDARD_CUTS.begin(), m_STANDARD_CUTS.end(), value);
        if (it != m_STANDARD_CUTS.end()) {
          passedall &= m_bools.at(key);
        }
      }
      m_passallcuts.set(*event, passedall, sys);

      bool pass_baseline=false;
      if(m_bools.at(HHBBLL::PASS_TRIGGER) && m_bools.at(HHBBLL::EXACTLY_TWO_LEPTONS) && m_bools.at(HHBBLL::EXACTLY_TWO_B_JETS)) pass_baseline=true;
      
      // do the CUTFLOW only with sys="" -> NOSYS
      if (sys.name()=="") {

        // Compute total_events
        m_total_events+=1;

        for (const auto &cut : m_inputCutKeys) {
          if (m_bbllCuts.exists(m_boolnames.at(cut))) {
            m_bbllCuts(m_boolnames.at(cut)).passed = m_bools.at(cut);
            if (m_bbllCuts(m_boolnames.at(cut)).passed) {
              m_bbllCuts(m_boolnames.at(cut)).counter += 1;
            }
          }
        }
      }

      // Check how many consecutive cuts are passed by the event.
      unsigned int consecutive_cuts = 0;
      for (size_t i = 0; i < m_bbllCuts.size(); ++i) {
        if (m_bbllCuts[i].passed)
          consecutive_cuts++;
        else
          break;
      }

      // Here we basically increment the  N_events(pass_i  AND pass_i-1  AND ... AND pass_0) for the i-cut.
      // I think this is an elegant way to do it :) . Considering the difficulties a configurable cut list imposes.
      for (unsigned int i=0; i<consecutive_cuts; i++) {
        m_bbllCuts[i].relativeCounter+=1;
      }

      for (auto& [key, var] : m_bools) {
        m_Bbranches.at(key).set(*event, var, sys);
      }

      if (!m_bypass && !pass_baseline) continue;
      filter.setPassed(true);
    }
    return StatusCode::SUCCESS;
  }

  StatusCode HHbbllSelectorAlg::finalize()
  {
    //adapt the following for each syst TODO
    ATH_MSG_INFO("Total events = " << m_total_events <<std::endl);
    ANA_CHECK (m_filterParams.finalize ());
    m_bbllCuts.CheckCutResults(); // Print CheckCutResults

    if(m_saveCutFlow) {
      m_bbllCuts.DoAbsoluteEfficiency(m_total_events, efficiency("AbsoluteEfficiency"));
      m_bbllCuts.DoRelativeEfficiency(m_total_events, efficiency("RelativeEfficiency"));
      m_bbllCuts.DoStandardCutFlow(m_total_events, efficiency("StandardCutFlow"));
      m_bbllCuts.DoCutflowLabeling(m_total_events, hist("EventsPassed_BinLabeling"));

    }
    else {
      delete efficiency("AbsoluteEfficiency");
      delete efficiency("RelativeEfficiency");
      delete efficiency("StandardCutFlow");
      delete hist("EventsPassed_BinLabeling");
    }

    return StatusCode::SUCCESS;
  }

  void HHbbllSelectorAlg::evaluateTriggerCuts(int year, std::unordered_map<HHBBLL::RunBooleans, bool> runBoolMap, const xAOD::EventInfo *event, const xAOD::Electron* ele0,  
                          const xAOD::Electron* ele1, const xAOD::Muon* mu0, const xAOD::Muon* mu1, CutManager& bbllCuts, const CP::SystematicSet& sys) {

    if (!bbllCuts.exists("PASS_TRIGGER"))
        return;

    if (ele0 || mu0) evaluateSingleLeptonTrigger(year, runBoolMap, event, ele0, mu0, sys);
    if (ele1 || mu1) evaluateSingleLeptonTrigger(year, runBoolMap, event, ele1, mu1, sys);
    if ((ele0 && ele1) || (mu0 && mu1)) evaluateDiLeptonTrigger(year, runBoolMap, event, ele0, ele1, mu0, mu1, sys);
    if (ele0 && mu0) evaluateAsymmetricLeptonTrigger(year, event, ele0, mu0, sys);

    bool pass_trigger_ASLT = m_bools.at(HHBBLL::pass_trigger_ASLT1_em) || m_bools.at(HHBBLL::pass_trigger_ASLT1_me) ||
    m_bools.at(HHBBLL::pass_trigger_ASLT2);

    if (m_bools.at(HHBBLL::pass_trigger_SLT) || m_bools.at(HHBBLL::pass_trigger_DLT) || pass_trigger_ASLT) m_bools.at(HHBBLL::PASS_TRIGGER) = true;
  }

  void HHbbllSelectorAlg::evaluateSingleLeptonTrigger(
      int year, std::unordered_map<HHBBLL::RunBooleans, bool> runBoolMap, const xAOD::EventInfo *event,
      const xAOD::Electron *ele, const xAOD::Muon *mu, const CP::SystematicSet& sys)
  {
    // Check single electron triggers
    std::vector<std::string> single_ele_paths;

    if(year==2015){
      single_ele_paths = {
        "HLT_e24_lhmedium_L1EM20VH", "HLT_e60_lhmedium",
        "HLT_e120_lhloose"
      };
    }
    else if(2016<=year && year<=2018){
      single_ele_paths = {
        "HLT_e26_lhtight_nod0_ivarloose", "HLT_e60_lhmedium_nod0",
        "HLT_e140_lhloose_nod0"
      };
    }
    else if(runBoolMap.at(HHBBLL::is22_75bunches)){
      single_ele_paths = {
        "HLT_e17_lhvloose_L1EM15VHI", "HLT_e20_lhvloose_L1EM15VH",
        "HLT_e250_etcut_L1EM22VHI"
      };
    }
    else if(year==2022){
      single_ele_paths = {
        "HLT_e26_lhtight_ivarloose_L1EM22VHI", "HLT_e60_lhmedium_L1EM22VHI",
        "HLT_e140_lhloose_L1EM22VHI", "HLT_e300_etcut_L1EM22VHI"
      };
    }
    else if(runBoolMap.at(HHBBLL::is23_75bunches)){
      single_ele_paths = {
        "HLT_e26_lhtight_ivarloose_L1EM22VHI", "HLT_e60_lhmedium_L1EM22VHI",
        "HLT_e140_lhloose_L1EM22VHI", "HLT_e140_lhloose_noringer_L1EM22VHI",
        "HLT_e300_etcut_L1EM22VHI"
      };
    }
    else if(year==2023){
      single_ele_paths = {
        "HLT_e26_lhtight_ivarloose_L1eEM26M", "HLT_e60_lhmedium_L1eEM26M",
        "HLT_e140_lhloose_L1eEM26M", "HLT_e140_lhloose_noringer_L1eEM26M",
        "HLT_e300_etcut_L1eEM26M"
      };
    }

    bool trigPassed_SET = false;
    if(ele){
      for(const auto& trig : single_ele_paths){
        bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
        if (pass){
          bool match = m_matchingTool->match(*ele, trig);
          trigPassed_SET |= match;
        }
      }
      trigPassed_SET &= ele->pt() > m_pt_threshold[HHBBLL::SLT][HHBBLL::ele];
    }

    // Check single muon triggers
    std::vector<std::string> single_mu_paths;

    if(year==2015){
      single_mu_paths = {"HLT_mu20_iloose_L1MU15", "HLT_mu50"};
    }
    else if(2016<=year && year<=2018){
      single_mu_paths = {"HLT_mu26_ivarmedium", "HLT_mu50"};
    }
    else if(2022<=year && year<=2023 &&
	    !runBoolMap.at(HHBBLL::is22_75bunches) &&
	    !runBoolMap.at(HHBBLL::is23_75bunches) &&
	    !runBoolMap.at(HHBBLL::is23_400bunches)){
      single_mu_paths = {
        "HLT_mu24_ivarmedium_L1MU14FCH", "HLT_mu50_L1MU14FCH",
        "HLT_mu60_0eta105_msonly_L1MU14FCH", "HLT_mu60_L1MU14FCH",
        "HLT_mu80_msonly_3layersEC_L1MU14FCH"
      };
    }

    bool trigPassed_SMT = false;
    if (mu){
      for(const auto& trig : single_mu_paths){
        bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
        if (pass){
          bool match = m_matchingTool->match(*mu, trig);
          trigPassed_SMT |= match;
        }
      }
      trigPassed_SMT &= mu->pt() > m_pt_threshold[HHBBLL::SLT][HHBBLL::mu];
    }

    m_bools.at(HHBBLL::pass_trigger_SLT) |= (trigPassed_SET || trigPassed_SMT);
  }

  void HHbbllSelectorAlg::evaluateDiLeptonTrigger(
      int year, std::unordered_map<HHBBLL::RunBooleans, bool> runBoolMap, const xAOD::EventInfo *event,
      const xAOD::Electron *ele0, const xAOD::Electron *ele1, const xAOD::Muon *mu0,
      const xAOD::Muon *mu1, const CP::SystematicSet& sys)
  {
    std::vector<std::string> di_ele_paths;
    
    if(year==2015){
      di_ele_paths = {"HLT_2e12_lhloose_L12EM10VH"};
    }
    else if(year==2016){
      di_ele_paths = {"HLT_2e17_lhvloose_nod0"};
    }
    else if (runBoolMap.at(HHBBLL::is17PeriodB5_B8)){
      di_ele_paths = {
        "HLT_2e24_lhvloose_nod0", "HLT_e24_lhvloose_nod0_2e12_lhvloose_nod0_L1EM20VH_3EM10VH"
      };
    }
    else if(2017<=year && year<=2018){
      di_ele_paths = {
        "HLT_2e17_lhvloose_nod0_L12EM15VHI", "HLT_2e24_lhvloose_nod0",
        "HLT_e24_lhvloose_nod0_2e12_lhvloose_nod0_L1EM20VH_3EM10VH"
      };
    }
    else if(year==2022){
      di_ele_paths = {
        "HLT_e24_lhvloose_2e12_lhvloose_L1EM20VH_3EM10VH", "HLT_2e17_lhvloose_L12EM15VHI",
        "HLT_2e24_lhvloose_L12EM20VH"
      };
    }
    else if(year==2023){
      di_ele_paths = {
        "HLT_e24_lhvloose_2e12_lhvloose_L1eEM24L_3eEM12L", "HLT_2e17_lhvloose_L12eEM18M",
        "HLT_2e24_lhvloose_L12eEM24L"
      };
    }

    bool trigPassed_DET = false;
    if (ele0 && ele1) {
      for (const auto &trig : di_ele_paths){
        bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
        if (pass) {
          bool match = m_matchingTool->match({ele0, ele1}, trig);
          trigPassed_DET |= match;
        }
      }
      trigPassed_DET &= ele0->pt() > m_pt_threshold[HHBBLL::DLT][HHBBLL::leadingele];
      trigPassed_DET &= ele1->pt() > m_pt_threshold[HHBBLL::DLT][HHBBLL::subleadingele];
    }

    // Check di-muon triggers
    std::vector<std::string> di_mu_paths;

    if(year==2015){
      di_mu_paths = {"HLT_mu18_mu8noL1"};
    }
    else if(2016<=year && year<=2018){
      di_mu_paths = {"HLT_mu22_mu8noL1"};
    }
    else if(2022<=year && year<=2023){
      di_mu_paths = {"HLT_mu22_mu8noL1_L1MU14FCH", "HLT_2mu14_L12MU8F"};
    }

    bool trigPassed_DMT = false;
    if (mu0 && mu1) {
      for (const auto &trig : di_mu_paths){
        bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
        if (pass) {
          bool match = m_matchingTool->match({mu0, mu1}, trig);
          trigPassed_DMT |= match;
        }
      }
      trigPassed_DMT &= mu0->pt() > m_pt_threshold[HHBBLL::DLT][HHBBLL::leadingmu];
      trigPassed_DMT &= mu1->pt() > m_pt_threshold[HHBBLL::DLT][HHBBLL::subleadingmu];
    }

    m_bools.at(HHBBLL::pass_trigger_DLT) = (trigPassed_DET || trigPassed_DMT);
  }

  void HHbbllSelectorAlg::evaluateAsymmetricLeptonTrigger(
      int year, const xAOD::EventInfo *event, const xAOD::Electron *ele, const xAOD::Muon *mu, const CP::SystematicSet& sys)
  {
    bool trigPassed_ASLT1_em = false;
    bool trigPassed_ASLT1_me = false;
    bool trigPassed_ASLT2 = false;
    if (ele && mu) {

      std::vector<std::string> asym_lepton_paths;

      if(year==2015){
        asym_lepton_paths = {"HLT_e17_lhloose_mu14"};
      }
      else if(2016<=year && year<=2018){
        asym_lepton_paths = {"HLT_e17_lhloose_nod0_mu14"};
      }
      else if(2022<=year && year<=2023){
        asym_lepton_paths = {"HLT_e17_lhloose_mu14_L1EM15VH_MU8F"};
      }

      for(const auto& trig : asym_lepton_paths){
        bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
        if (pass){
          bool match = m_matchingTool->match(*ele, trig) && m_matchingTool->match(*mu, trig);
          trigPassed_ASLT2 |= match;
        }
      }
      trigPassed_ASLT2 &= ele->pt() > m_pt_threshold[HHBBLL::ASLT2][HHBBLL::leadingele];
      trigPassed_ASLT2 &= mu->pt() > m_pt_threshold[HHBBLL::ASLT2][HHBBLL::leadingmu];

      if (ele->pt() > mu->pt()) {

        asym_lepton_paths = {};

        if(year==2016){
          asym_lepton_paths = {"HLT_e26_lhmedium_nod0_L1EM22VHI_mu8noL1"};
        }
        else if(2017<=year && year<=2018){
          asym_lepton_paths = {"HLT_e26_lhmedium_nod0_mu8noL1"};
        }
        else if(2022<=year && year<=2023){
          asym_lepton_paths = {"HLT_e26_lhmedium_mu8noL1_L1EM22VHI"};
        }

        for(const auto& trig : asym_lepton_paths){
          bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
          if (pass){
            bool match = m_matchingTool->match(*ele, trig) && m_matchingTool->match(*mu, trig);
            trigPassed_ASLT1_em |= match;
          }
        }
        trigPassed_ASLT1_em &= ele->pt() > m_pt_threshold[HHBBLL::ASLT1_em][HHBBLL::leadingele];
        trigPassed_ASLT1_em &= mu->pt() > m_pt_threshold[HHBBLL::ASLT1_em][HHBBLL::leadingmu];

      } else {

        asym_lepton_paths = {};

        if(year==2015){
          asym_lepton_paths = {"HLT_e7_lhmedium_mu24"};
        }
        else if(2016<=year && year<=2018){
          asym_lepton_paths = {"HLT_e7_lhmedium_nod0_mu24"};
        }
        else if(2022<=year && year<=2023){
          asym_lepton_paths = {"HLT_e7_lhmedium_mu24_L1MU14FCH"};
        }

        for(const auto& trig : asym_lepton_paths){
          bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
          if (pass){
            bool match = m_matchingTool->match(*ele, trig) && m_matchingTool->match(*mu, trig);
            trigPassed_ASLT1_me |= match;
          }
        }
        trigPassed_ASLT1_me &= ele->pt() > m_pt_threshold[HHBBLL::ASLT1_me][HHBBLL::leadingele];
        trigPassed_ASLT1_me &= mu->pt() > m_pt_threshold[HHBBLL::ASLT1_me][HHBBLL::leadingmu];
      }
    }

    m_bools.at(HHBBLL::pass_trigger_ASLT1_em) = trigPassed_ASLT1_em;
    m_bools.at(HHBBLL::pass_trigger_ASLT1_me) = trigPassed_ASLT1_me;
    m_bools.at(HHBBLL::pass_trigger_ASLT2) = trigPassed_ASLT2;
  }

  void HHbbllSelectorAlg::evaluateLeptonCuts
  (const xAOD::ElectronContainer& electrons, const xAOD::MuonContainer& muons,
   CutManager& bbllCuts)
  {
    double mee = -99;
    double mmumu = -99;
    double memu = -99;
    bool Dilepton_Pass_DF = false;
    bool Two_Opposite_Sign_Electrons = false;
    bool Two_Opposite_Sign_Muons = false;
    bool Opposite_Sign_ElecMu = false;

    if (electrons.size() + muons.size() == 2)
      m_bools.at(HHBBLL::EXACTLY_TWO_LEPTONS) = true;

    if (electrons.size() >= 2)
    {
      mee = (electrons.at(0)->p4() + electrons.at(1)->p4()).M();
      Two_Opposite_Sign_Electrons = electrons.at(0)->charge()*electrons.at(1)->charge() == -1;

    }
    if (muons.size() >= 2)
    {
      mmumu = (muons.at(0)->p4() + muons.at(1)->p4()).M();
      Two_Opposite_Sign_Muons = muons.at(0)->charge()*muons.at(1)->charge() == -1;
    }
    if (electrons.size() == 1 && muons.size() == 1)
    {
      memu = (electrons.at(0)->p4() + muons.at(0)->p4()).M();
      Dilepton_Pass_DF = (memu > 15. * Athena::Units::GeV && memu < 110. * Athena::Units::GeV);
      Opposite_Sign_ElecMu = electrons.at(0)->charge()*muons.at(0)->charge() == -1;
    }
    bool Dilepton_Pass_SF_SR1 = ((electrons.size() >= 2  && mee > 15. * Athena::Units::GeV && mee < 75. * Athena::Units::GeV) ||
      (muons.size() >= 2 && mmumu > 15. * Athena::Units::GeV && mmumu < 75. * Athena::Units::GeV));
    bool Dilepton_Pass_SF_SR2 = ((electrons.size() >= 2  && mee > 75. * Athena::Units::GeV && mee < 110. * Athena::Units::GeV) ||
      (muons.size() >= 2 && mmumu > 75. * Athena::Units::GeV && mmumu < 110. * Athena::Units::GeV));

    if ((Two_Opposite_Sign_Electrons || Two_Opposite_Sign_Muons || Opposite_Sign_ElecMu)
      && bbllCuts.exists("TWO_OPPOSITE_CHARGE_LEPTONS"))
    {
      m_bools.at(HHBBLL::TWO_OPPOSITE_CHARGE_LEPTONS) = true;
    }

    if(bbllCuts.exists("DILEPTON_MASS_SR1")) m_bools.at(HHBBLL::DILEPTON_MASS_SR1) = (Dilepton_Pass_SF_SR1 || Dilepton_Pass_DF);
    if(bbllCuts.exists("DILEPTON_MASS_SR2")) m_bools.at(HHBBLL::DILEPTON_MASS_SR2) = Dilepton_Pass_SF_SR2;

  }

  void HHbbllSelectorAlg::evaluateJetCuts(const ConstDataVector<xAOD::JetContainer>& bjets,
                            const ConstDataVector<xAOD::JetContainer>& nonbjets, CutManager& bbllCuts)
  {

    ///All jets in the containers should have pT>20GeV. Check minPt of your JetSelectorAlg in the bbll_config file.

    bool VBFVeto = false;
    double mbb = -99;
    float max_mjj = 0;
    float max_delta_eta_jj = 0;

    if (nonbjets.size() >= 2){
      bool jetsFound = false;

      for(unsigned int i=0;i<nonbjets.size()-1;i++){
        for(unsigned int j=i+1;j<nonbjets.size();j++){
	  const xAOD::Jet* nonbjet1 = nonbjets.at(i);
          const xAOD::Jet* nonbjet2 = nonbjets.at(j);

	  if (nonbjet1->pt() >= 30. * Athena::Units::GeV
            && nonbjet2->pt() >= 30. * Athena::Units::GeV) {
	    jetsFound = true;

            float mjj = (nonbjet1->p4() + nonbjet2->p4()).M();
	    float delta_eta_jj = std::abs(nonbjet1->eta() - nonbjet2->eta());

	    if (mjj > max_mjj) max_mjj = mjj;
	    if (delta_eta_jj > max_delta_eta_jj)  max_delta_eta_jj = delta_eta_jj;
	  }
        }
      }

      if (jetsFound) {
        VBFVeto = !(max_delta_eta_jj > 4 && max_mjj > 600. * Athena::Units::GeV);
      }
    }
    if(bbllCuts.exists("VBFVETO_SR1")) m_bools.at(HHBBLL::VBFVETO_SR1) = VBFVeto;

    if (bjets.size()==2)
      m_bools.at(HHBBLL::EXACTLY_TWO_B_JETS) = true;

    if (bjets.size() >= 2){
      mbb = (bjets.at(0)->p4() + bjets.at(1)->p4()).M();
    }

    if(bbllCuts.exists("DIBJET_MASS_SR2")) m_bools.at(HHBBLL::DIBJET_MASS_SR2) = (mbb > 40. * Athena::Units::GeV && mbb < 210. * Athena::Units::GeV);

  }

  void HHbbllSelectorAlg::evaluateBJetLeptonCuts
  (const ConstDataVector<xAOD::JetContainer>& bjets,
   const xAOD::ElectronContainer& electrons, const xAOD::MuonContainer& muons)
  {
    bool TWO_ISO_ELECTRONS = (electrons.size() >= 2);
    bool TWO_ISO_MUONS = (muons.size() >= 2);
    bool TWO_ISO_ELECMUs = (electrons.size() == 1 && muons.size() == 1);
    bool EXACTLY_TWO_B_JETS = bjets.size()==2;

    bool IS_ee = electrons.size() == 2 && muons.size() == 0;
    bool IS_mm = electrons.size() == 0 && muons.size() == 2;
    bool IS_em = electrons.size() == 1 && muons.size() == 1;

    m_bools.at(HHBBLL::IS_ee) = IS_ee;
    m_bools.at(HHBBLL::IS_mm) = IS_mm;
    m_bools.at(HHBBLL::IS_em) = IS_em;
    m_bools.at(HHBBLL::IS_SF) = (IS_ee || IS_mm);
    m_bools.at(HHBBLL::Pass_ll) = ((TWO_ISO_ELECTRONS || TWO_ISO_MUONS || TWO_ISO_ELECMUs) && EXACTLY_TWO_B_JETS);
  }  

  void HHbbllSelectorAlg::setRunNumberQuantities
  (unsigned int runNumber, int& year,
   std::unordered_map<HHBBLL::RunBooleans, bool>& runBoolMap) {
    // References:
    // https://atlas-tagservices.cern.ch/tagservices/RunBrowser/runBrowserReport/rBR_Period_Report.php
    // https://twiki.cern.ch/twiki/bin/view/Atlas/LowestUnprescaled

    runBoolMap.at(HHBBLL::is15) =
      266904 <= runNumber && runNumber <= 284484;
    runBoolMap.at(HHBBLL::is16) =
      296939 <= runNumber && runNumber <= 311481;
    runBoolMap.at(HHBBLL::is17PeriodB5_B8) =
      326834 <= runNumber && runNumber <= 328393;
    runBoolMap.at(HHBBLL::is22_75bunches) =
      427882 <= runNumber && runNumber < 428071;
    runBoolMap.at(HHBBLL::is23_75bunches) =
      450360 <= runNumber && runNumber < 450894;
    runBoolMap.at(HHBBLL::is23_400bunches) =
      450894 <= runNumber && runNumber < 451094;

    if(m_years.size()==1) year = m_years[0];
    else{
      if(runBoolMap.at(HHBBLL::is15)) year = 2015;
      else if(runBoolMap.at(HHBBLL::is16)) year = 2016;
    }

    // Single-lepton triggers
    if(year==2015)
      m_pt_threshold[HHBBLL::SLT][HHBBLL::ele] = 25. * Athena::Units::GeV;
    // 2022 75 bunches
    else if(runBoolMap.at(HHBBLL::is22_75bunches))
      m_pt_threshold[HHBBLL::SLT][HHBBLL::ele] = 18. * Athena::Units::GeV;
    else
      m_pt_threshold[HHBBLL::SLT][HHBBLL::ele] = 27. * Athena::Units::GeV;

    if(year==2015)
      m_pt_threshold[HHBBLL::SLT][HHBBLL::mu] = 21. * Athena::Units::GeV;
    else if(year<=2016 && year<=2018)
      m_pt_threshold[HHBBLL::SLT][HHBBLL::mu] = 27. * Athena::Units::GeV;
    else
      m_pt_threshold[HHBBLL::SLT][HHBBLL::mu] = 25. * Athena::Units::GeV;

    //Di-lepton triggers
    //ee
    if(year==2015) {
      m_pt_threshold[HHBBLL::DLT][HHBBLL::leadingele] = 13. * Athena::Units::GeV;
      m_pt_threshold[HHBBLL::DLT][HHBBLL::subleadingele] = 13. * Athena::Units::GeV;
    }
    // prescaled periods B5-B8
    // https://twiki.cern.ch/twiki/bin/view/Atlas/TrigEgammaRecommendedTriggers2017
    else if(runBoolMap.at(HHBBLL::is17PeriodB5_B8)) {
      m_pt_threshold[HHBBLL::DLT][HHBBLL::leadingele] = 25. * Athena::Units::GeV;
      m_pt_threshold[HHBBLL::DLT][HHBBLL::subleadingele] = 25. * Athena::Units::GeV;
    } else {
      m_pt_threshold[HHBBLL::DLT][HHBBLL::leadingele] = 18. * Athena::Units::GeV;
      m_pt_threshold[HHBBLL::DLT][HHBBLL::subleadingele] = 18. * Athena::Units::GeV;
    }

    //mm
    if(year==2015) {
      m_pt_threshold[HHBBLL::DLT][HHBBLL::leadingmu] = 19. * Athena::Units::GeV;
      m_pt_threshold[HHBBLL::DLT][HHBBLL::subleadingmu] = 10. * Athena::Units::GeV;
    }
    else if(year<=2016 && year<=2018) {
      m_pt_threshold[HHBBLL::DLT][HHBBLL::leadingmu] = 24. * Athena::Units::GeV;
      m_pt_threshold[HHBBLL::DLT][HHBBLL::subleadingmu] = 10. * Athena::Units::GeV;
    } else {
      m_pt_threshold[HHBBLL::DLT][HHBBLL::leadingmu] = 15. * Athena::Units::GeV;
      m_pt_threshold[HHBBLL::DLT][HHBBLL::subleadingmu] = 15. * Athena::Units::GeV;
    }

  }

}

