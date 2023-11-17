/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Carl Gwilliam

#include "HHbbttSelectorAlg.h"
#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>
#include <AthContainers/ConstDataVector.h>

namespace HHBBTT
{
  HHbbttSelectorAlg ::HHbbttSelectorAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
    : EL::AnaAlgorithm(name, pSvcLocator)
  {

  }

  StatusCode HHbbttSelectorAlg ::initialize()
  {

    // Initialise global event filter
    ATH_CHECK (m_filterParams.initialize(m_systematicsList));

    // Read syst-aware input handles
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_tauHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    ATH_CHECK (m_runNumber.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst-aware output decorators
    for (const std::string &var : m_Bvarnames){
      CP::SysWriteDecorHandle<bool> whandle{var+"_%SYS%", this};
      m_Bbranches.emplace(var, whandle);
      ATH_CHECK(m_Bbranches.at(var).initialize(m_systematicsList, m_eventHandle));
    };

    m_tauWPDecorKey = m_tauHandle.getNamePattern() +
      ".baselineSelection_" + m_tauWPName;
    m_eleWPDecorKey = m_electronHandle.getNamePattern() +
      ".baselineSelection_" + m_eleWPName;
    m_muonWPDecorKey = m_muonHandle.getNamePattern() +
      ".baselineSelection_" + m_muonWPName;

    ATH_CHECK (m_tauWPDecorKey.initialize());
    ATH_CHECK (m_eleWPDecorKey.initialize());
    ATH_CHECK (m_muonWPDecorKey.initialize());

    ATH_CHECK(m_selected_el.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK(m_selected_mu.initialize(m_systematicsList, m_muonHandle));
    ATH_CHECK(m_selected_tau.initialize(m_systematicsList, m_tauHandle));

    // make trigger decorators
    for (auto trig : m_triggers){
      CP::SysReadDecorHandle<bool> deco {this, "trig"+trig, trig, "Name of trigger"};
      m_triggerdecos.emplace(trig, deco);
      ATH_CHECK(m_triggerdecos.at(trig).initialize(m_systematicsList, m_eventHandle));
    }

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());    
    for ( auto name : m_channel_names){
      if( name == "lephad") m_channels.push_back(HHBBTT::LepHad);
      else if ( name == "hadhad") m_channels.push_back(HHBBTT::HadHad);
      else{
        ATH_MSG_ERROR("Unknown channel");
        return StatusCode::FAILURE;
      }
    }

    //finding which years are set in the config
    is15 = std::find(m_years.begin(), m_years.end(), 2015) != m_years.end();
    is16 = std::find(m_years.begin(), m_years.end(), 2016) != m_years.end();
    is17 = std::find(m_years.begin(), m_years.end(), 2017) != m_years.end();
    is18 = std::find(m_years.begin(), m_years.end(), 2018) != m_years.end();
    is22 = std::find(m_years.begin(), m_years.end(), 2022) != m_years.end();
    is23 = std::find(m_years.begin(), m_years.end(), 2023) != m_years.end();

    return StatusCode::SUCCESS;
  }

  StatusCode HHbbttSelectorAlg ::execute()
  {

    // Global filter originally false
    CP::SysFilterReporterCombiner filterCombiner (m_filterParams, false);

    SG::ReadDecorHandle<xAOD::TauJetContainer, char> tauWPDecorHandle(m_tauWPDecorKey);
    SG::ReadDecorHandle<xAOD::ElectronContainer, char> eleWPDecorHandle(m_eleWPDecorKey);
    SG::ReadDecorHandle<xAOD::MuonContainer, char> muonWPDecorHandle(m_muonWPDecorKey);

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector()){
      CP::SysFilterReporter filter (filterCombiner, sys);

      // Retrive inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      const xAOD::TauJetContainer *taus = nullptr;
      ANA_CHECK (m_tauHandle.retrieve (taus, sys));

      const xAOD::MissingETContainer *metCont = nullptr;
      ANA_CHECK (m_metHandle.retrieve (metCont, sys));
      const xAOD::MissingET* met = (*metCont)["Final"];
      if (!met) {
	ATH_MSG_ERROR("Could not retrieve MET");
	return StatusCode::FAILURE;	
      }

      // Apply selection

      // Cuts - just to test for now
      //if (taus->size() != 2) continue;
      //if (! (taus->at(0)->pt() > 60000)) continue;

      std::vector<std::string> trigger_channels = {"SLT", "LTT", "STT", "DTT"};
      std::vector<std::string> vars = {"ele", "mu", "leadingtau", "subleadingtau", "leadingjet", "subleadingjet"};
      std::vector<std::string> jvars = {"leadingjet", "subleadingjet"};

      for (const auto& trigger_channel : trigger_channels) {
          for (const auto& var : vars) {
              m_pt_threshold[trigger_channel][var] = 0.0;
          }
      }

      applyTriggerSelection(taus, event, sys);
      m_Bbranches.at("pass_trigger_SLT").set(*event, trigPassed_SLT, sys);
      m_Bbranches.at("pass_trigger_LTT").set(*event, trigPassed_LTT, sys);
      m_Bbranches.at("pass_trigger_STT").set(*event, trigPassed_STT, sys);
      m_Bbranches.at("pass_trigger_DTT").set(*event, trigPassed_DTT, sys);

      // flags
      TWO_JETS = false;
      TWO_BJETS = false;
      MBB_MASS = false;
      // flags for lephad
      N_LEPTONS_CUT_LEPHAD = false;
      ONE_TAU = false;
      OS_CHARGE_LEPHAD = false;
      pass_baseline_SLT = false;
      pass_baseline_LTT = false;
      pass_SLT = false;
      pass_LTT = false;
      // flags for hadhad
      N_LEPTONS_CUT_HADHAD = false;
      TWO_TAU = false;
      OS_CHARGE_HADHAD = false;
      pass_baseline_STT = false;
      pass_baseline_DTT = false;
      pass_STT = false;
      pass_DTT = false;

      //************
      // lepton
      //************
      int n_leptons = 0;
      int n_looseleptons = 0;
      int charge_lepton = 0;
      bool lep_ptcut_SLT = false;
      bool lep_ptcut_LTT = false;
      for (const xAOD::Electron *electron : *electrons)
      {
        bool passElectronWP = eleWPDecorHandle(*electron);
        m_selected_el.set(*electron, false, sys);
        if (passElectronWP && electron->pt() > m_pt_threshold["LTT"]["ele"])
        {
          if (electron->pt() > m_pt_threshold["LTT"]["ele"] && electron->pt() < m_pt_threshold["SLT"]["ele"])
            lep_ptcut_LTT = true;
          else if (electron->pt() > m_pt_threshold["SLT"]["ele"])
            lep_ptcut_SLT = true;
          charge_lepton = electron->charge();
          m_selected_el.set(*electron, true, sys);
          n_leptons += 1;
        }
        else
          n_looseleptons += 1;
      }

      for (const xAOD::Muon *muon : *muons)
      {
        bool passMuonWP = muonWPDecorHandle(*muon);
        m_selected_mu.set(*muon, false, sys);
        if (passMuonWP && std::abs(muon->eta()) < 2.5
	    && muon->pt() > m_pt_threshold["LTT"]["mu"])
        {
          if (muon->pt() >  m_pt_threshold["LTT"]["mu"]
	      && muon->pt() <  m_pt_threshold["SLT"]["mu"])
            lep_ptcut_LTT = true;
          else if (muon->pt() >  m_pt_threshold["SLT"]["mu"])
            lep_ptcut_SLT = true;
          charge_lepton = muon->charge();
          m_selected_mu.set(*muon, true, sys);
          n_leptons += 1;
        }
        else
          n_looseleptons += 1;
      }

      if (n_leptons == 1 && n_looseleptons == 0)
        N_LEPTONS_CUT_LEPHAD = true;

      if (n_leptons == 0 && n_looseleptons == 0)
        N_LEPTONS_CUT_HADHAD = true;

      //************
      // taujet
      //************
      int n_taus = 0;
      int charge_tau0 = 0;
      int charge_tau1 = 0;
      bool tau_ptcut_SLT = false;
      bool tau_ptcut_LTT = false;
      bool tau_ptcut_STT_lead = false;
      int tau_ptcut_STT_sublead = 0;
      bool tau_ptcut_DTT_lead = false;
      int tau_ptcut_DTT_sublead = 0;
      bool tau_ptcut_STT = false;
      bool tau_ptcut_DTT = false;
      for (const xAOD::TauJet *tau : *taus)
      {
        bool isTauID = tauWPDecorHandle(*tau);
        m_selected_tau.set(*tau, false, sys);
        if (isTauID && tau->pt() > 20000)
        {
          if (std::abs(tau->eta()) < 2.3) {
            if (tau->pt() >  m_pt_threshold["SLT"]["leadingtau"])
              tau_ptcut_SLT = true;
            if (tau->pt() > m_pt_threshold["LTT"]["leadingtau"])
              tau_ptcut_LTT = true;
          }
          if (tau->pt() > m_pt_threshold["STT"]["leadingtau"])
            tau_ptcut_STT_lead = true;
          if (tau->pt() > m_pt_threshold["STT"]["subleadingtau"])
            tau_ptcut_STT_sublead++;
          if (tau->pt() > m_pt_threshold["DTT"]["leadingtau"])
            tau_ptcut_DTT_lead = true;
          if (tau->pt() > m_pt_threshold["DTT"]["subleadingtau"])
            tau_ptcut_DTT_sublead++;
          m_selected_tau.set(*tau, true, sys);
          n_taus += 1;
          if (charge_tau0 == 0)
            charge_tau0 = tau->charge();
          else
            charge_tau1 = tau->charge();
        }
      }

      if (n_taus == 1)
        ONE_TAU = true;

      if (n_taus == 2) {
        TWO_TAU = true;
        if (tau_ptcut_STT_lead && tau_ptcut_STT_sublead >= 2)
          tau_ptcut_STT = true;
        if (tau_ptcut_DTT_lead && tau_ptcut_DTT_sublead >= 2)
          tau_ptcut_DTT = true;
      }

      //************
      // jet
      //************
      int n_jets = 0;
      bool jet_ptcut_SLT = false;
      bool jet_ptcut_LTT = false;
      bool jet_ptcut_STT = false;
      bool jet_ptcut_DTT = false;
      std::unordered_map<std::string, std::unordered_map<std::string, bool>> jet_ptcut;
      for (const auto& trigger_channel : trigger_channels) {
          for (const auto& jvar : jvars) {
              jet_ptcut[trigger_channel][jvar] = false;
          }
      }
      bool jet_deltaR_DTT = false;
      TLorentzVector bb(0, 0, 0, 0);
      float mbb = 0;
      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>>(
          SG::VIEW_ELEMENTS);

      for (const xAOD::Jet *jet : *jets)
      {
        if (std::abs(jet->eta()) < 2.5)
        {
          n_jets += 1;
          if (WPgiven)
          {
            if (m_isBtag.get(*jet, sys))
              bjets->push_back(jet);
          }
        }
      }

      if (n_jets >= 2)
      {
        TWO_JETS = true;
        for (const auto& trigger_channel : trigger_channels) {
            if (jets->at(0)->pt() > m_pt_threshold[trigger_channel]["leadingjet"])
              jet_ptcut[trigger_channel]["leadingjet"] = true;
            if (jets->at(1)->pt() > m_pt_threshold[trigger_channel]["subleadingjet"])
              jet_ptcut[trigger_channel]["subleadingjet"] = true;
        }
        if (DTT_DeltaR_cut)
        {
          if (jets->at(0)->p4().DeltaR(jets->at(1)->p4()) <= 2.5) 
              jet_deltaR_DTT = true;
        }
        else  jet_deltaR_DTT = true;
        if (bjets->size() == 2)
        {
          TWO_BJETS = true;
          bb = bjets->at(0)->p4() + bjets->at(1)->p4();
          mbb = bb.M();
        }
      }
      if (jet_ptcut["SLT"]["leadingjet"] && jet_ptcut["SLT"]["subleadingjet"])
          jet_ptcut_SLT = true;
      if (jet_ptcut["LTT"]["leadingjet"] && jet_ptcut["LTT"]["subleadingjet"])
          jet_ptcut_LTT = true;
      if (jet_ptcut["STT"]["leadingjet"] && jet_ptcut["STT"]["subleadingjet"])
          jet_ptcut_STT = true;
      if (jet_ptcut["DTT"]["leadingjet"] && jet_ptcut["DTT"]["subleadingjet"] && jet_deltaR_DTT)
          jet_ptcut_DTT = true;

      //****************
      // event level info
      //****************
      if (mbb < 150000)
        MBB_MASS = true;
      if (charge_tau0 != charge_lepton)
        OS_CHARGE_LEPHAD = true;
      if (charge_tau0 == - charge_tau1)
        OS_CHARGE_HADHAD = true;

      if (N_LEPTONS_CUT_LEPHAD && ONE_TAU && TWO_JETS){
        // SLT
        if (lep_ptcut_SLT && tau_ptcut_SLT && jet_ptcut_SLT){
           pass_baseline_SLT = true;
           if (trigPassed_SLT && TWO_BJETS && MBB_MASS && OS_CHARGE_LEPHAD)
              pass_SLT = true;
        }
        // LTT
        if (lep_ptcut_LTT && tau_ptcut_LTT && jet_ptcut_LTT){
           pass_baseline_LTT = true;
           if (trigPassed_LTT && TWO_BJETS && MBB_MASS && OS_CHARGE_LEPHAD)
              pass_LTT = true;
        }
      }
      if (N_LEPTONS_CUT_HADHAD && TWO_TAU && TWO_JETS){
        // STT
        if (tau_ptcut_STT && jet_ptcut_STT){
           pass_baseline_STT = true;
           if (trigPassed_STT && TWO_BJETS && OS_CHARGE_HADHAD)
              pass_STT = true;
        }
        // DTT
        if (tau_ptcut_DTT && jet_ptcut_DTT){
           pass_baseline_DTT = true;
           if (trigPassed_DTT && TWO_BJETS && OS_CHARGE_HADHAD)
              pass_DTT = true;
        }
      }

      m_Bbranches.at("pass_baseline_SLT").set(*event, pass_baseline_SLT, sys);
      m_Bbranches.at("pass_baseline_LTT").set(*event, pass_baseline_LTT, sys);
      m_Bbranches.at("pass_baseline_STT").set(*event, pass_baseline_STT, sys);
      m_Bbranches.at("pass_baseline_DTT").set(*event, pass_baseline_DTT, sys);
      m_Bbranches.at("pass_SLT").set(*event, pass_SLT, sys);
      m_Bbranches.at("pass_LTT").set(*event, pass_LTT, sys);
      m_Bbranches.at("pass_STT").set(*event, pass_STT, sys);
      m_Bbranches.at("pass_DTT").set(*event, pass_DTT, sys);

      bool pass = false;
      for(const auto& channel : m_channels){
       if(channel == HHBBTT::LepHad) pass |= (pass_SLT || pass_LTT);
       else if(channel == HHBBTT::HadHad) pass |= (pass_STT || pass_DTT);
      }
      if (!m_bypass && !pass) continue;

      // Global event filter true if any syst passes and controls
      // if event is passed to output writing or not
      filter.setPassed(true);
    }

    return StatusCode::SUCCESS;
  }

