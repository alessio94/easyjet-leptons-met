/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ZQGSelectorAlg.h"
#include <AthenaKernel/Units.h>

#include <AsgDataHandles/ReadDecorHandle.h>
#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>
#include <AthContainers/ConstDataVector.h>


namespace ZQG
{

  ZQGSelectorAlg::ZQGSelectorAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : EL::AnaAlgorithm(name, pSvcLocator)
  {
  }


  StatusCode ZQGSelectorAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("     ZQGSelectorAlg      \n");
    ATH_MSG_INFO("*********************************\n");

    // Initialise global event filter
    ATH_CHECK (m_filterParams.initialize(m_systematicsList));
    
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_largejetHandle.initialize(m_systematicsList));
    // if (!m_isBtag.empty()) {
    //   ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    // }

    for (const std::string &var : m_PCBTnames){
      ATH_MSG_DEBUG("initializing PCBT: " << var);
      CP::SysReadDecorHandle<int> rhandle{var, this};
      m_PCBTs.emplace(var, rhandle);
      ATH_CHECK(m_PCBTs.at(var).initialize(m_systematicsList, m_jetHandle));
    };

    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));    

    ATH_CHECK(m_year.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK(m_is17_periodB5_B8.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_is22_75bunches.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_is23_75bunches.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_is23_400bunches.initialize(m_systematicsList, m_eventHandle));
    
    ATH_CHECK (m_matchingTool.retrieve());

    ATH_CHECK(m_passTruthCutsKey.initialize());


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
    ATH_CHECK (m_pass_cuts.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());


    if(m_saveCutFlow) ATH_CHECK (initialiseCutflow());
    return StatusCode::SUCCESS;
  }


  StatusCode ZQGSelectorAlg::execute()
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
      bool isMC = event->eventType(xAOD::EventInfo::IS_SIMULATION);
      bool pass_truth_baseline = false;
      if (isMC){
        // Find truth decision
        SG::ReadDecorHandle<xAOD::TruthEventContainer, bool> m_passTruthCuts(m_passTruthCutsKey);
        // Retrieve the truth decision
        if (!m_passTruthCuts.isPresent() || m_passTruthCuts->empty()) {
          ATH_MSG_ERROR("PassTruthCuts decision is empty or not found!");
          return StatusCode::FAILURE;
        }
        pass_truth_baseline = (*m_passTruthCuts)[0];
      }
      
      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      std::string ftag2D_WP = "ftag_quantile_GN2v01_Continuous2D";
      std::vector<int> ctag_values = {1,2,3};
      std::vector<int> btag_values = {4,5,6};
      auto cjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      if(std::find(m_PCBTnames.begin(), m_PCBTnames.end(), ftag2D_WP)!=m_PCBTnames.end())
      {
        for(const xAOD::Jet* jet : *jets) {
          int pcbt = m_PCBTs.at(ftag2D_WP).get(*jet, sys);
          if (std::find(ctag_values.begin(), ctag_values.end(), pcbt)!=ctag_values.end() && std::abs(jet->eta())<2.5) cjets->push_back(jet);
          if (std::find(btag_values.begin(), btag_values.end(), pcbt)!=btag_values.end() && std::abs(jet->eta())<2.5) bjets->push_back(jet);
        }
      }

      const xAOD::JetContainer *largeJets = nullptr;
      ANA_CHECK (m_largejetHandle.retrieve (largeJets, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));
      
      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));


      m_bools.at(ZQG::IS_ee) = false;
      m_bools.at(ZQG::IS_mm) = false;
      m_bools.at(ZQG::IS_em) = false;

      m_bools.at(ZQG::pass_trigger_SLT) = false;

      m_bools.at(ZQG::PASS_TRIGGER) = false;
      m_bools.at(ZQG::EXACTLY_TWO_LEPTONS) = false;
      m_bools.at(ZQG::OPPOSITE_CHARGE_LEPTONS) = false;
      m_bools.at(ZQG::DILEPTON_MASS_WINDOW) = false;

      m_bools.at(ZQG::ONE_B_JETS) = false;
      m_bools.at(ZQG::TWO_B_JETS) = false;
      m_bools.at(ZQG::ONE_C_JETS) = false;
      m_bools.at(ZQG::TWO_C_JETS) = false;
      m_bools.at(ZQG::ONE_LARGE_JET) = false;

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

      evaluateTriggerCuts(event, ele0, ele1, mu0, mu1, m_ZQGCuts, sys);
      evaluateLeptonCuts(*electrons, *muons, m_ZQGCuts);
      evaluateBJetCuts(*bjets, m_ZQGCuts);
      evaluateCJetCuts(*cjets, m_ZQGCuts);
      evaluateLargeJetCuts(largeJets);
      
      bool pass_baseline=true;
      for (const auto& [key, value] : m_boolnames) {
        auto it = std::find(m_BASELINE_CUTS.begin(), m_BASELINE_CUTS.end(), value);
        if (it != m_BASELINE_CUTS.end()) {
          pass_baseline &= m_bools.at(key);
        }
      }
      m_pass_cuts.set(*event, pass_baseline, sys);

      // do the CUTFLOW only with sys="" -> NOSYS
      if (sys.name()=="") {

        // Compute total_events
        m_total_events+=1;

        for (const auto &cut : m_inputCutKeys) {
          if (m_ZQGCuts.exists(m_boolnames.at(cut))) {
            m_ZQGCuts(m_boolnames.at(cut)).passed = m_bools.at(cut);
            if (m_ZQGCuts(m_boolnames.at(cut)).passed) {
              m_ZQGCuts(m_boolnames.at(cut)).counter += 1;
            }
          }
        }
      }

      // Check how many consecutive cuts are passed by the event.
      unsigned int consecutive_cuts = 0;
      for (size_t i = 0; i < m_ZQGCuts.size(); ++i) {
        if (m_ZQGCuts[i].passed)
          consecutive_cuts++;
        else
          break;
      }

      // Here we basically increment the  N_events(pass_i  AND pass_i-1  AND ... AND pass_0) for the i-cut.
      // I think this is an elegant way to do it :) . Considering the difficulties a configurable cut list imposes.
      for (unsigned int i=0; i<consecutive_cuts; i++) {
        m_ZQGCuts[i].relativeCounter+=1;
      }

      for (auto& [key, var] : m_bools) {
        m_Bbranches.at(key).set(*event, var, sys);
      }

      ATH_MSG_VERBOSE("pass_baseline = " << pass_baseline);
      ATH_MSG_VERBOSE("pass_truth_baseline = " << pass_truth_baseline);

      if (pass_baseline != pass_truth_baseline) {
        ATH_MSG_VERBOSE("Baseline cuts do not match truth baseline cuts! "
                        << "pass_baseline: " << pass_baseline
                        << " pass_truth_baseline: " << pass_truth_baseline);
      }

      if (!m_bypass && (!pass_baseline && !pass_truth_baseline )) continue;
      ATH_MSG_VERBOSE("Saving event number: " << event->eventNumber() << " in run: " << event->runNumber());
      filter.setPassed(true);
    }

    return StatusCode::SUCCESS;
  }

  StatusCode ZQGSelectorAlg::finalize()
  {
    //adapt the following for each syst TODO
    ATH_MSG_INFO("Total events = " << m_total_events <<std::endl);
    ANA_CHECK (m_filterParams.finalize ());
    m_ZQGCuts.CheckCutResults(); // Print CheckCutResults

    if(m_saveCutFlow) {
      m_ZQGCuts.DoAbsoluteEfficiency(m_total_events, efficiency("AbsoluteEfficiency"));
      m_ZQGCuts.DoRelativeEfficiency(m_total_events, efficiency("RelativeEfficiency"));
      m_ZQGCuts.DoStandardCutFlow(m_total_events, efficiency("StandardCutFlow"));
      m_ZQGCuts.DoCutflowLabeling(m_total_events, hist("EventsPassed_BinLabeling"));

    }

    return StatusCode::SUCCESS;
  }

  void ZQGSelectorAlg::evaluateTriggerCuts
  (const xAOD::EventInfo *event,
   const xAOD::Electron* ele0, const xAOD::Electron* ele1,
   const xAOD::Muon* mu0, const xAOD::Muon* mu1,
   CutManager& ZQGCuts, const CP::SystematicSet& sys) {

    if (!ZQGCuts.exists("PASS_TRIGGER"))
        return;
    if (ele0 || mu0) evaluateSingleLeptonTrigger(event, ele0, mu0, sys);
    if (ele1 || mu1) evaluateSingleLeptonTrigger(event, ele1, mu1, sys);

    if (m_bools.at(ZQG::pass_trigger_SLT)) m_bools.at(ZQG::PASS_TRIGGER) = true;
  }

  void ZQGSelectorAlg::evaluateSingleLeptonTrigger
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
    else if(year==2024){
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
      trigPassed_SET &= ele->pt() > m_pt_threshold[ZQG::SLT][ZQG::ele];
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
    else if( year==2024 ){
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
      trigPassed_SMT &= mu->pt() > m_pt_threshold[ZQG::SLT][ZQG::mu];
    }

    m_bools.at(ZQG::pass_trigger_SLT) |= (trigPassed_SET || trigPassed_SMT);
  }

  void ZQGSelectorAlg::evaluateLeptonCuts
  (const xAOD::ElectronContainer& electrons, const xAOD::MuonContainer& muons,
   CutManager& ZQGCuts)
  {
    TLorentzVector ll;
    double mll = -99;

    bool OPPOSITE_CHARGE_LEPTONS = false;

    if (electrons.size() + muons.size() == 2)
      m_bools.at(ZQG::EXACTLY_TWO_LEPTONS) = true;

    if (electrons.size() >= 2)
    {
      m_bools.at(ZQG::IS_ee) = true;
      ll = electrons.at(0)->p4() + electrons.at(1)->p4();
      OPPOSITE_CHARGE_LEPTONS = electrons.at(0)->charge()*electrons.at(1)->charge() == -1;
    }

    if (muons.size() >= 2)
    {
      m_bools.at(ZQG::IS_mm) = true;
      ll = muons.at(0)->p4() + muons.at(1)->p4();
      OPPOSITE_CHARGE_LEPTONS = muons.at(0)->charge()*muons.at(1)->charge() == -1;
    }

    if (electrons.size() == 1 && muons.size() == 1)
    {
      m_bools.at(ZQG::IS_em) = true;
      ll = electrons.at(0)->p4() + muons.at(0)->p4();
      OPPOSITE_CHARGE_LEPTONS = electrons.at(0)->charge()*muons.at(0)->charge() == -1;
    }

    mll = ll.M();
    
    if(ZQGCuts.exists("OPPOSITE_CHARGE_LEPTONS")) m_bools.at(ZQG::OPPOSITE_CHARGE_LEPTONS) = OPPOSITE_CHARGE_LEPTONS;
    if(ZQGCuts.exists("DILEPTON_MASS_WINDOW")) m_bools.at(ZQG::DILEPTON_MASS_WINDOW) = ( mll >= 76.*Athena::Units::GeV && mll <= 106.*Athena::Units::GeV );


  }

  void ZQGSelectorAlg::evaluateBJetCuts
  (const ConstDataVector<xAOD::JetContainer>& bjets, CutManager& ZQGCuts)
  { 

    ///All jets in the containers should have pT>20GeV. Check minPt of your JetSelectorAlg in the ZQG_config file.
    if (bjets.size() >= 1 && ZQGCuts.exists("ONE_B_JETS")){
      m_bools.at(ZQG::ONE_B_JETS) = true;
    }

    if (bjets.size() >= 2 && ZQGCuts.exists("TWO_B_JETS")){
      m_bools.at(ZQG::TWO_B_JETS) = true;
    }

  }  
  
  void ZQGSelectorAlg::evaluateCJetCuts
  (const ConstDataVector<xAOD::JetContainer>& cjets, CutManager& ZQGCuts)
  { 

    ///All jets in the containers should have pT>20GeV. Check minPt of your JetSelectorAlg in the ZQG_config file.
    if (cjets.size() >= 1 && ZQGCuts.exists("ONE_C_JETS")){
      m_bools.at(ZQG::ONE_C_JETS) = true;
    }

    if (cjets.size() >= 2 && ZQGCuts.exists("TWO_C_JETS")){
      m_bools.at(ZQG::TWO_C_JETS) = true;
    }

  }  

  void ZQGSelectorAlg::evaluateLargeJetCuts
  (const xAOD::JetContainer *largeJets)
  {
    m_bools.at(ZQG::ONE_LARGE_JET) = largeJets->size();
  }

  void ZQGSelectorAlg::setThresholds(const xAOD::EventInfo* event,
					const CP::SystematicSet& sys) {
    
    int year = m_year.get(*event, sys);

    // Single-lepton triggers
    if(year==2015)
      m_pt_threshold[ZQG::SLT][ZQG::ele] = 25. * Athena::Units::GeV;
    // 2022 75 bunches
    else if(m_is22_75bunches.get(*event, sys))
      m_pt_threshold[ZQG::SLT][ZQG::ele] = 18. * Athena::Units::GeV;
    else
      m_pt_threshold[ZQG::SLT][ZQG::ele] = 27. * Athena::Units::GeV;

    if(year==2015)
      m_pt_threshold[ZQG::SLT][ZQG::mu] = 21. * Athena::Units::GeV;
    else if(year>=2016 && year<=2018)
      m_pt_threshold[ZQG::SLT][ZQG::mu] = 27. * Athena::Units::GeV;
    else
      m_pt_threshold[ZQG::SLT][ZQG::mu] = 25. * Athena::Units::GeV;
  }

  StatusCode ZQGSelectorAlg::initialiseCutflow(){

    std::vector<std::string> boolnameslist;
    for (const auto& [key, value] : m_boolnames) {
      boolnameslist.push_back(value);
    }
    m_ZQGCuts.CheckInputCutList(m_inputCutList, boolnameslist);

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
      m_ZQGCuts.add(m_boolnames[cut]);
    }

    //After filling the CutManager, book your histograms.
    const unsigned int nbins = m_ZQGCuts.size() + 1; //  need an extra bin for the total num of events.
    ANA_CHECK (book (TEfficiency("AbsoluteEfficiency","Absolute Efficiency of ZQG cuts;Cuts;#epsilon",
				 nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("RelativeEfficiency","Relative Efficiency of ZQG cuts;Cuts;#epsilon",
				 nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("StandardCutFlow","StandardCutFlow of ZQG cuts;Cuts;#epsilon",
				 nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TH1F("EventsPassed_BinLabeling", "Events passed by each cut / Bin labeling", nbins, 0.5, nbins + 0.5)));

    return StatusCode::SUCCESS;
  }

}

