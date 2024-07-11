/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FullLepSelectorAlg.h"
#include <AthenaKernel/Units.h>

#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>

namespace VBSHIGGS{

  FullLepSelectorAlg::FullLepSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator) : EL::AnaAlgorithm(name, pSvcLocator){  }
  StatusCode FullLepSelectorAlg::initialize(){
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("     FullLepSelectorAlg      \n");
    ATH_MSG_INFO("*********************************\n");

    // Initialise global event filter
    ATH_CHECK (m_filterParams.initialize(m_systematicsList));
    
    ATH_CHECK (m_signaljetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_vbsjetHandle.initialize(m_systematicsList));

    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_signaljetHandle));
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

    for (auto& [key, value] : m_boolnames) {
      m_bools.emplace(key, false);
      CP::SysWriteDecorHandle<bool> whandle{value+"_%SYS%", this};
      m_Bbranches.emplace(key, whandle);
      ATH_CHECK(m_Bbranches.at(key).initialize(m_systematicsList, m_eventHandle));
    }
    ////////////////// Trigger ///////////////////
    // make trigger decorators
    for (auto trig : m_triggers){
      CP::SysReadDecorHandle<bool> deco {this, "trig"+trig, trig, "Name of trigger"};
      m_triggerdecos.emplace(trig, deco);
      ATH_CHECK(m_triggerdecos.at(trig).initialize(m_systematicsList, m_eventHandle));
    }
    //Asymmetric Lepton triggers
    //Configuration 1
    m_pt_threshold[VBSHIGGS::ASLT1_em][VBSHIGGS::leadingele] = 27. * Athena::Units::GeV;
    m_pt_threshold[VBSHIGGS::ASLT1_em][VBSHIGGS::leadingmu] = 9. * Athena::Units::GeV;

    m_pt_threshold[VBSHIGGS::ASLT1_me][VBSHIGGS::leadingmu] = 26. * Athena::Units::GeV;
    m_pt_threshold[VBSHIGGS::ASLT1_me][VBSHIGGS::leadingele] = 9. * Athena::Units::GeV;

    //Configuration 2
    m_pt_threshold[VBSHIGGS::ASLT2][VBSHIGGS::leadingele] = 18. * Athena::Units::GeV;
    m_pt_threshold[VBSHIGGS::ASLT2][VBSHIGGS::leadingmu] = 15. * Athena::Units::GeV;

    ////////////////////////////////
    std::vector<std::string> boolnameslist;
    for (const auto& [key, value] : m_boolnames) {
      boolnameslist.push_back(value);
    }
    m_vbshiggsCuts.CheckInputCutList(m_inputCutList, boolnameslist);

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
      ATH_MSG_ERROR("CutLists don't match. Please double check your configuration " + m_inputCutList[index]);
    }

    for (const auto &cut : m_inputCutKeys) {
      m_vbshiggsCuts.add(m_boolnames[cut]);
    }

    // special flag for all cuts
    ATH_CHECK (m_passallcuts.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());
    
    //After filling the CutManager, book your histograms.
    const unsigned int nbins = m_vbshiggsCuts.size() + 1; //  need an extra bin for the total num of events.
    ANA_CHECK (book (TEfficiency("AbsoluteEfficiency","Absolute Efficiency of vbshiggs cuts;Cuts;#epsilon",
                                nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("RelativeEfficiency","Relative Efficiency of vbshiggs cuts;Cuts;#epsilon",
                                nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("StandardCutFlow","StandardCutFlow of vbshiggs cuts;Cuts;#epsilon",
                                nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TH1F("EventsPassed_BinLabeling", "Events passed by each cut / Bin labeling", nbins, 0.5, nbins + 0.5)));

    return StatusCode::SUCCESS;
  }

  
  StatusCode FullLepSelectorAlg::execute(){
    // Global filter originally false
    CP::SysFilterReporterCombiner filterCombiner (m_filterParams, false);

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector()){
      CP::SysFilterReporter filter (filterCombiner, sys);

      // Retrive inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));
  
      const xAOD::JetContainer *signalJets = nullptr;
      ANA_CHECK (m_signaljetHandle.retrieve (signalJets, sys));

      const xAOD::JetContainer *vbsjets = nullptr;
      ANA_CHECK (m_vbsjetHandle.retrieve (vbsjets, sys));

      bool WPgiven = !m_isBtag.empty();
      std::vector<const xAOD::Jet*> bjets;
      for(const xAOD::Jet* jet : *signalJets) {
        if (WPgiven) {
          if (m_isBtag.get(*jet, sys) && std::abs(jet->eta())<2.5) bjets.push_back(jet);
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

      for (auto& [key, value] : m_boolnames) m_bools.at(key) = false;

      setThresholds(event, sys);
      evaluateTriggerCuts(event, electrons, muons, sys);
      leptonSelection(electrons, muons, met);
      bjetSelection(bjets);
      vbsjetsSelection(vbsjets);
      m_passallcuts.set(*event, true, sys);

      bool pass_baseline = m_bools.at(VBSHIGGS::PASS_TRIGGER) && m_bools.at(VBSHIGGS::AT_LEAST_TWO_LEPTONS) && m_bools.at(VBSHIGGS::AT_LEAST_ONE_B_JET );

      // do the CUTFLOW only with sys="" -> NOSYS
      if (sys.name()==""){
      
        // Compute total_events
        m_total_events+=1;

        // Count how many cuts the event passed and increase the relative counter
        for (const auto &cut : m_inputCutKeys) {
          if(m_vbshiggsCuts.exists(m_boolnames.at(cut))) {
            m_vbshiggsCuts(m_boolnames.at(cut)).passed = m_bools.at(cut);
            if (m_vbshiggsCuts(m_boolnames.at(cut)).passed){
              m_vbshiggsCuts(m_boolnames.at(cut)).counter+=1;
            }
          }
        }

        // Check how many consecutive cuts are passed by the event.
        unsigned int consecutive_cuts = 0;
        for (size_t i = 0; i < m_vbshiggsCuts.size(); ++i) {
          if (m_vbshiggsCuts[i].passed)
            consecutive_cuts++;
          else
            break;
        }

        // Here we basically increment the  N_events(pass_i  AND pass_i-1  AND ... AND pass_0) for the i-cut.
        for (unsigned int i=0; i<consecutive_cuts; i++) {
          m_vbshiggsCuts[i].relativeCounter+=1;
        }
      }

      // Fill syst-aware output decorators
      for (auto& [key, var] : m_bools) {
        m_Bbranches.at(key).set(*event, var, sys);
      }

      // Global event filter true if any syst passes and controls
      // if event is passed to output writing or not
      if (!m_bypass && !pass_baseline) continue;
      filter.setPassed(true);
    }
    return StatusCode::SUCCESS;
  }
  StatusCode FullLepSelectorAlg::finalize(){

    //adapt the following for each syst TODO
    ATH_MSG_INFO("Total events = " << m_total_events <<std::endl);
    ANA_CHECK (m_filterParams.finalize ());
    m_vbshiggsCuts.CheckCutResults(); // Print CheckCutResults

    if(m_saveCutFlow) {
      m_vbshiggsCuts.DoAbsoluteEfficiency(m_total_events, efficiency("AbsoluteEfficiency"));
      m_vbshiggsCuts.DoRelativeEfficiency(m_total_events, efficiency("RelativeEfficiency"));
      m_vbshiggsCuts.DoStandardCutFlow(m_total_events, efficiency("StandardCutFlow"));
      m_vbshiggsCuts.DoCutflowLabeling(m_total_events, hist("EventsPassed_BinLabeling"));

    }
    else {
      delete efficiency("AbsoluteEfficiency");
      delete efficiency("RelativeEfficiency");
      delete efficiency("StandardCutFlow");
      delete hist("EventsPassed_BinLabeling");
    }
    return StatusCode::SUCCESS;
  }


  void FullLepSelectorAlg :: leptonSelection (const xAOD::ElectronContainer* electrons, const xAOD::MuonContainer* muons, const xAOD::MissingET *met){

    int n_leptons = 0;

    const xAOD::Electron* ele0 = nullptr;
    const xAOD::Electron* ele1 = nullptr;
    for (const xAOD::Electron *electron : *electrons){
      if(!ele0) ele0 = electron;
      else if(!ele1) ele1 = electron;

      n_leptons += 1;
    }

    if (ele1) {
      if (ele0->charge() * ele1->charge() == 1)
        m_bools.at(VBSHIGGS::TWO_SS_CHARGE_LEPTONS) = true; 
      else
        m_bools.at(VBSHIGGS::TWO_OS_CHARGE_LEPTONS) = true;
        
    }

    const xAOD::Muon* mu0 = nullptr;
    const xAOD::Muon* mu1 = nullptr;
    for (const xAOD::Muon *muon : *muons){
      if(!mu0) mu0 = muon;
      else if(!mu1) mu1 = muon;

      n_leptons += 1;
    }

    if (mu1) {
      if (mu0->charge() * mu1->charge() == 1)
        m_bools.at(VBSHIGGS::TWO_SS_CHARGE_LEPTONS) = true;
      else
        m_bools.at(VBSHIGGS::TWO_OS_CHARGE_LEPTONS) = true;
        
    }
    else if (n_leptons == 2 && mu0) {
      if (ele0->charge() * mu0->charge() == 1 )
        m_bools.at(VBSHIGGS::TWO_SS_CHARGE_LEPTONS) = true;
      else
        m_bools.at(VBSHIGGS::TWO_OS_CHARGE_LEPTONS) = true;
    }
    if (n_leptons >= 2)
      m_bools.at(VBSHIGGS::AT_LEAST_TWO_LEPTONS) = true;

    if (n_leptons == 2)
      m_bools.at(VBSHIGGS::EXACTLY_TWO_LEPTONS) = true;

    // met cut
    if (met->met() > 30 * Athena::Units::GeV) m_bools.at(VBSHIGGS::PASS_MET) = true;
  }//Lepton Selection

  void FullLepSelectorAlg :: bjetSelection(std::vector<const xAOD::Jet*> bjets) {
    if (bjets.size()<2) return;

    // require exactly 2 bjets in the event
    if (bjets.size()==2) m_bools.at(VBSHIGGS::EXACTLY_TWO_B_JETS ) = true;
    if (bjets.size()>=1) m_bools.at(VBSHIGGS::AT_LEAST_ONE_B_JET ) = true;
    const xAOD::Jet * lead_bjet = bjets.at(0);
    const xAOD::Jet * sublead_bjet = bjets.at(1);
    
    //Delta R(b, b) cut 
    float min_DR_bb = 2.;
    float dR_bb = lead_bjet->p4().DeltaR(sublead_bjet->p4());
    if (dR_bb < min_DR_bb)
      m_bools.at(VBSHIGGS::PASS_DELTA_R_BB) = true;
    
    float low_mbb = 100.;
    float high_mbb = 160.;
    float mbb = (lead_bjet->p4() + sublead_bjet->p4()).M();
    if (mbb > low_mbb * Athena::Units::GeV && mbb < high_mbb * Athena::Units::GeV ){
      m_bools.at(VBSHIGGS::PASS_mBB) = true;
    }
  }//bjet selections
  
  void FullLepSelectorAlg :: vbsjetsSelection(const xAOD::JetContainer * vbsjets){
    
    if (vbsjets->size() >= 2){
      //TODO, create an object holding the vbs jets passing the mjj + deta selections
      const xAOD::Jet* vbsJet1 = vbsjets->at(0);
      const xAOD::Jet* vbsJet2 = vbsjets->at(1);;
      double mjj = (vbsJet1->p4() + vbsJet2->p4()).M();
      double dEta_jj = std::abs(vbsJet1->eta() - vbsJet2->eta());

      if ( mjj > 300 * Athena::Units::GeV  && dEta_jj > 3.0 ) {
        m_bools.at(VBSHIGGS::PASS_VBS_BASELINE) = true;
      }
    }
  }//vbsjetsSelection

  void FullLepSelectorAlg::evaluateTriggerCuts
  (const xAOD::EventInfo *event,
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

    if (ele0 || mu0) evaluateSingleLeptonTrigger(event, ele0, mu0, sys);
    if (ele1 || mu1) evaluateSingleLeptonTrigger(event, ele1, mu1, sys);
    if ((ele0 && ele1) || (mu0 && mu1)) evaluateDiLeptonTrigger(event, ele0, ele1, mu0, mu1, sys);
    if (ele0 && mu0) evaluateAsymmetricLeptonTrigger(event, ele0, mu0, sys);
    bool pass_trigger_ASLT = m_bools.at(VBSHIGGS::pass_trigger_ASLT1_em) ||
      m_bools.at(VBSHIGGS::pass_trigger_ASLT1_me) ||
      m_bools.at(VBSHIGGS::pass_trigger_ASLT2);

    if (m_bools.at(VBSHIGGS::pass_trigger_SLT) || m_bools.at(VBSHIGGS::pass_trigger_DLT) || pass_trigger_ASLT) m_bools.at(VBSHIGGS::PASS_TRIGGER) = true;
  }

  void FullLepSelectorAlg::evaluateSingleLeptonTrigger
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
      trigPassed_SET &= ele->pt() > m_pt_threshold[VBSHIGGS::SLT][VBSHIGGS::ele];
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
      trigPassed_SMT &= mu->pt() > m_pt_threshold[VBSHIGGS::SLT][VBSHIGGS::mu];
    }

    m_bools.at(VBSHIGGS::pass_trigger_SLT) |= (trigPassed_SET || trigPassed_SMT);
  }

  void FullLepSelectorAlg::evaluateDiLeptonTrigger
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
      trigPassed_DET &= ele0->pt() > m_pt_threshold[VBSHIGGS::DLT][VBSHIGGS::leadingele];
      trigPassed_DET &= ele1->pt() > m_pt_threshold[VBSHIGGS::DLT][VBSHIGGS::subleadingele];
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
      trigPassed_DMT &= mu0->pt() > m_pt_threshold[VBSHIGGS::DLT][VBSHIGGS::leadingmu];
      trigPassed_DMT &= mu1->pt() > m_pt_threshold[VBSHIGGS::DLT][VBSHIGGS::subleadingmu];
    }

    m_bools.at(VBSHIGGS::pass_trigger_DLT) = (trigPassed_DET || trigPassed_DMT);
  }
  void FullLepSelectorAlg::evaluateAsymmetricLeptonTrigger
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
      trigPassed_ASLT2 &= ele->pt() > m_pt_threshold[VBSHIGGS::ASLT2][VBSHIGGS::leadingele];
      trigPassed_ASLT2 &= mu->pt() > m_pt_threshold[VBSHIGGS::ASLT2][VBSHIGGS::leadingmu];
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
        
        trigPassed_ASLT1_em &= ele->pt() > m_pt_threshold[VBSHIGGS::ASLT1_em][VBSHIGGS::leadingele];
        trigPassed_ASLT1_em &= mu->pt() > m_pt_threshold[VBSHIGGS::ASLT1_em][VBSHIGGS::leadingmu];

      } 
      else {

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
        trigPassed_ASLT1_me &= ele->pt() > m_pt_threshold[VBSHIGGS::ASLT1_me][VBSHIGGS::leadingele];
        trigPassed_ASLT1_me &= mu->pt() > m_pt_threshold[VBSHIGGS::ASLT1_me][VBSHIGGS::leadingmu];
      }
    }

    m_bools.at(VBSHIGGS::pass_trigger_ASLT1_em) = trigPassed_ASLT1_em;
    m_bools.at(VBSHIGGS::pass_trigger_ASLT1_me) = trigPassed_ASLT1_me;
    m_bools.at(VBSHIGGS::pass_trigger_ASLT2) = trigPassed_ASLT2;
  }

  void FullLepSelectorAlg::setThresholds(const xAOD::EventInfo* event,
					const CP::SystematicSet& sys) {
    
    int year = m_year.get(*event, sys);

    // Single-lepton triggers
    if(year==2015)
      m_pt_threshold[VBSHIGGS::SLT][VBSHIGGS::ele] = 25. * Athena::Units::GeV;
    // 2022 75 bunches
    else if(m_is22_75bunches.get(*event, sys))
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
    else if(m_is17_periodB5_B8.get(*event, sys)) {
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

  }
}//name space