  StatusCode HHbbttSelectorAlg::finalize() {
    ANA_CHECK (m_filterParams.finalize ());
    return StatusCode::SUCCESS;
  }

  void HHbbttSelectorAlg::applyTriggerSelection(const xAOD::TauJetContainer* taus, const xAOD::EventInfo* event, const CP::SystematicSet& sys){
    //************
    // trigger selecton
    //************

    unsigned int rdmNumber=0;
    static const SG::AuxElement::ConstAccessor<unsigned int> randomrunnumber("RandomRunNumber");

    if(m_isMC){
      if(randomrunnumber.isAvailable(*event)){
        rdmNumber = randomrunnumber(*(event));
      }
    }
    else{
      rdmNumber = m_runNumber.get(*event, sys);
    }

    // References:
    // https://atlas-tagservices.cern.ch/tagservices/RunBrowser/runBrowserReport/rBR_Period_Report.php
    // https://twiki.cern.ch/twiki/bin/view/Atlas/LowestUnprescaled
    //
    is16PeriodA = 296939 <= rdmNumber && rdmNumber <= 300287;
    is16PeriodB_D3 = 300345 <= rdmNumber && rdmNumber <= 302872;
    is16PeriodD4_end = 302919 <= rdmNumber && rdmNumber <= 311481;
    is17PeriodB1_B4 = 325713 <= rdmNumber && rdmNumber <= 326695;
    is17PeriodB5_B7 = 326834 <= rdmNumber && rdmNumber <= 327490;
    is17PeriodB8_end = 327582 <= rdmNumber && rdmNumber <= 341649;
    is18PeriodB_end = 348885 <= rdmNumber && rdmNumber <= 364485;
    is18PeriodK_end = 355529 <= rdmNumber && rdmNumber <= 364485;

    // Runs in which the L1Topo was mistakingly disabled
    l1topo_disabled = (rdmNumber == 336506) || (rdmNumber == 336548) ||
                      (rdmNumber == 336567);

    // lephad
    trigPassed_SLT = false;
    trigPassed_LTT = false;

    // hadhad
    trigPassed_STT = false;
    trigPassed_DTT = false;
    DTT_DeltaR_cut = false;

    // only run trigger selection if in channel
    for (const auto &channel : m_channels){
      if (channel == HHBBTT::LepHad)
      {
        applyLepHadTriggerSelection(taus, event,
                                    sys); // maybe split this as well
      }
      else if (channel == HHBBTT::HadHad)
      {
        applySingleTauTriggerSelection(event, sys);
        applyDiTauTriggerSelection(event, sys);
      }
    }
  }

