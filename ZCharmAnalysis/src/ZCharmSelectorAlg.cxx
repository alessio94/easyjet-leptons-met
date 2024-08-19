/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ZCharmSelectorAlg.h"
#include <AthenaKernel/Units.h>

#include <AsgDataHandles/ReadDecorHandle.h>
#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>
#include <AthContainers/ConstDataVector.h>


namespace ZCC
{

  ZCharmSelectorAlg::ZCharmSelectorAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : EL::AnaAlgorithm(name, pSvcLocator)
  {
  }


  StatusCode ZCharmSelectorAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("     ZCharmSelectorAlg      \n");
    ATH_MSG_INFO("*********************************\n");

    // Initialise global event filter
    ATH_CHECK (m_filterParams.initialize(m_systematicsList));
    
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_largejetHandle.initialize(m_systematicsList));
    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));    

    ATH_CHECK(m_year.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK(m_is17_periodB5_B8.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_is22_75bunches.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_is23_75bunches.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_is23_400bunches.initialize(m_systematicsList, m_eventHandle));
    
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

    // special flag for all cuts
    ATH_CHECK (m_passallcuts.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    std::vector<std::string> boolnameslist;
    for (const auto& [key, value] : m_boolnames) {
      boolnameslist.push_back(value);
    }
    m_ZCharmCuts.CheckInputCutList(m_inputCutList, boolnameslist);

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
      m_ZCharmCuts.add(m_boolnames[cut]);
    }

    //After filling the CutManager, book your histograms.
    const unsigned int nbins = m_ZCharmCuts.size() + 1; //  need an extra bin for the total num of events.
    ANA_CHECK (book (TEfficiency("AbsoluteEfficiency","Absolute Efficiency of ZCharm cuts;Cuts;#epsilon",
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("RelativeEfficiency","Relative Efficiency of ZCharm cuts;Cuts;#epsilon",
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("StandardCutFlow","StandardCutFlow of ZCharm cuts;Cuts;#epsilon",
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TH1F("EventsPassed_BinLabeling", "Events passed by each cut / Bin labeling", nbins, 0.5, nbins + 0.5)));

    return StatusCode::SUCCESS;
  }


  StatusCode ZCharmSelectorAlg::execute()
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
          if (m_isBtag.get(*jet, sys) && std::abs(jet->eta())<2.5) bjets->push_back(jet);
          else nonbjets->push_back(jet);
        }
      }

      const xAOD::JetContainer *largeJets = nullptr;
      ANA_CHECK (m_largejetHandle.retrieve (largeJets, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));
      
      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      const xAOD::MissingETContainer *metCont = nullptr;
      ANA_CHECK (m_metHandle.retrieve (metCont, sys));
      const xAOD::MissingET* met = (*metCont)["Final"]; // To check
      if (!met) {
        ATH_MSG_ERROR("Could not retrieve MET");
        return StatusCode::FAILURE;
      }

      m_bools.at(ZCC::IS_ee) = false;
      m_bools.at(ZCC::IS_mm) = false;
      m_bools.at(ZCC::IS_em) = false;

      m_bools.at(ZCC::pass_trigger_SLT) = false;
      m_bools.at(ZCC::pass_trigger_DLT) = false;

      m_bools.at(ZCC::PASS_TRIGGER) = false;
      m_bools.at(ZCC::EXACTLY_TWO_LEPTONS) = false;
      m_bools.at(ZCC::OPPOSITE_CHARGE_LEPTONS) = false;
      m_bools.at(ZCC::DILEPTON_MASS_WINDOW) = false;

      m_bools.at(ZCC::MET) = false;
      m_bools.at(ZCC::ONE_B_JETS) = false;
      m_bools.at(ZCC::TWO_B_JETS) = false;
      m_bools.at(ZCC::ONE_LARGE_JET) = false;

      setThresholds(event, sys);

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

      evaluateTriggerCuts(event, ele0, ele1, mu0, mu1, m_ZCharmCuts, sys);
      evaluateLeptonCuts(*electrons, *muons, met, m_ZCharmCuts);
      evaluateBJetCuts(*bjets, m_ZCharmCuts);
      evaluateLargeJetCuts(largeJets);
      
      bool passedall = true;
      for (const auto& [key, value] : m_boolnames) {
        auto it = std::find(m_STANDARD_CUTS.begin(), m_STANDARD_CUTS.end(), value);
        if (it != m_STANDARD_CUTS.end()) {
          passedall &= m_bools.at(key);
        }
      }
      m_passallcuts.set(*event, passedall, sys);

      bool pass_baseline=false;
      if(m_bools.at(ZCC::PASS_TRIGGER) && m_bools.at(ZCC::EXACTLY_TWO_LEPTONS) && m_bools.at(ZCC::OPPOSITE_CHARGE_LEPTONS) && m_bools.at(ZCC::DILEPTON_MASS_WINDOW) && m_bools.at(ZCC::MET) && m_bools.at(ZCC::ONE_B_JETS)) pass_baseline=true;
      
      // do the CUTFLOW only with sys="" -> NOSYS
      if (sys.name()=="") {

        // Compute total_events
        m_total_events+=1;

        for (const auto &cut : m_inputCutKeys) {
          if (m_ZCharmCuts.exists(m_boolnames.at(cut))) {
            m_ZCharmCuts(m_boolnames.at(cut)).passed = m_bools.at(cut);
            if (m_ZCharmCuts(m_boolnames.at(cut)).passed) {
              m_ZCharmCuts(m_boolnames.at(cut)).counter += 1;
            }
          }
        }
      }

      // Check how many consecutive cuts are passed by the event.
      unsigned int consecutive_cuts = 0;
      for (size_t i = 0; i < m_ZCharmCuts.size(); ++i) {
        if (m_ZCharmCuts[i].passed)
          consecutive_cuts++;
        else
          break;
      }

      // Here we basically increment the  N_events(pass_i  AND pass_i-1  AND ... AND pass_0) for the i-cut.
      // I think this is an elegant way to do it :) . Considering the difficulties a configurable cut list imposes.
      for (unsigned int i=0; i<consecutive_cuts; i++) {
        m_ZCharmCuts[i].relativeCounter+=1;
      }

      for (auto& [key, var] : m_bools) {
        m_Bbranches.at(key).set(*event, var, sys);
      }

      if (!m_bypass && !pass_baseline) continue;
      filter.setPassed(true);
    }
    return StatusCode::SUCCESS;
  }

  StatusCode ZCharmSelectorAlg::finalize()
  {
    //adapt the following for each syst TODO
    ATH_MSG_INFO("Total events = " << m_total_events <<std::endl);
    ANA_CHECK (m_filterParams.finalize ());
    m_ZCharmCuts.CheckCutResults(); // Print CheckCutResults

    if(m_saveCutFlow) {
      m_ZCharmCuts.DoAbsoluteEfficiency(m_total_events, efficiency("AbsoluteEfficiency"));
      m_ZCharmCuts.DoRelativeEfficiency(m_total_events, efficiency("RelativeEfficiency"));
      m_ZCharmCuts.DoStandardCutFlow(m_total_events, efficiency("StandardCutFlow"));
      m_ZCharmCuts.DoCutflowLabeling(m_total_events, hist("EventsPassed_BinLabeling"));

    }
    else {
      delete efficiency("AbsoluteEfficiency");
      delete efficiency("RelativeEfficiency");
      delete efficiency("StandardCutFlow");
      delete hist("EventsPassed_BinLabeling");
    }

    return StatusCode::SUCCESS;
  }

  void ZCharmSelectorAlg::evaluateTriggerCuts
  (const xAOD::EventInfo *event,
   const xAOD::Electron* ele0, const xAOD::Electron* ele1,
   const xAOD::Muon* mu0, const xAOD::Muon* mu1,
   CutManager& ZCharmCuts, const CP::SystematicSet& sys) {

    if (!ZCharmCuts.exists("PASS_TRIGGER"))
        return;

    if (ele0 || mu0) evaluateSingleLeptonTrigger(event, ele0, mu0, sys);
    if (ele1 || mu1) evaluateSingleLeptonTrigger(event, ele1, mu1, sys);
    if ((ele0 && ele1) || (mu0 && mu1)) evaluateDiLeptonTrigger(event, ele0, ele1, mu0, mu1, sys);

    if (m_bools.at(ZCC::pass_trigger_SLT) || m_bools.at(ZCC::pass_trigger_DLT)) m_bools.at(ZCC::PASS_TRIGGER) = true;
  }

  void ZCharmSelectorAlg::evaluateSingleLeptonTrigger
  (const xAOD::EventInfo *event,
   const xAOD::Electron *ele, const xAOD::Muon *mu,
   const CP::SystematicSet& sys)
  {
    // Check single electron triggers
    std::vector<std::string> single_ele_paths;

    int year = m_year.get(*event, sys);
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
    else if(m_is22_75bunches.get(*event, sys)){
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
    else if(m_is23_75bunches.get(*event, sys)){
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
      trigPassed_SET &= ele->pt() > m_pt_threshold[ZCC::SLT][ZCC::ele];
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
	    !m_is22_75bunches.get(*event, sys) &&
	    !m_is23_75bunches.get(*event, sys) &&
	    !m_is23_400bunches.get(*event, sys)){
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
      trigPassed_SMT &= mu->pt() > m_pt_threshold[ZCC::SLT][ZCC::mu];
    }

    m_bools.at(ZCC::pass_trigger_SLT) |= (trigPassed_SET || trigPassed_SMT);
  }

  void ZCharmSelectorAlg::evaluateDiLeptonTrigger
  (const xAOD::EventInfo *event,
   const xAOD::Electron *ele0, const xAOD::Electron *ele1,
   const xAOD::Muon *mu0, const xAOD::Muon *mu1,
   const CP::SystematicSet& sys)
  {
    std::vector<std::string> di_ele_paths;

    int year = m_year.get(*event, sys);
    if(year==2015){
      di_ele_paths = {"HLT_2e12_lhloose_L12EM10VH"};
    }
    else if(year==2016){
      di_ele_paths = {"HLT_2e17_lhvloose_nod0"};
    }
    else if(m_is17_periodB5_B8.get(*event, sys)){
      di_ele_paths = {
        "HLT_2e24_lhvloose_nod0"
      };
    }
    else if(2017<=year && year<=2018){
      di_ele_paths = {
        "HLT_2e17_lhvloose_nod0_L12EM15VHI", "HLT_2e24_lhvloose_nod0"
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
      trigPassed_DET &= ele0->pt() > m_pt_threshold[ZCC::DLT][ZCC::leadingele];
      trigPassed_DET &= ele1->pt() > m_pt_threshold[ZCC::DLT][ZCC::subleadingele];
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
      trigPassed_DMT &= mu0->pt() > m_pt_threshold[ZCC::DLT][ZCC::leadingmu];
      trigPassed_DMT &= mu1->pt() > m_pt_threshold[ZCC::DLT][ZCC::subleadingmu];
    }

    m_bools.at(ZCC::pass_trigger_DLT) = (trigPassed_DET || trigPassed_DMT);
  }


  void ZCharmSelectorAlg::evaluateLeptonCuts
  (const xAOD::ElectronContainer& electrons, const xAOD::MuonContainer& muons, const xAOD::MissingET* met,
   CutManager& ZCharmCuts)
  {
    TLorentzVector ll;
    double mll = -99;
    double pTll = -99;

    bool OPPOSITE_CHARGE_LEPTONS = false;

    if (electrons.size() + muons.size() == 2)
      m_bools.at(ZCC::EXACTLY_TWO_LEPTONS) = true;

    if (electrons.size() >= 2)
    {
      m_bools.at(ZCC::IS_ee) = true;
      ll = electrons.at(0)->p4() + electrons.at(1)->p4();
      OPPOSITE_CHARGE_LEPTONS = electrons.at(0)->charge()*electrons.at(1)->charge() == -1;
    }

    if (muons.size() >= 2)
    {
      m_bools.at(ZCC::IS_mm) = true;
      ll = muons.at(0)->p4() + muons.at(1)->p4();
      OPPOSITE_CHARGE_LEPTONS = muons.at(0)->charge()*muons.at(1)->charge() == -1;
    }

    if (electrons.size() == 1 && muons.size() == 1)
    {
      m_bools.at(ZCC::IS_em) = true;
      ll = electrons.at(0)->p4() + muons.at(0)->p4();
      OPPOSITE_CHARGE_LEPTONS = electrons.at(0)->charge()*muons.at(0)->charge() == -1;
    }

    mll = ll.M();
    pTll = ll.Pt();
    
    if(ZCharmCuts.exists("OPPOSITE_CHARGE_LEPTONS")) m_bools.at(ZCC::OPPOSITE_CHARGE_LEPTONS) = OPPOSITE_CHARGE_LEPTONS;
    if(ZCharmCuts.exists("DILEPTON_MASS_WINDOW")) m_bools.at(ZCC::DILEPTON_MASS_WINDOW) = ( mll >= 76.*Athena::Units::GeV && mll <= 106.*Athena::Units::GeV );

    if( (pTll < 150 * Athena::Units::GeV && pTll != -99 && met->met() < 60 * Athena::Units::GeV) || pTll >= 150 * Athena::Units::GeV ){
      if(ZCharmCuts.exists("MET")) m_bools.at(ZCC::MET) = true;
    }

  }

  void ZCharmSelectorAlg::evaluateBJetCuts
  (const ConstDataVector<xAOD::JetContainer>& bjets, CutManager& ZCharmCuts)
  { 

    ///All jets in the containers should have pT>20GeV. Check minPt of your JetSelectorAlg in the ZCharm_config file.
    if (bjets.size() >= 1 && ZCharmCuts.exists("ONE_B_JETS")){
      m_bools.at(ZCC::ONE_B_JETS) = true;
    }

    if (bjets.size() >= 2 && ZCharmCuts.exists("TWO_B_JETS")){
      m_bools.at(ZCC::TWO_B_JETS) = true;
    }

  }  

  void ZCharmSelectorAlg::evaluateLargeJetCuts
  (const xAOD::JetContainer *largeJets)
  {
    m_bools.at(ZCC::ONE_LARGE_JET) = largeJets->size();
  }

  void ZCharmSelectorAlg::setThresholds(const xAOD::EventInfo* event,
					const CP::SystematicSet& sys) {
    
    int year = m_year.get(*event, sys);

    // Single-lepton triggers
    if(year==2015)
      m_pt_threshold[ZCC::SLT][ZCC::ele] = 25. * Athena::Units::GeV;
    // 2022 75 bunches
    else if(m_is22_75bunches.get(*event, sys))
      m_pt_threshold[ZCC::SLT][ZCC::ele] = 18. * Athena::Units::GeV;
    else
      m_pt_threshold[ZCC::SLT][ZCC::ele] = 27. * Athena::Units::GeV;

    if(year==2015)
      m_pt_threshold[ZCC::SLT][ZCC::mu] = 21. * Athena::Units::GeV;
    else if(year<=2016 && year<=2018)
      m_pt_threshold[ZCC::SLT][ZCC::mu] = 27. * Athena::Units::GeV;
    else
      m_pt_threshold[ZCC::SLT][ZCC::mu] = 25. * Athena::Units::GeV;

    //Di-lepton triggers
    //ee
    if(year==2015) {
      m_pt_threshold[ZCC::DLT][ZCC::leadingele] = 13. * Athena::Units::GeV;
      m_pt_threshold[ZCC::DLT][ZCC::subleadingele] = 13. * Athena::Units::GeV;
    }
    // prescaled periods B5-B8
    // https://twiki.cern.ch/twiki/bin/view/Atlas/TrigEgammaRecommendedTriggers2017
    else if(m_is17_periodB5_B8.get(*event, sys)) {
      m_pt_threshold[ZCC::DLT][ZCC::leadingele] = 25. * Athena::Units::GeV;
      m_pt_threshold[ZCC::DLT][ZCC::subleadingele] = 25. * Athena::Units::GeV;
    } else {
      m_pt_threshold[ZCC::DLT][ZCC::leadingele] = 18. * Athena::Units::GeV;
      m_pt_threshold[ZCC::DLT][ZCC::subleadingele] = 18. * Athena::Units::GeV;
    }

    //mm
    if(year==2015) {
      m_pt_threshold[ZCC::DLT][ZCC::leadingmu] = 19. * Athena::Units::GeV;
      m_pt_threshold[ZCC::DLT][ZCC::subleadingmu] = 10. * Athena::Units::GeV;
    }
    else if(year<=2016 && year<=2018) {
      m_pt_threshold[ZCC::DLT][ZCC::leadingmu] = 24. * Athena::Units::GeV;
      m_pt_threshold[ZCC::DLT][ZCC::subleadingmu] = 10. * Athena::Units::GeV;
    } else {
      m_pt_threshold[ZCC::DLT][ZCC::leadingmu] = 15. * Athena::Units::GeV;
      m_pt_threshold[ZCC::DLT][ZCC::subleadingmu] = 15. * Athena::Units::GeV;
    }

  }

}

