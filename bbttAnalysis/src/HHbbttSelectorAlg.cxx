/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Carl Gwilliam

#include "HHbbttSelectorAlg.h"

#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>
#include <AthContainers/ConstDataVector.h>
#include <AthenaKernel/Units.h>

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
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    ATH_CHECK (m_generatorWeight.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK(m_year.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK(m_is2016_periodA.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_is2016_periodB_D3.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_is2022_75bunches.initialize(m_systematicsList, m_eventHandle));
    
    // Intialise booleans with value false. Also initialise syst-aware output decorators
    for (auto& [key, value] : m_boolnames) {
      m_bools.emplace(key, false);
      CP::SysWriteDecorHandle<bool> whandle{value+"_%SYS%", this};
      m_Bbranches.emplace(key, whandle);
      ATH_CHECK(m_Bbranches.at(key).initialize(m_systematicsList, m_eventHandle));
    };

    m_tauWPDecorHandle = CP::SysReadDecorHandle<char>
      ("baselineSelection_" + m_tauWPName+"_%SYS%", this);
    m_eleWPDecorHandle = CP::SysReadDecorHandle<char>
      ("baselineSelection_" + m_eleWPName+"_%SYS%", this);
    m_muonWPDecorHandle = CP::SysReadDecorHandle<char>
      ("baselineSelection_"+m_muonWPName+"_%SYS%", this);

    ATH_CHECK(m_tauWPDecorHandle.initialize(m_systematicsList, m_tauHandle));
    ATH_CHECK(m_antiTauDecorHandle.initialize(m_systematicsList, m_tauHandle));
    ATH_CHECK(m_eleWPDecorHandle.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK(m_muonWPDecorHandle.initialize(m_systematicsList, m_muonHandle));

    ATH_CHECK(m_selected_el.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK(m_selected_mu.initialize(m_systematicsList, m_muonHandle));
    ATH_CHECK(m_selected_tau.initialize(m_systematicsList, m_tauHandle));
    
    for (const auto& [channel, name] : m_triggerChannels){
      SG::ReadDecorHandleKey<xAOD::EventInfo> deco;
      deco = "EventInfo.pass_trigger_"+name;
      m_trigPass_DecorKey.emplace(channel, deco);
      ATH_CHECK(m_trigPass_DecorKey.at(channel).initialize());
    }

    SG::ReadDecorHandleKey<xAOD::MuonContainer> mu_SLT;
    mu_SLT = m_muonHandle.getNamePattern() + ".trigMatch_SLT";
    m_mu_trigMatch_DecorKey.emplace(HHBBTT::SLT, mu_SLT);
    SG::ReadDecorHandleKey<xAOD::MuonContainer> mu_LTT;
    mu_LTT = m_muonHandle.getNamePattern() + ".trigMatch_LTT";
    m_mu_trigMatch_DecorKey.emplace(HHBBTT::LTT, mu_LTT);
    SG::ReadDecorHandleKey<xAOD::MuonContainer> mu_MTT_2016;
    mu_MTT_2016 = m_muonHandle.getNamePattern() + ".trigMatch_MTT_2016";
    m_mu_trigMatch_DecorKey.emplace(HHBBTT::MTT_2016, mu_MTT_2016);
    SG::ReadDecorHandleKey<xAOD::MuonContainer> mu_MTT_high;
    mu_MTT_high = m_muonHandle.getNamePattern() + ".trigMatch_MTT_high";
    m_mu_trigMatch_DecorKey.emplace(HHBBTT::MTT_high, mu_MTT_high);
    SG::ReadDecorHandleKey<xAOD::MuonContainer> mu_MTT_low;
    mu_MTT_low = m_muonHandle.getNamePattern() + ".trigMatch_MTT_low";
    m_mu_trigMatch_DecorKey.emplace(HHBBTT::MTT_low, mu_MTT_low);

    for (const auto& [channel, key] : m_mu_trigMatch_DecorKey){
      ATH_CHECK(m_mu_trigMatch_DecorKey.at(channel).initialize());
    }
    
    SG::ReadDecorHandleKey<xAOD::ElectronContainer> ele_SLT;
    ele_SLT = m_electronHandle.getNamePattern() + ".trigMatch_SLT";
    m_ele_trigMatch_DecorKey.emplace(HHBBTT::SLT, ele_SLT);
    SG::ReadDecorHandleKey<xAOD::ElectronContainer> ele_LTT;
    ele_LTT = m_electronHandle.getNamePattern() + ".trigMatch_LTT";
    m_ele_trigMatch_DecorKey.emplace(HHBBTT::LTT, ele_LTT);
    SG::ReadDecorHandleKey<xAOD::ElectronContainer> ele_ETT;
    ele_ETT = m_electronHandle.getNamePattern() + ".trigMatch_ETT";
    m_ele_trigMatch_DecorKey.emplace(HHBBTT::ETT, ele_ETT);
    SG::ReadDecorHandleKey<xAOD::ElectronContainer> ele_ETT_4J12;
    ele_ETT_4J12 = m_electronHandle.getNamePattern() + ".trigMatch_ETT_4J12";
    m_ele_trigMatch_DecorKey.emplace(HHBBTT::ETT_4J12, ele_ETT_4J12);
    
    for (const auto& [channel, key] : m_ele_trigMatch_DecorKey){
      ATH_CHECK(m_ele_trigMatch_DecorKey.at(channel).initialize());
    }

    for (const auto& [channel, name] : m_triggerChannels){
      if(channel==HHBBTT::SLT) continue;
      SG::ReadDecorHandleKey<xAOD::TauJetContainer> deco;
      deco = m_tauHandle.getNamePattern() + ".trigMatch_"+name;
      m_tau_trigMatch_DecorKey.emplace(channel, deco);
      ATH_CHECK(m_tau_trigMatch_DecorKey.at(channel).initialize());
    }

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());    
    for ( auto name : m_channel_names){
      if( name == "lephad2b") m_channels.push_back(HHBBTT::LepHad2B);
      else if ( name == "hadhad2b") m_channels.push_back(HHBBTT::HadHad2B);
      else if ( name == "lephad1b") m_channels.push_back(HHBBTT::LepHad1B);
      else if ( name == "hadhad1b") m_channels.push_back(HHBBTT::HadHad1B);
      else if ( name == "ZCR") m_channels.push_back(HHBBTT::ZCR);
      else if ( name == "TopEMuCR") m_channels.push_back(HHBBTT::TopEMuCR);
      else{
        ATH_MSG_ERROR("Unknown channel");
        return StatusCode::FAILURE;
      }
    }

    ATH_CHECK (initialiseCutflow());
    return StatusCode::SUCCESS;
  }

    

  StatusCode HHbbttSelectorAlg ::execute()
  {
    // Global filter originally false
    CP::SysFilterReporterCombiner filterCombiner (m_filterParams, false);

    trigPassReadDecoMap trigPass_decos;
    for (const auto& [channel, key] : m_trigPass_DecorKey){
      trigPass_decos.emplace(channel, key);
    }

    muTrigMatchReadDecoMap mu_trigMatchDecos;
    for (const auto& [channel, key] : m_mu_trigMatch_DecorKey){
      mu_trigMatchDecos.emplace(channel, key);
    }

    eleTrigMatchReadDecoMap ele_trigMatchDecos;
    for (const auto& [channel, key] : m_ele_trigMatch_DecorKey){
      ele_trigMatchDecos.emplace(channel, key);
    }

    tauTrigMatchReadDecoMap tau_trigMatchDecos;
    for (const auto& [channel, key] : m_tau_trigMatch_DecorKey){
      tau_trigMatchDecos.emplace(channel, key);
    }

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector()){
      CP::SysFilterReporter filter (filterCombiner, sys);

      // Retrieve inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      setThresholds(event, sys);

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      const xAOD::TauJetContainer *taus = nullptr;
      ANA_CHECK (m_tauHandle.retrieve (taus, sys));

      // Apply selection

      for (auto& [key, value] : m_boolnames) m_bools.at(key) = false;

      //************
      // lepton
      //************
      int n_leptons = 0;
      int n_looseleptons = 0;

      const xAOD::Electron* ele0 = nullptr;
      const xAOD::Electron* ele1 = nullptr;
      for (const xAOD::Electron *electron : *electrons)
      {
        bool passElectronWP = m_eleWPDecorHandle.get(*electron, sys);
        m_selected_el.set(*electron, false, sys);
        if (passElectronWP &&
	    electron->pt() > m_pt_threshold[HHBBTT::LTT][HHBBTT::ele])
	{
          m_selected_el.set(*electron, true, sys);
          if(!ele0) ele0 = electron;
          else if(!ele1) ele1 = electron;
          n_leptons += 1;
        }
        else
          n_looseleptons += 1;
      }
      if (ele1) {
        if (ele0->charge() != ele1->charge())
          m_bools.at(HHBBTT::OS_CHARGE_LEPTONS) = true;
      }

      const xAOD::Muon* mu0 = nullptr;
      const xAOD::Muon* mu1 = nullptr;
      for (const xAOD::Muon *muon : *muons)
      {
        bool passMuonWP = m_muonWPDecorHandle.get(*muon, sys);
        m_selected_mu.set(*muon, false, sys);
        if (passMuonWP && std::abs(muon->eta()) < 2.5 &&
	    muon->pt() > m_pt_threshold[HHBBTT::LTT][HHBBTT::mu])
        {
          m_selected_mu.set(*muon, true, sys);
          if(!mu0) mu0 = muon;
          else if(!mu1) mu1 = muon;
          n_leptons += 1;
        }
        else
          n_looseleptons += 1;
      }
      if (mu1) {
        if (mu0->charge() != mu1->charge())
          m_bools.at(HHBBTT::OS_CHARGE_LEPTONS) = true;
      } else if (n_leptons == 2 && mu0) {
        if (ele0->charge() != mu0->charge())
          m_bools.at(HHBBTT::OS_CHARGE_LEPTONS) = true;
      }


      int charge_lepton = 0;
      bool lep_ptcut_SLT = false;
      bool lep_ptcut_LTT = false;
      if(ele0){
	charge_lepton = ele0->charge();
	if (ele0->pt() > m_pt_threshold[HHBBTT::SLT][HHBBTT::ele])
	  lep_ptcut_SLT = true;
	else
	  lep_ptcut_LTT = true;
      }
      if(mu0){
	charge_lepton = mu0->charge();
	if (mu0->pt() > m_pt_threshold[HHBBTT::SLT][HHBBTT::mu])
	  lep_ptcut_SLT = true;
	else
	  lep_ptcut_LTT = true;
      }

      if (n_leptons == 1 && n_looseleptons == 0)
        m_bools.at(HHBBTT::N_LEPTONS_CUT_LEPHAD) = true;

      if (n_leptons == 0 && n_looseleptons == 0)
        m_bools.at(HHBBTT::N_LEPTONS_CUT_HADHAD) = true;

      //************
      // taujet
      //************
      int n_taus = 0;

      const xAOD::TauJet* tau0 = nullptr;
      const xAOD::TauJet* tau1 = nullptr;
      for (const xAOD::TauJet *tau : *taus)
      {
        bool passTauWP = m_tauWPDecorHandle.get(*tau, sys);
        if (m_doAntiIDRegions) {
          passTauWP |= m_antiTauDecorHandle.get(*tau, sys);
        }
        m_selected_tau.set(*tau, false, sys);
        if (passTauWP && tau->pt() > 20. * Athena::Units::GeV)
        {
          m_selected_tau.set(*tau, true, sys);
          n_taus += 1;
          if(n_taus==1) tau0 = tau;
          else if(n_taus==2) tau1 = tau;
        }
      }

      bool tau_ptcut_SLT = n_taus>0;
      bool tau_ptcut_LTT = n_taus>0 &&
	tau0->pt() > m_pt_threshold[HHBBTT::LTT][HHBBTT::leadingtau];
      bool tau_ptcut_STT_lead = n_taus>0 &&
	tau0->pt() > m_pt_threshold[HHBBTT::STT][HHBBTT::leadingtau];
      bool tau_ptcut_STT_sublead = n_taus>1 &&
	tau1->pt() > m_pt_threshold[HHBBTT::STT][HHBBTT::subleadingtau];
      bool tau_ptcut_DTT_lead = n_taus>0 &&
	tau0->pt() > m_pt_threshold[HHBBTT::DTT][HHBBTT::leadingtau];
      bool tau_ptcut_DTT_sublead = n_taus>1 &&
	tau1->pt() > m_pt_threshold[HHBBTT::DTT][HHBBTT::subleadingtau];
      bool tau_ptcut_STT = tau_ptcut_STT_lead && tau_ptcut_STT_sublead;
      bool tau_ptcut_DTT = tau_ptcut_DTT_lead && tau_ptcut_DTT_sublead;



      if (n_taus == 1)
        m_bools.at(HHBBTT::ONE_TAU) = true;

      bool tau_DR_L1Topo = false;
      if (n_taus == 2) {
        m_bools.at(HHBBTT::TWO_TAU) = true;
        tau_DR_L1Topo = tau0->p4().DeltaR(tau1->p4())<2.5;
      }

      const xAOD::Jet* jet0 = jets->size()>0 ? jets->at(0) : nullptr;
      const xAOD::Jet* jet1 = jets->size()>1 ? jets->at(1) : nullptr;

      if(m_useTriggerSel){
	applyTriggerSelection(event, trigPass_decos,
			      ele0, ele_trigMatchDecos,
			      mu0, mu_trigMatchDecos,
			      tau0, tau1, tau_trigMatchDecos,
			      jet0, jet1);
      }
      else{
        m_bools.at(HHBBTT::pass_trigger_SLT) = true;
        m_bools.at(HHBBTT::pass_trigger_LTT) = true;
        m_bools.at(HHBBTT::pass_trigger_STT) = true;
        m_bools.at(HHBBTT::pass_trigger_DTT) = true;
        m_bools.at(HHBBTT::pass_trigger_DTT_2016) = true;
        m_bools.at(HHBBTT::pass_trigger_DTT_4J12) = true;
        m_bools.at(HHBBTT::pass_trigger_DTT_L1Topo) = true;
        m_bools.at(HHBBTT::pass_trigger_DTT_4J12_delayed) = true;
        m_bools.at(HHBBTT::pass_trigger_DTT_L1Topo_delayed) = true;
        m_bools.at(HHBBTT::pass_trigger_DBT) = true;

      }

      m_bools.at(HHBBTT::pass_trigger_SR) =
	(m_bools.at(HHBBTT::pass_trigger_SLT) ||
	 m_bools.at(HHBBTT::pass_trigger_LTT) ||
	 m_bools.at(HHBBTT::pass_trigger_STT) ||
	 m_bools.at(HHBBTT::pass_trigger_DTT) ||
	 m_bools.at(HHBBTT::pass_trigger_DBT));


      //************
      // jet
      //************
      int n_jets = 0;
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

      bool jet_ptcut_DTT_2016 = false;
      bool jet_ptcut_DTT_4J12 = false;
      bool jet_ptcut_DTT_L1Topo = false;

      if (n_jets >= 2)
      {
        m_bools.at(HHBBTT::TWO_JETS) = true;
        if (jet0->pt() > m_pt_threshold[HHBBTT::DTT_2016][HHBBTT::leadingjet])
          jet_ptcut_DTT_2016 = true;
        if (jet0->pt() > m_pt_threshold[HHBBTT::DTT_L1Topo][HHBBTT::leadingjet])
          jet_ptcut_DTT_L1Topo = true;
        if (jet0->pt() > m_pt_threshold[HHBBTT::DTT_4J12][HHBBTT::leadingjet] &&
	    jet1->pt() > m_pt_threshold[HHBBTT::DTT_4J12][HHBBTT::subleadingjet])
          jet_ptcut_DTT_4J12 = true;


        if (bjets->size() == 2)
        {
          m_bools.at(HHBBTT::TWO_BJETS) = (bjets->at(0)->pt() > 45. * Athena::Units::GeV && bjets->at(1)->pt() > 20. * Athena::Units::GeV);
          bb = bjets->at(0)->p4() + bjets->at(1)->p4();
          mbb = bb.M();
        } else if (bjets->size() == 1) {
          m_bools.at(HHBBTT::ONE_BJET) = (bjets->at(0)->pt() > 45. * Athena::Units::GeV);
        }
      }



      //****************
      // event level info
      //****************
      if (mbb < 150. * Athena::Units::GeV)
        m_bools.at(HHBBTT::MBB_MASS) = true;
      if (n_taus==1 && n_leptons==1 && tau0->charge() != charge_lepton)
        m_bools.at(HHBBTT::OS_CHARGE_LEPHAD) = true;
      if (n_taus==2 && tau0->charge() == -tau1->charge())
        m_bools.at(HHBBTT::OS_CHARGE_HADHAD) = true;

      if (m_bools.at(HHBBTT::N_LEPTONS_CUT_LEPHAD) &&
	  m_bools.at(HHBBTT::ONE_TAU) &&
	  m_bools.at(HHBBTT::TWO_JETS)){
        // SLT
        if (lep_ptcut_SLT && tau_ptcut_SLT){
          m_bools.at(pass_baseline_SLT) = true;
          if (m_bools.at(HHBBTT::pass_trigger_SLT)){
            if (m_bools.at(HHBBTT::TWO_BJETS))
              m_bools.at(HHBBTT::pass_SLT_2B) = true;
            else if (m_bools.at(HHBBTT::ONE_BJET))
              m_bools.at(HHBBTT::pass_SLT_1B) = true;
          }
        }

        // LTT
        if (lep_ptcut_LTT && tau_ptcut_LTT){
          m_bools.at(HHBBTT::pass_baseline_LTT) = true;
          if (m_bools.at(HHBBTT::pass_trigger_LTT)){
            if (!m_bools.at(HHBBTT::pass_SLT_2B) &&
                m_bools.at(HHBBTT::TWO_BJETS))
              m_bools.at(HHBBTT::pass_LTT_2B) = true;
            else if(!m_bools.at(HHBBTT::pass_SLT_1B) &&
                    m_bools.at(HHBBTT::ONE_BJET))
              m_bools.at(HHBBTT::pass_LTT_1B) = true;
          }
        }
      }

      if (m_bools.at(HHBBTT::N_LEPTONS_CUT_HADHAD) &&
	  m_bools.at(HHBBTT::TWO_TAU) &&
	  m_bools.at(HHBBTT::TWO_JETS)){
        // STT
        if (tau_ptcut_STT){
          m_bools.at(HHBBTT::pass_baseline_STT) = true;
          if (m_bools.at(HHBBTT::pass_trigger_STT)) {
            if (m_bools.at(HHBBTT::TWO_BJETS)) m_bools.at(HHBBTT::pass_STT_2B) = true;
            else if (m_bools.at(HHBBTT::ONE_BJET)) m_bools.at(HHBBTT::pass_STT_1B) = true;
          }
        }
        // DTT
        if(!m_bools.at(HHBBTT::pass_baseline_STT) && tau_ptcut_DTT){
          int year = m_year.get(*event, sys);
          if(2015<=year && year<=2016){
            if(jet_ptcut_DTT_2016){
              m_bools.at(HHBBTT::pass_baseline_DTT_2016) = true;
              if (m_bools.at(HHBBTT::pass_trigger_DTT_2016)) {
                if (m_bools.at(HHBBTT::TWO_BJETS)) m_bools.at(HHBBTT::pass_DTT_2016_2B) = true;
                else if (m_bools.at(HHBBTT::ONE_BJET)) m_bools.at(HHBBTT::pass_DTT_2016_1B) = true;
              }
            }
          }
          else if(jet_ptcut_DTT_4J12){
            m_bools.at(HHBBTT::pass_baseline_DTT_4J12) = true;
              if (m_bools.at(HHBBTT::pass_trigger_DTT_4J12)) {
                if (m_bools.at(HHBBTT::TWO_BJETS)) m_bools.at(HHBBTT::pass_DTT_4J12_2B) = true;
                else if (m_bools.at(HHBBTT::ONE_BJET)) m_bools.at(HHBBTT::pass_DTT_4J12_1B) = true;
              }
              if (m_bools.at(HHBBTT::pass_trigger_DTT_4J12_delayed)) {
                if (m_bools.at(HHBBTT::TWO_BJETS)) m_bools.at(HHBBTT::pass_DTT_4J12_delayed_2B) = true;
                else if (m_bools.at(HHBBTT::ONE_BJET)) m_bools.at(HHBBTT::pass_DTT_4J12_delayed_1B) = true;
              }
          }
          else if(jet_ptcut_DTT_L1Topo && tau_DR_L1Topo){
            m_bools.at(HHBBTT::pass_baseline_DTT_L1Topo) = true;
              if (m_bools.at(HHBBTT::pass_trigger_DTT_L1Topo)) {
                if (m_bools.at(HHBBTT::TWO_BJETS)) m_bools.at(HHBBTT::pass_DTT_L1Topo_2B) = true;
                else if (m_bools.at(HHBBTT::ONE_BJET)) m_bools.at(HHBBTT::pass_DTT_L1Topo_1B) = true;
              }
              if (m_bools.at(HHBBTT::pass_trigger_DTT_L1Topo_delayed)) {
                if (m_bools.at(HHBBTT::TWO_BJETS)) m_bools.at(HHBBTT::pass_DTT_L1Topo_delayed_2B) = true;
                else if (m_bools.at(HHBBTT::ONE_BJET)) m_bools.at(HHBBTT::pass_DTT_L1Topo_delayed_1B) = true;
              }
          }
        }

        //DBT
        int year = m_year.get(*event, sys);
        if(!m_bools.at(HHBBTT::pass_baseline_STT) && year>=2022){
          m_bools.at(HHBBTT::pass_baseline_DBT) = true;
          if(m_bools.at(HHBBTT::pass_trigger_DBT)){
            if (m_bools.at(HHBBTT::TWO_BJETS)) m_bools.at(HHBBTT::pass_DBT_2B) = true;
            else if (m_bools.at(HHBBTT::ONE_BJET)) m_bools.at(HHBBTT::pass_DBT_1B) = true;
          }
        }
      }

      m_bools.at(HHBBTT::pass_baseline_DTT) =
	      (m_bools.at(HHBBTT::pass_baseline_DTT_2016)  ||
	       m_bools.at(HHBBTT::pass_baseline_DTT_4J12)  ||
               m_bools.at(HHBBTT::pass_baseline_DTT_L1Topo));
      m_bools.at(HHBBTT::pass_DTT_2B) =
	      (m_bools.at(HHBBTT::pass_DTT_2016_2B)  ||
	       m_bools.at(HHBBTT::pass_DTT_4J12_2B)  ||
               m_bools.at(HHBBTT::pass_DTT_L1Topo_2B)  ||
               m_bools.at(HHBBTT::pass_DTT_4J12_delayed_2B)  ||
               m_bools.at(HHBBTT::pass_DTT_L1Topo_delayed_2B));
      m_bools.at(HHBBTT::pass_DTT_1B) = 
              (m_bools.at(HHBBTT::pass_DTT_2016_1B)  ||
               m_bools.at(HHBBTT::pass_DTT_4J12_1B)  ||
               m_bools.at(HHBBTT::pass_DTT_L1Topo_1B)  ||
               m_bools.at(HHBBTT::pass_DTT_4J12_delayed_1B)  ||
               m_bools.at(HHBBTT::pass_DTT_L1Topo_delayed_1B));

      m_bools.at(HHBBTT::pass_baseline_SR) =
	(m_bools.at(HHBBTT::pass_baseline_SLT) ||
	 m_bools.at(HHBBTT::pass_baseline_LTT) ||
	 m_bools.at(HHBBTT::pass_baseline_STT) ||
	 m_bools.at(HHBBTT::pass_baseline_DTT) ||
         m_bools.at(HHBBTT::pass_baseline_DBT));

      m_bools.at(HHBBTT::pass_SR_1B) =
	(m_bools.at(HHBBTT::pass_SLT_1B) ||
	 m_bools.at(HHBBTT::pass_LTT_1B) ||
	 m_bools.at(HHBBTT::pass_STT_1B) ||
	 m_bools.at(HHBBTT::pass_DTT_1B) ||
	 m_bools.at(HHBBTT::pass_DBT_1B));

      m_bools.at(HHBBTT::pass_SR_2B) =
	(m_bools.at(HHBBTT::pass_SLT_2B) ||
	 m_bools.at(HHBBTT::pass_LTT_2B) ||
	 m_bools.at(HHBBTT::pass_STT_2B) ||
	 m_bools.at(HHBBTT::pass_DTT_2B) ||
	 m_bools.at(HHBBTT::pass_DBT_2B));

      m_bools.at(HHBBTT::pass_baseline_LepHad) =
	(m_bools.at(HHBBTT::pass_baseline_SLT) ||
	 m_bools.at(HHBBTT::pass_baseline_LTT) );
      m_bools.at(HHBBTT::pass_baseline_HadHad) =
	(m_bools.at(HHBBTT::pass_baseline_STT) ||
	 m_bools.at(HHBBTT::pass_baseline_DTT) ||
	 m_bools.at(HHBBTT::pass_baseline_DBT) );
      m_bools.at(HHBBTT::pass_LepHad_2B) =
	(m_bools.at(HHBBTT::pass_SLT_2B) ||
	 m_bools.at(HHBBTT::pass_LTT_2B) );
      m_bools.at(HHBBTT::pass_HadHad_2B) =
	(m_bools.at(HHBBTT::pass_STT_2B) ||
	 m_bools.at(HHBBTT::pass_DTT_2B) ||
	 m_bools.at(HHBBTT::pass_DBT_2B) );
      m_bools.at(HHBBTT::pass_LepHad_1B) =
	(m_bools.at(HHBBTT::pass_SLT_1B) ||
	 m_bools.at(HHBBTT::pass_LTT_1B) );
      m_bools.at(HHBBTT::pass_HadHad_1B) =
	(m_bools.at(HHBBTT::pass_STT_1B) ||
	 m_bools.at(HHBBTT::pass_DTT_1B) ||
	 m_bools.at(HHBBTT::pass_DBT_1B) );
      m_bools.at(HHBBTT::pass_LepHad) =
	(m_bools.at(HHBBTT::pass_LepHad_2B) ||
	 m_bools.at(HHBBTT::pass_LepHad_1B) );
      m_bools.at(HHBBTT::pass_HadHad) =
	(m_bools.at(HHBBTT::pass_HadHad_2B) ||
	 m_bools.at(HHBBTT::pass_HadHad_1B) );

      // Z+HF and top (e+mu) control regions
      if (m_bools.at(HHBBTT::pass_trigger_SLT) && m_bools.at(HHBBTT::TWO_BJETS)){
        if(jets->at(0)->pt() > 45. * Athena::Units::GeV && n_leptons == 2) {
          float mll = -999.;
          float lep1_pt = -999.;
          if (ele1) {
            mll = (ele0->p4() + ele1->p4()).M();
            lep1_pt = ele1->pt();
          }
          else if (mu1) {
            mll = (mu0->p4() + mu1->p4()).M();
            lep1_pt = mu1->pt();
          }
          m_bools.at(HHBBTT::pass_ZCR) = mll > 75. * Athena::Units::GeV && mll < 110. * Athena::Units::GeV &&
            lep1_pt > 40. * Athena::Units::GeV;

          // No subleading ele + muon = e+mu event
          m_bools.at(HHBBTT::pass_TopEMuCR) = !ele1 && !mu1 && ele0->pt() > 40. * Athena::Units::GeV && mu0->pt() > 40. * Athena::Units::GeV;
        }
      }

      bool pass = false;
      for(const auto& channel : m_channels){
       if(channel == HHBBTT::LepHad2B) pass |= m_bools.at(HHBBTT::pass_LepHad_2B);
       else if(channel == HHBBTT::HadHad2B) pass |= m_bools.at(HHBBTT::pass_HadHad_2B);
       else if(channel == HHBBTT::LepHad1B) pass |= m_bools.at(HHBBTT::pass_LepHad_1B);
       else if(channel == HHBBTT::HadHad1B) pass |= m_bools.at(HHBBTT::pass_HadHad_1B);
       else if(channel == HHBBTT::ZCR) pass |= m_bools.at(HHBBTT::pass_ZCR);
       else if(channel == HHBBTT::TopEMuCR) pass |= m_bools.at(HHBBTT::pass_TopEMuCR);
      }

      //****************
      // Cutflow
      //****************

      // do the CUTFLOW only with sys="" -> NOSYS
      if (sys.name()==""){

        // Compute total_events
        m_total_events+=1;
        if(m_isMC) m_total_mcEventWeight+= m_generatorWeight.get(*event, sys);


        // Count which cuts the event passed
        for (const auto &cut : m_inputCutKeys) {
          if(m_bbttCuts.exists(m_boolnames.at(cut))) {
            m_bbttCuts(m_boolnames.at(cut)).passed = m_bools.at(cut);
            if (m_bbttCuts(m_boolnames.at(cut)).passed) {
              m_bbttCuts(m_boolnames.at(cut)).counter += 1;
              if(m_isMC) m_bbttCuts(m_boolnames.at(cut)).w_counter += m_generatorWeight.get(*event, sys);
            }
          }
        }

        // Check how many consecutive cuts are passed by the event.
        unsigned int consecutive_cuts = 0;
        for (size_t i = 0; i < m_bbttCuts.size(); ++i) {
          if (m_bbttCuts[i].passed)
            consecutive_cuts++;
          else
            break;
        }

        // Here we basically increment the  N_events(pass_i  AND pass_i-1  AND ... AND pass_0) for the i-cut.
        for (unsigned int i=0; i<consecutive_cuts; i++) {
          m_bbttCuts[i].relativeCounter += 1;
          if(m_isMC) m_bbttCuts[i].w_relativeCounter += m_generatorWeight.get(*event, sys);
        }
      }

      // Fill syst-aware output decorators
      for (auto& [key, var] : m_bools) {
        m_Bbranches.at(key).set(*event, var, sys);
      };

      if (!m_bypass && !pass) continue;

      // Global event filter true if any syst passes and controls
      // if event is passed to output writing or not
      filter.setPassed(true);
    }

    return StatusCode::SUCCESS;
  }

  StatusCode HHbbttSelectorAlg::finalize() {
    ANA_CHECK (m_filterParams.finalize ());

    m_bbttCuts.CheckCutResults(); // Print CheckCutResults

    if(m_saveCutFlow) {
      m_bbttCuts.DoAbsoluteEfficiency(m_total_events, efficiency("AbsoluteEfficiency"));
      m_bbttCuts.DoRelativeEfficiency(m_total_events, efficiency("RelativeEfficiency"));
      m_bbttCuts.DoStandardCutFlow(m_total_events, efficiency("StandardCutFlow"));
      if(m_isMC) {
        m_bbttCuts.DoWeightedAbsoluteEfficiency(m_total_mcEventWeight, efficiency("WeightedAbsoluteEfficiency"));
        m_bbttCuts.DoWeightedRelativeEfficiency(m_total_mcEventWeight, efficiency("WeightedRelativeEfficiency"));
        m_bbttCuts.DoWeightedStandardCutFlow(m_total_mcEventWeight, efficiency("WeightedStandardCutFlow"));
      }
      m_bbttCuts.DoCutflowLabeling(m_total_events, hist("EventsPassed_BinLabeling"));
    }
    else {
      delete efficiency("AbsoluteEfficiency");
      delete efficiency("RelativeEfficiency");
      delete efficiency("StandardCutFlow");
      if(m_isMC) {
        delete efficiency("WeightedAbsoluteEfficiency");
        delete efficiency("WeightedRelativeEfficiency");
        delete efficiency("WeightedStandardCutFlow");
      }
      delete hist("EventsPassed_BinLabeling");
    }

    return StatusCode::SUCCESS;
  }

  void HHbbttSelectorAlg::applyTriggerSelection
  (const xAOD::EventInfo* event, const trigPassReadDecoMap& triggerdecos,
   const xAOD::Electron* ele, const eleTrigMatchReadDecoMap& ele_trigMatchDecos,
   const xAOD::Muon* mu, const muTrigMatchReadDecoMap& mu_trigMatchDecos,
   const xAOD::TauJet* tau0, const xAOD::TauJet* tau1,
   const tauTrigMatchReadDecoMap& tau_trigMatchDecos,
   const xAOD::Jet* jet0, const xAOD::Jet* jet1){

    // only run trigger selection if in channel
    bool use_SLT = false;
    bool use_LTT = false;
    bool use_STT = false;
    bool use_DTT = false;
    bool use_DBT = false;
    for (const auto &channel : m_channels){
      if (channel == HHBBTT::LepHad2B || channel == HHBBTT::LepHad1B){
	use_SLT = true;
	use_LTT = true;
      }
      else if (channel == HHBBTT::HadHad2B || channel == HHBBTT::HadHad1B){
	use_STT = true;
	use_DTT = true;
	use_DBT = true;
      }
      else if (channel == HHBBTT::ZCR || channel == HHBBTT::TopEMuCR){
	use_SLT = true;
      }
    }

    if(use_SLT){
      applySingleLepTriggerSelection(event, triggerdecos,
				     ele, ele_trigMatchDecos,
				     mu, mu_trigMatchDecos);
    }
    if(use_LTT){
      applyLepHadTriggerSelection(event, triggerdecos,
				  ele, ele_trigMatchDecos,
				  mu, mu_trigMatchDecos,
				  tau0, tau_trigMatchDecos,
				  jet0, jet1);
    }
    if(use_STT){
      applySingleTauTriggerSelection(event, triggerdecos,
				     tau0, tau_trigMatchDecos);
    }
    if(use_DTT){
      applyDiTauTriggerSelection(event, triggerdecos,
				 tau0, tau1, tau_trigMatchDecos,
				 jet0, jet1);
    }
    if(use_DBT){
      applyDiBJetTriggerSelection(event, triggerdecos,
         jet0, jet1);
    }

  }

  void HHbbttSelectorAlg::applySingleLepTriggerSelection
  (const xAOD::EventInfo* event, const trigPassReadDecoMap& triggerdecos,
   const xAOD::Electron* ele, const eleTrigMatchReadDecoMap& ele_trigMatchDecos,
   const xAOD::Muon* mu, const muTrigMatchReadDecoMap& mu_trigMatchDecos) {

    bool trigPassed_SET = triggerdecos.at(HHBBTT::SLT)(*event);
    if(ele){
      trigPassed_SET &= ele_trigMatchDecos.at(HHBBTT::SLT)(*ele);
      trigPassed_SET &= ele->pt() > m_pt_threshold[HHBBTT::SLT][HHBBTT::ele];
    }
    else trigPassed_SET = false;

    bool trigPassed_SMT = triggerdecos.at(HHBBTT::SLT)(*event);
    if(mu){
      trigPassed_SMT &= mu_trigMatchDecos.at(HHBBTT::SLT)(*mu);
      trigPassed_SMT &= mu->pt() > m_pt_threshold[HHBBTT::SLT][HHBBTT::mu];
    }
    else trigPassed_SMT = false;

    m_bools.at(HHBBTT::pass_trigger_SLT) = (trigPassed_SET || trigPassed_SMT);
  }

  void HHbbttSelectorAlg::applyLepHadTriggerSelection
  (const xAOD::EventInfo* event,  const trigPassReadDecoMap& triggerdecos,
   const xAOD::Electron* ele, const eleTrigMatchReadDecoMap& ele_trigMatchDecos,
   const xAOD::Muon* mu, const muTrigMatchReadDecoMap& mu_trigMatchDecos,
   const xAOD::TauJet* tau, const tauTrigMatchReadDecoMap& tau_trigMatchDecos,
   const xAOD::Jet* jet0, const xAOD::Jet* jet1){

    bool trigPassed_ETT_4J12 = triggerdecos.at(HHBBTT::ETT_4J12)(*event);
    if(ele && tau && jet0 && jet1){
      trigPassed_ETT_4J12 &= ele_trigMatchDecos.at(HHBBTT::ETT_4J12)(*ele);
      trigPassed_ETT_4J12 &= tau_trigMatchDecos.at(HHBBTT::ETT_4J12)(*tau);
      trigPassed_ETT_4J12 &=
	(ele->pt() > m_pt_threshold[HHBBTT::ETT_4J12][HHBBTT::ele] &&
	 tau->pt() > m_pt_threshold[HHBBTT::ETT_4J12][HHBBTT::leadingtau] &&
	 jet0->pt() > m_pt_threshold[HHBBTT::ETT_4J12][HHBBTT::leadingjet] &&
	 jet1->pt() > m_pt_threshold[HHBBTT::ETT_4J12][HHBBTT::subleadingjet]);
    }
    else trigPassed_ETT_4J12 = false;

    bool trigPassed_ETT = triggerdecos.at(HHBBTT::ETT)(*event);
    if(ele && tau && jet0){
      trigPassed_ETT &= ele_trigMatchDecos.at(HHBBTT::ETT)(*ele);
      trigPassed_ETT &= tau_trigMatchDecos.at(HHBBTT::ETT)(*tau);
      trigPassed_ETT &=
	(ele->pt() > m_pt_threshold[HHBBTT::ETT][HHBBTT::ele] &&
	 tau->pt() > m_pt_threshold[HHBBTT::ETT][HHBBTT::leadingtau] &&
	 jet0->pt() > m_pt_threshold[HHBBTT::ETT][HHBBTT::leadingjet]);
    }
    else trigPassed_ETT = false;

    bool trigPassed_MTT_2016 = triggerdecos.at(HHBBTT::MTT_2016)(*event);
    if(mu && tau && jet0){
      trigPassed_MTT_2016 &= mu_trigMatchDecos.at(HHBBTT::MTT_2016)(*mu);
      trigPassed_MTT_2016 &= tau_trigMatchDecos.at(HHBBTT::MTT_2016)(*tau);
      trigPassed_MTT_2016 &=
	(mu->pt() > m_pt_threshold[HHBBTT::MTT_2016][HHBBTT::mu] &&
	 tau->pt() > m_pt_threshold[HHBBTT::MTT_2016][HHBBTT::leadingtau] &&
	 jet0->pt() > m_pt_threshold[HHBBTT::MTT_2016][HHBBTT::leadingjet]);
    }
    else trigPassed_MTT_2016 = false;

    bool trigPassed_MTT_high = triggerdecos.at(HHBBTT::MTT_high)(*event);
    if(mu && tau && jet0){
      trigPassed_MTT_high &= mu_trigMatchDecos.at(HHBBTT::MTT_high)(*mu);
      trigPassed_MTT_high &= tau_trigMatchDecos.at(HHBBTT::MTT_high)(*tau);
      trigPassed_MTT_high &=
	(mu->pt() > m_pt_threshold[HHBBTT::MTT_high][HHBBTT::mu] &&
	 tau->pt() > m_pt_threshold[HHBBTT::MTT_high][HHBBTT::leadingtau] &&
	 jet0->pt() > m_pt_threshold[HHBBTT::MTT_high][HHBBTT::leadingjet]);
    }
    else trigPassed_MTT_high = false;

    bool trigPassed_MTT_low = triggerdecos.at(HHBBTT::MTT_low)(*event);
    if(mu && tau && jet0 && jet1){
      trigPassed_MTT_low &= mu_trigMatchDecos.at(HHBBTT::MTT_low)(*mu);
      trigPassed_MTT_low &= tau_trigMatchDecos.at(HHBBTT::MTT_low)(*tau);
      trigPassed_MTT_low &=
	(mu->pt() > m_pt_threshold[HHBBTT::MTT_low][HHBBTT::mu] &&
	 tau->pt() > m_pt_threshold[HHBBTT::MTT_low][HHBBTT::leadingtau] &&
	 tau->pt() < m_pt_threshold[HHBBTT::MTT_low][HHBBTT::leadingtaumax] &&
	 jet0->pt() > m_pt_threshold[HHBBTT::MTT_low][HHBBTT::leadingjet] &&
	 jet1->pt() > m_pt_threshold[HHBBTT::MTT_low][HHBBTT::subleadingjet]);
    }
    else trigPassed_MTT_low = false;

    m_bools.at(HHBBTT::pass_trigger_LTT) =
      (trigPassed_ETT || trigPassed_ETT_4J12 ||
       trigPassed_MTT_2016 || trigPassed_MTT_low || trigPassed_MTT_high);
  }

  void HHbbttSelectorAlg::applySingleTauTriggerSelection
  (const xAOD::EventInfo* event, const trigPassReadDecoMap& triggerdecos,
   const xAOD::TauJet* tau, const tauTrigMatchReadDecoMap& tau_trigMatchDecos){

    bool trigPassed_STT = triggerdecos.at(HHBBTT::STT)(*event);
    if(tau){
      trigPassed_STT &= tau_trigMatchDecos.at(HHBBTT::STT)(*tau);
      trigPassed_STT &= tau->pt() > m_pt_threshold[HHBBTT::STT][HHBBTT::leadingtau];
    }
    else trigPassed_STT = false;

    m_bools.at(HHBBTT::pass_trigger_STT) = trigPassed_STT;
  }

  void HHbbttSelectorAlg::applyDiTauTriggerSelection
  (const xAOD::EventInfo* event, const trigPassReadDecoMap& triggerdecos,
   const xAOD::TauJet* tau0, const xAOD::TauJet* tau1,
   const tauTrigMatchReadDecoMap& tau_trigMatchDecos,
   const xAOD::Jet* jet0, const xAOD::Jet* jet1){

    bool trigPassed_DTT_2016 = triggerdecos.at(HHBBTT::DTT_2016)(*event);
    if(tau0 && tau1 && jet0){
      trigPassed_DTT_2016 &= tau_trigMatchDecos.at(HHBBTT::DTT_2016)(*tau0);
      trigPassed_DTT_2016 &= tau_trigMatchDecos.at(HHBBTT::DTT_2016)(*tau1);
      trigPassed_DTT_2016 &=
	(tau0->pt() > m_pt_threshold[HHBBTT::DTT][HHBBTT::leadingtau] &&
	 tau1->pt() > m_pt_threshold[HHBBTT::DTT][HHBBTT::subleadingtau] &&
	 jet0->pt() > m_pt_threshold[HHBBTT::DTT_2016][HHBBTT::leadingjet]);
    }
    else trigPassed_DTT_2016 = false;
    m_bools.at(HHBBTT::pass_trigger_DTT_2016) = trigPassed_DTT_2016;

    bool trigPassed_DTT_4J12 = triggerdecos.at(HHBBTT::DTT_4J12)(*event);
    if(tau0 && tau1 && jet0 && jet1){
      trigPassed_DTT_4J12 &= tau_trigMatchDecos.at(HHBBTT::DTT_4J12)(*tau0);
      trigPassed_DTT_4J12 &= tau_trigMatchDecos.at(HHBBTT::DTT_4J12)(*tau1);
      trigPassed_DTT_4J12 &=
	(tau0->pt() > m_pt_threshold[HHBBTT::DTT][HHBBTT::leadingtau] &&
	 tau1->pt() > m_pt_threshold[HHBBTT::DTT][HHBBTT::subleadingtau] &&
	 jet0->pt() > m_pt_threshold[HHBBTT::DTT_4J12][HHBBTT::leadingjet] &&
	 jet1->pt() > m_pt_threshold[HHBBTT::DTT_4J12][HHBBTT::subleadingjet]);
    }
    else trigPassed_DTT_4J12 = false;
    m_bools.at(HHBBTT::pass_trigger_DTT_4J12) = trigPassed_DTT_4J12;

    bool trigPassed_DTT_L1Topo = triggerdecos.at(HHBBTT::DTT_L1Topo)(*event);
    if(tau0 && tau1 && jet0){
      trigPassed_DTT_L1Topo &= tau_trigMatchDecos.at(HHBBTT::DTT_L1Topo)(*tau0);
      trigPassed_DTT_L1Topo &= tau_trigMatchDecos.at(HHBBTT::DTT_L1Topo)(*tau1);
      trigPassed_DTT_L1Topo &=
	 (tau0->pt() > m_pt_threshold[HHBBTT::DTT][HHBBTT::leadingtau] &&
	  tau1->pt() > m_pt_threshold[HHBBTT::DTT][HHBBTT::subleadingtau] &&
         tau0->p4().DeltaR(tau1->p4())<2.5 &&
	  jet0->pt() > m_pt_threshold[HHBBTT::DTT_L1Topo][HHBBTT::leadingjet]);
    }
    else trigPassed_DTT_L1Topo = false;
    m_bools.at(HHBBTT::pass_trigger_DTT_L1Topo) = trigPassed_DTT_L1Topo;

    bool trigPassed_DTT_4J12_delayed = triggerdecos.at(HHBBTT::DTT_4J12_delayed)(*event);
    if(tau0 && tau1 && jet0 && jet1){
      trigPassed_DTT_4J12_delayed &= tau_trigMatchDecos.at(HHBBTT::DTT_4J12_delayed)(*tau0);
      trigPassed_DTT_4J12_delayed &= tau_trigMatchDecos.at(HHBBTT::DTT_4J12_delayed)(*tau1);
      trigPassed_DTT_4J12_delayed &=
        (tau0->pt() > m_pt_threshold[HHBBTT::DTT][HHBBTT::leadingtau] &&
         tau1->pt() > m_pt_threshold[HHBBTT::DTT][HHBBTT::subleadingtau] &&
         jet0->pt() > m_pt_threshold[HHBBTT::DTT_4J12][HHBBTT::leadingjet] &&
         jet1->pt() > m_pt_threshold[HHBBTT::DTT_4J12][HHBBTT::subleadingjet]);
    }
    else trigPassed_DTT_4J12_delayed = false;
    m_bools.at(HHBBTT::pass_trigger_DTT_4J12_delayed) = trigPassed_DTT_4J12_delayed;

    bool trigPassed_DTT_L1Topo_delayed = triggerdecos.at(HHBBTT::DTT_L1Topo_delayed)(*event);
    if(tau0 && tau1 && jet0){
      trigPassed_DTT_L1Topo_delayed &= tau_trigMatchDecos.at(HHBBTT::DTT_L1Topo_delayed)(*tau0);
      trigPassed_DTT_L1Topo_delayed &= tau_trigMatchDecos.at(HHBBTT::DTT_L1Topo_delayed)(*tau1);
      trigPassed_DTT_L1Topo_delayed &=
         (tau0->pt() > m_pt_threshold[HHBBTT::DTT][HHBBTT::leadingtau] &&
          tau1->pt() > m_pt_threshold[HHBBTT::DTT][HHBBTT::subleadingtau] &&
         tau0->p4().DeltaR(tau1->p4())<2.5 &&
          jet0->pt() > m_pt_threshold[HHBBTT::DTT_L1Topo][HHBBTT::leadingjet]);
    }
    else trigPassed_DTT_L1Topo_delayed = false;
    m_bools.at(HHBBTT::pass_trigger_DTT_L1Topo_delayed) = trigPassed_DTT_L1Topo_delayed;


    m_bools.at(HHBBTT::pass_trigger_DTT) =
      (m_bools.at(HHBBTT::pass_trigger_DTT_2016) ||
       m_bools.at(HHBBTT::pass_trigger_DTT_4J12) ||
       m_bools.at(HHBBTT::pass_trigger_DTT_L1Topo) ||
       m_bools.at(HHBBTT::pass_trigger_DTT_4J12_delayed) ||
       m_bools.at(HHBBTT::pass_trigger_DTT_L1Topo_delayed));
  }

  
   void HHbbttSelectorAlg::applyDiBJetTriggerSelection
  (const xAOD::EventInfo* event, const trigPassReadDecoMap& triggerdecos,
   const xAOD::Jet* jet0, const xAOD::Jet* jet1){

    bool trigPassed_DBT = triggerdecos.at(HHBBTT::DBT)(*event);
    if( jet0 && jet1){
      //TO DO: implement bjet trig-matching
      trigPassed_DBT &=
	 (jet0->pt() > m_pt_threshold[HHBBTT::DBT][HHBBTT::leadingjet] &&
          jet1->pt() > m_pt_threshold[HHBBTT::DBT][HHBBTT::subleadingjet]);
    }
    else trigPassed_DBT = false;

    m_bools.at(HHBBTT::pass_trigger_DBT) = trigPassed_DBT;
  }


  StatusCode HHbbttSelectorAlg ::initialiseCutflow()
  {
    // CutFlow
    std::vector<std::string> boolnamelist;
    for (const auto& [key, value]: m_boolnames) {
      boolnamelist.push_back(value);
    }
    m_bbttCuts.CheckInputCutList(m_inputCutList,boolnamelist);

    // Initialize an array containing the enum values needed for the cutlist
    m_inputCutKeys.resize(m_inputCutList.size());
    std::vector<bool> inputWasFound (m_inputCutList.size(), false);
    for (const auto& [key, value]: m_boolnames) {
      auto it = std::find(m_inputCutList.begin(), m_inputCutList.end(), value);
      if(it != m_inputCutList.end()) {
        auto index = it - m_inputCutList.begin();
        m_inputCutKeys.at(index) = key;
        inputWasFound.at(index) = true;
      }
    }
    // Check that every element of m_inputCutList has a corresponding enum in m_inputCutEnum
    for (unsigned int index = 0; index < inputWasFound.size(); index++) {
      if(inputWasFound.at(index)) continue;
      ATH_MSG_ERROR("Doubled or falsely spelled cuts in CutList (see config file)." + m_inputCutList[index]);
    }
    // Initialize a vector of CutEntry structs based on the input Cut List
    for (const auto &cut : m_inputCutKeys) {
      m_bbttCuts.add(m_boolnames[cut]);
    }

    //After filling the CutManager, book your histograms.
    const unsigned int nbins = m_bbttCuts.size() + 1; //  need an extra bin for the total num of events.
    ANA_CHECK (book (TEfficiency("AbsoluteEfficiency","Absolute Efficiency of HH->bbtt cuts;Cuts;#epsilon",
				 nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("RelativeEfficiency","Relative Efficiency of HH->bbtt cuts;Cuts;#epsilon",
				 nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("StandardCutFlow","StandardCutFlow of HH->bbtt cuts;Cuts;#epsilon",
                                  nbins, 0.5, nbins + 0.5)));
    if(m_isMC) {
      ANA_CHECK (book (TEfficiency("WeightedAbsoluteEfficiency","Weighted Absolute Efficiency of HH->bbtt cuts;Cuts;#epsilon",
				 nbins, 0.5, nbins + 0.5)));
      ANA_CHECK (book (TEfficiency("WeightedRelativeEfficiency","Weighted Relative Efficiency of HH->bbtt cuts;Cuts;#epsilon",
				 nbins, 0.5, nbins + 0.5)));
      ANA_CHECK (book (TEfficiency("WeightedStandardCutFlow","Weighted StandardCutFlow of HH->bbtt cuts;Cuts;#epsilon",
         nbins, 0.5, nbins + 0.5)));
    }
    ANA_CHECK (book (TH1F("EventsPassed_BinLabeling", "Events passed by each cut / Bin labeling", nbins, 0.5, nbins + 0.5)));
    return StatusCode::SUCCESS;
  }


  void HHbbttSelectorAlg::setThresholds(const xAOD::EventInfo* event,
					const CP::SystematicSet& sys){

    // Global thresholds independent from data-taking period

    // Lepton+tau triggers
    float min_ele_LTT = 18. * Athena::Units::GeV;
    m_pt_threshold[HHBBTT::LTT][HHBBTT::ele] = min_ele_LTT;

    float min_mu_LTT = 15. * Athena::Units::GeV;
    m_pt_threshold[HHBBTT::LTT][HHBBTT::mu] = min_mu_LTT;

    float min_tau_LTT = 30. * Athena::Units::GeV;
    m_pt_threshold[HHBBTT::LTT][HHBBTT::leadingtau] = min_tau_LTT;

    m_pt_threshold[HHBBTT::ETT_4J12][HHBBTT::ele] = min_ele_LTT;
    m_pt_threshold[HHBBTT::ETT_4J12][HHBBTT::leadingtau] = min_tau_LTT;
    m_pt_threshold[HHBBTT::ETT_4J12][HHBBTT::leadingjet] = 45. * Athena::Units::GeV;
    m_pt_threshold[HHBBTT::ETT_4J12][HHBBTT::subleadingjet] = 45. * Athena::Units::GeV;

    m_pt_threshold[HHBBTT::ETT][HHBBTT::ele] = min_ele_LTT;
    m_pt_threshold[HHBBTT::ETT][HHBBTT::leadingtau] = min_tau_LTT;
    m_pt_threshold[HHBBTT::ETT][HHBBTT::leadingjet] = 80. * Athena::Units::GeV;

    m_pt_threshold[HHBBTT::MTT_2016][HHBBTT::mu] = min_mu_LTT;
    m_pt_threshold[HHBBTT::MTT_2016][HHBBTT::leadingtau] = min_tau_LTT;
    m_pt_threshold[HHBBTT::MTT_2016][HHBBTT::leadingjet] = 80. * Athena::Units::GeV;

    float min_tau_MTT_high = 40. * Athena::Units::GeV;
    m_pt_threshold[HHBBTT::MTT_high][HHBBTT::mu] = min_mu_LTT;
    m_pt_threshold[HHBBTT::MTT_high][HHBBTT::leadingtau] = min_tau_MTT_high;
    m_pt_threshold[HHBBTT::MTT_high][HHBBTT::leadingjet] = 45. * Athena::Units::GeV;

    m_pt_threshold[HHBBTT::MTT_low][HHBBTT::mu] = min_mu_LTT;
    m_pt_threshold[HHBBTT::MTT_high][HHBBTT::leadingtau] = min_tau_LTT;
    m_pt_threshold[HHBBTT::MTT_high][HHBBTT::leadingtaumax] = min_tau_MTT_high;
    m_pt_threshold[HHBBTT::MTT_high][HHBBTT::leadingjet] = 45. * Athena::Units::GeV;
    m_pt_threshold[HHBBTT::MTT_high][HHBBTT::subleadingjet] = 45. * Athena::Units::GeV;

    // Single-tau trigger
    m_pt_threshold[HHBBTT::STT][HHBBTT::subleadingtau] = 20. * Athena::Units::GeV;

    // Di-tau triggers
    m_pt_threshold[HHBBTT::DTT_2016][HHBBTT::leadingjet] = 80. * Athena::Units::GeV;
    m_pt_threshold[HHBBTT::DTT_4J12][HHBBTT::leadingjet] = 45. * Athena::Units::GeV;
    m_pt_threshold[HHBBTT::DTT_4J12][HHBBTT::subleadingjet] = 45. * Athena::Units::GeV;

    // Di-b-jets triggers
    m_pt_threshold[HHBBTT::DBT][HHBBTT::leadingjet] = 20. * Athena::Units::GeV;
    m_pt_threshold[HHBBTT::DBT][HHBBTT::subleadingjet] = 20. * Athena::Units::GeV;
    
    // Run-dependent thresholds

    int year = m_year.get(*event, sys);

    // Single-lepton triggers
    if(year==2015)
      m_pt_threshold[HHBBTT::SLT][HHBBTT::ele] = 25. * Athena::Units::GeV;
    else if(m_is2022_75bunches.get(*event, sys))
      m_pt_threshold[HHBBTT::SLT][HHBBTT::ele] = 18. * Athena::Units::GeV;
    else
      m_pt_threshold[HHBBTT::SLT][HHBBTT::ele] = 27. * Athena::Units::GeV;

    if(year==2015)
      m_pt_threshold[HHBBTT::SLT][HHBBTT::mu] = 21. * Athena::Units::GeV;
    else if(2016<=year && year<=2018)
      m_pt_threshold[HHBBTT::SLT][HHBBTT::mu] = 27. * Athena::Units::GeV;
    else
      m_pt_threshold[HHBBTT::SLT][HHBBTT::mu] = 25. * Athena::Units::GeV;

    // Single tau triggers
    float min_tau_STT = 180. * Athena::Units::GeV;
    if(year==2015 || m_is2016_periodA.get(*event, sys))
      min_tau_STT = 100. * Athena::Units::GeV;
    else if(m_is2016_periodB_D3.get(*event, sys))
      min_tau_STT = 140. * Athena::Units::GeV;
    m_pt_threshold[HHBBTT::STT][HHBBTT::leadingtau] = min_tau_STT;

    m_pt_threshold[HHBBTT::DTT][HHBBTT::leadingtau] = 40. * Athena::Units::GeV;
    m_pt_threshold[HHBBTT::DTT][HHBBTT::subleadingtau] = 30. * Athena::Units::GeV;
    m_pt_threshold[HHBBTT::DTT_L1Topo][HHBBTT::leadingjet] = 80. * Athena::Units::GeV;
    if(year >= 2022){
      m_pt_threshold[HHBBTT::DTT][HHBBTT::leadingtau] = 20. * Athena::Units::GeV;
      m_pt_threshold[HHBBTT::DTT][HHBBTT::subleadingtau] = 20. * Athena::Units::GeV;
      m_pt_threshold[HHBBTT::DTT_L1Topo][HHBBTT::leadingjet] = 20. * Athena::Units::GeV;
      m_pt_threshold[HHBBTT::DTT_4J12][HHBBTT::leadingjet] = 20. * Athena::Units::GeV;
      m_pt_threshold[HHBBTT::DTT_4J12][HHBBTT::subleadingjet] = 20. * Athena::Units::GeV;
    }
  }

}