  void HHbbttSelectorAlg ::applyLepHadTriggerSelection(
      const xAOD::TauJetContainer *taus, const xAOD::EventInfo *event,
      const CP::SystematicSet &sys)
  {
    bool trigPassed_SET = false;
    bool trigPassed_SMT = false;
    bool trigPassed_ETT = false;
    bool trigPassed_MTT = false;
    bool trigPassed_MTT_low  = false;
    bool trigPassed_MTT_high = false;

    std::vector<std::string> single_ele_paths;
    std::vector<std::string> single_mu_paths;
    std::vector<std::string> ele_tau_paths;
    std::vector<std::string> ele_tau_paths_4J12;
    std::vector<std::string> mu_tau_paths;
    std::vector<std::string> mu_tau_paths_low;
    std::vector<std::string> mu_tau_paths_high;

    // SLT
    m_pt_threshold["SLT"]["leadingtau"] = 20000;
    m_pt_threshold["SLT"]["leadingjet"] = 45000;
    m_pt_threshold["SLT"]["subleadingjet"] = 20000;
    if(is15){
      m_pt_threshold["SLT"]["ele"] = 25000;
      m_pt_threshold["SLT"]["mu"] = 21000;
      single_ele_paths = {"trigPassed_HLT_e24_lhmedium_L1EM20VH", "trigPassed_HLT_e60_lhmedium", "trigPassed_HLT_e120_lhloose"};
      single_mu_paths = {"trigPassed_HLT_mu20_iloose_L1MU15", "trigPassed_HLT_mu50"};
    }
    else if(is16 || is17 || is18){
      m_pt_threshold["SLT"]["ele"] = 27000;
      m_pt_threshold["SLT"]["mu"] = 27000;
      single_ele_paths = {"trigPassed_HLT_e26_lhtight_nod0_ivarloose", "trigPassed_HLT_e60_lhmedium_nod0", "trigPassed_HLT_e140_lhloose_nod0"};
      single_mu_paths = {"trigPassed_HLT_mu26_ivarmedium", "trigPassed_HLT_mu50"};
    }
    else if (is22) {
      m_pt_threshold["SLT"]["ele"] = 27000;
      m_pt_threshold["SLT"]["mu"] = 27000;
      single_ele_paths = {"trigPassed_HLT_e26_lhtight_ivarloose_L1EM22VHI", "trigPassed_HLT_e60_lhmedium_L1EM22VHI", "trigPassed_HLT_e140_lhloose_L1EM22VHI"};
      single_mu_paths = {"trigPassed_HLT_mu24_ivarmedium_L1MU14FCH", "trigPassed_HLT_mu50_L1MU14FCH"};
    }
    else if (is23) {
      m_pt_threshold["SLT"]["ele"] = 27000;
      m_pt_threshold["SLT"]["mu"] = 27000;
      single_ele_paths = {"trigPassed_HLT_e26_lhtight_ivarloose_L1eEM26M", "trigPassed_HLT_e60_lhmedium_L1eEM26M", "trigPassed_HLT_e140_lhloose_L1eEM26M"};
      single_mu_paths = {"trigPassed_HLT_mu24_ivarmedium_L1MU14FCH", "trigPassed_HLT_mu50_L1MU14FCH"};
    }

    // Pass single electron trigger
    for(const auto& path : single_ele_paths){
     trigPassed_SET |= m_triggerdecos.at(path).get(*event, sys);
     if(trigPassed_SET) break;
    }

    // Pass single muon trigger
    for(const auto& path : single_mu_paths){
     trigPassed_SMT |= m_triggerdecos.at(path).get(*event, sys);
     if(trigPassed_SMT) break;
    }

    if (trigPassed_SET || trigPassed_SMT) trigPassed_SLT = true;

    // LTT
    m_pt_threshold["LTT"]["ele"] = 18000;
    m_pt_threshold["LTT"]["mu"] = 15000;
    m_pt_threshold["LTT"]["leadingtau"] = 30000;
    m_pt_threshold["LTT"]["leadingjet"] = 45000; // default value
    m_pt_threshold["LTT"]["subleadingjet"] = 20000; // default value
    if(is15 || is16PeriodA){
      ele_tau_paths = {"trigPassed_HLT_e17_lhmedium_nod0_tau25_medium1_tracktwo"};
      mu_tau_paths = {"trigPassed_HLT_mu14_tau25_medium1_tracktwo"};
    }
    else if(is16PeriodB_D3 || is16PeriodD4_end){
      ele_tau_paths = {"trigPassed_HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwo"};
      mu_tau_paths = {"trigPassed_HLT_mu14_ivarloose_tau25_medium1_tracktwo"};
    }
    else if(is17PeriodB1_B4 || is17PeriodB5_B7 || is17PeriodB8_end){
      ele_tau_paths = {"trigPassed_HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwo"};
      ele_tau_paths_4J12 = {"trigPassed_HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwo_L1EM15VHI_2TAU12IM_4J12"};
      mu_tau_paths_low = {"trigPassed_HLT_mu14_ivarloose_tau25_medium1_tracktwo_L1MU10_TAU12IM_3J12"};
      mu_tau_paths_high = {"trigPassed_HLT_mu14_ivarloose_tau35_medium1_tracktwo"};
    }
    else if(is18PeriodB_end){
      ele_tau_paths = {"trigPassed_HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwoEF"};
      ele_tau_paths_4J12 = {"trigPassed_HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwoEF_L1EM15VHI_2TAU12IM_4J12"};
      mu_tau_paths_low = {"trigPassed_HLT_mu14_ivarloose_tau25_medium1_tracktwoEF_L1MU10_TAU12IM_3J12"};
      mu_tau_paths_high = {"trigPassed_HLT_mu14_ivarloose_tau35_medium1_tracktwoEF"};
      if(is18PeriodK_end){
        ele_tau_paths.push_back("trigPassed_HLT_e17_lhmedium_nod0_ivarloose_tau25_mediumRNN_tracktwoMVA");
        ele_tau_paths_4J12.push_back("trigPassed_HLT_e17_lhmedium_nod0_ivarloose_tau25_mediumRNN_tracktwoMVA_L1EM15VHI_2TAU12IM_4J12");
        mu_tau_paths_low.push_back("trigPassed_HLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA_L1MU10_TAU12IM_3J12");
        mu_tau_paths_high .push_back("trigPassed_HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA");
      }
    }
    else if(is22){
      ele_tau_paths = {"trigPassed_HLT_e24_lhmedium_ivarloose_tau20_mediumRNN_tracktwoMVA_03dRAB_L1EM22VHI"};
      ele_tau_paths_4J12 = {"trigPassed_HLT_e17_lhmedium_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1EM15VHI_2TAU12IM_4J12"};
      mu_tau_paths_low = {"trigPassed_HLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1MU8F_TAU12IM_3J12"};
      mu_tau_paths_high = {"trigPassed_HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA_03dRAB_L1MU8F_TAU20IM"};
    }
    else if(is23){
      ele_tau_paths = {"trigPassed_HLT_e24_lhmedium_ivarloose_tau20_mediumRNN_tracktwoMVA_03dRAB_L1eEM26M"};
      ele_tau_paths_4J12 = {"trigPassed_HLT_e17_lhmedium_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1EM15VHI_2TAU12IM_4J12"};
      mu_tau_paths_low = {"trigPassed_HLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1MU8F_TAU12IM_3J12"};
      mu_tau_paths_high = {"trigPassed_HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA_03dRAB_L1MU8F_TAU20IM"};
    }

    // Pass electron + tau trigger
    for(const auto& path : ele_tau_paths_4J12){
     trigPassed_ETT |= m_triggerdecos.at(path).get(*event, sys);
     if(trigPassed_ETT) {
        m_pt_threshold["LTT"]["leadingjet"] = 45000;
        m_pt_threshold["LTT"]["subleadingjet"] = 45000;
        break;
     }
    }
    if(!trigPassed_ETT){
      for(const auto& path : ele_tau_paths){
        trigPassed_ETT |= m_triggerdecos.at(path).get(*event, sys);
        if(trigPassed_ETT) {
           m_pt_threshold["LTT"]["leadingjet"] = 80000;
           m_pt_threshold["LTT"]["subleadingjet"] = 20000;
           break;
        }
       }
    }

    // Pass muon + tau trigger
    for(const auto& path : mu_tau_paths){
     trigPassed_MTT |= m_triggerdecos.at(path).get(*event, sys);
     if(trigPassed_MTT) {
        m_pt_threshold["LTT"]["leadingjet"] = 80000;
        m_pt_threshold["LTT"]["subleadingjet"] = 20000;
        break;
     }
    }

    for(const auto& path : mu_tau_paths_high){
     trigPassed_MTT_high |= m_triggerdecos.at(path).get(*event, sys);
     if(trigPassed_MTT_high) break;
    }
    bool trig_pass_tau_high_pt = false;
    for (const xAOD::TauJet *tau : *taus){
       trig_pass_tau_high_pt |= tau->pt() > 40000;
       if(trig_pass_tau_high_pt) break;
    }
    trigPassed_MTT_high &= trig_pass_tau_high_pt;
    if(trigPassed_MTT_high){
      m_pt_threshold["LTT"]["leadingjet"] = 45000;
      m_pt_threshold["LTT"]["subleadingjet"] = 20000;
    }

    for(const auto& path : mu_tau_paths_low){
     trigPassed_MTT_low |= m_triggerdecos.at(path).get(*event, sys);
     if(trigPassed_MTT_low) break;
    }
    bool trig_pass_tau_low_pt = !trig_pass_tau_high_pt;
    trigPassed_MTT_low &= trig_pass_tau_low_pt;
    if(trigPassed_MTT_low){
      m_pt_threshold["LTT"]["leadingjet"] = 45000;
      m_pt_threshold["LTT"]["subleadingjet"] = 45000;
    }

    if(trigPassed_ETT || trigPassed_MTT || trigPassed_MTT_low ||
        trigPassed_MTT_high)
      trigPassed_LTT = true;
  }

  void HHbbttSelectorAlg ::applySingleTauTriggerSelection(
      const xAOD::EventInfo *event,
      const CP::SystematicSet &sys)
  {

    // hadhad
    std::vector<std::string> single_tau_paths;
    // STT
    m_pt_threshold["STT"]["subleadingtau"] = 25000;
    m_pt_threshold["STT"]["leadingjet"] = 45000;
    m_pt_threshold["STT"]["subleadingjet"] = 20000;

    if(is15 || is16PeriodA){
      m_pt_threshold["STT"]["leadingtau"] = 100000;
      single_tau_paths = {"trigPassed_HLT_tau80_medium1_tracktwo_L1TAU60"};
    }
    else if(is16PeriodB_D3){
      m_pt_threshold["STT"]["leadingtau"] = 140000;
      single_tau_paths = {"trigPassed_HLT_tau125_medium1_tracktwo"};
    }
    else if(is16PeriodD4_end || is17PeriodB1_B4){
      m_pt_threshold["STT"]["leadingtau"] = 180000;
      single_tau_paths = {"trigPassed_HLT_tau160_medium1_tracktwo"};
    }
    else if(is17PeriodB5_B7 || is17PeriodB8_end){
      m_pt_threshold["STT"]["leadingtau"] = 180000;
      single_tau_paths = {"trigPassed_HLT_tau160_medium1_tracktwo_L1TAU100"};
    }
    else if(is18){
      m_pt_threshold["STT"]["leadingtau"] = 180000;
      single_tau_paths = {"trigPassed_HLT_tau160_medium1_tracktwoEF_L1TAU100"};
    }
    else if(is18PeriodK_end){
      m_pt_threshold["STT"]["leadingtau"] = 180000;
      single_tau_paths = {"trigPassed_HLT_tau160_mediumRNN_tracktwoMVA_L1TAU100"};
    }
    else if(is22){
      m_pt_threshold["STT"]["leadingtau"] = 180000;
      single_tau_paths = {"trigPassed_HLT_tau160_mediumRNN_tracktwoMVA_L1TAU100"};
    }
    else if(is23){
      m_pt_threshold["STT"]["leadingtau"] = 180000;
      single_tau_paths = {"trigPassed_HLT_tau160_mediumRNN_tracktwoMVA_L1eTAU140"};
    }
    // Pass single tau trigger
    for(const auto& path : single_tau_paths){
     trigPassed_STT |= m_triggerdecos.at(path).get(*event, sys);
     if(trigPassed_STT) break;
    }
  }

  void HHbbttSelectorAlg ::applyDiTauTriggerSelection(
      const xAOD::EventInfo *event,
      const CP::SystematicSet &sys)
  {
    std::vector<std::string> ditau_paths;
    std::vector<std::string> ditau_paths_4J12;

    // DTT
    m_pt_threshold["DTT"]["leadingtau"] = 40000;
    m_pt_threshold["DTT"]["subleadingtau"] = 30000;
    m_pt_threshold["DTT"]["leadingjet"] = 45000; // default value
    m_pt_threshold["DTT"]["subleadingjet"] = 20000; // default value

    if(is15){
      ditau_paths = {"trigPassed_HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1TAU20IM_2TAU12IM"};
    }
    else if(is16PeriodA || is16PeriodB_D3 || is16PeriodD4_end || is17PeriodB1_B4){
      ditau_paths = {"trigPassed_HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo"};
    }
    else if(is17){
      ditau_paths_4J12 = {"trigPassed_HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1TAU20IM_2TAU12IM_4J12"};
    }
    else if(l1topo_disabled){
      ditau_paths = {"trigPassed_HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo"};
    }
    else if(!l1topo_disabled && (is17PeriodB5_B7 || is17PeriodB8_end)){
      ditau_paths = {"trigPassed_HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1DR_TAU20ITAU12I_J25"};
    }
    else if(is18){
      ditau_paths = {"trigPassed_HLT_tau35_medium1_tracktwoEF_tau25_medium1_tracktwoEF_L1DR_TAU20ITAU12I_J25"};
      ditau_paths_4J12 = {"trigPassed_HLT_tau35_medium1_tracktwoEF_tau25_medium1_tracktwoEF_L1TAU20IM_2TAU12IM_4J12p0ETA23"};
    }
    else if(is18PeriodK_end){
      ditau_paths = {"trigPassed_HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_L1DR_TAU20ITAU12I_J25"};
      ditau_paths_4J12 = {"trigPassed_HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_L1TAU20IM_2TAU12IM_4J12p0ETA23"};
    }
    else if(is22 || is23){
      ditau_paths = {"trigPassed_HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1DR_TAU20ITAU12I_J25"};
      ditau_paths_4J12 = {"trigPassed_HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25"};
    }
    // Pass di-tau trigger
    for(const auto& path : ditau_paths_4J12){
     trigPassed_DTT |= m_triggerdecos.at(path).get(*event, sys);
     if(trigPassed_DTT) {
        m_pt_threshold["DTT"]["leadingjet"] = 45000;
        m_pt_threshold["DTT"]["subleadingjet"] = 45000;
        break;}
    }
    if(!trigPassed_DTT){
      for(const auto& path : ditau_paths){
       trigPassed_DTT |= m_triggerdecos.at(path).get(*event, sys);
       if(trigPassed_DTT) {
          m_pt_threshold["DTT"]["leadingjet"] = 80000;
          m_pt_threshold["DTT"]["subleadingjet"] = 20000;
          if(!(is15 || is16)) DTT_DeltaR_cut = true;
          break;}
      }
    }
  }
}
