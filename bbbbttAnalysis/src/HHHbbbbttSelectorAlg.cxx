/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "HHHbbbbttSelectorAlg.h"

#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>
#include <AthContainers/ConstDataVector.h>
#include <AthenaKernel/Units.h>

namespace HHHBBBBTT
{
  HHHbbbbttSelectorAlg ::HHHbbbbttSelectorAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
    : EL::AnaAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode HHHbbbbttSelectorAlg ::initialize()
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

    if (m_saveCutFlow) ATH_CHECK (m_generatorWeight.initialize(m_systematicsList, m_eventHandle));

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
    
    fillLeptonWpDecoMap(m_eleWPNames, m_eleWPDecorHandleMap);
    fillLeptonWpDecoMap(m_muonWPNames, m_muonWPDecorHandleMap);

    ATH_CHECK(m_tauWPDecorHandle.initialize(m_systematicsList, m_tauHandle));
    ATH_CHECK(m_antiTauDecorHandle.initialize(m_systematicsList, m_tauHandle));

    for(auto& [k, handle] : m_eleWPDecorHandleMap)
      ATH_CHECK(handle.initialize(m_systematicsList, m_electronHandle, SG::AllowEmpty));
    for(auto& [k, handle] : m_muonWPDecorHandleMap)
      ATH_CHECK(handle.initialize(m_systematicsList, m_muonHandle, SG::AllowEmpty));

    ATH_CHECK(m_selected_tau.initialize(m_systematicsList, m_tauHandle));
    
    ATH_CHECK(m_selected_el.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK(m_selected_el_isIso.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK(m_selected_mu.initialize(m_systematicsList, m_muonHandle));
    ATH_CHECK(m_selected_mu_isIso.initialize(m_systematicsList, m_muonHandle));
    
    for (const auto& [channel, name] : m_triggerChannels){
      SG::ReadDecorHandleKey<xAOD::EventInfo> deco;
      deco = "EventInfo.pass_trigger_"+name;
      m_trigPass_DecorKey.emplace(channel, deco);
      ATH_CHECK(m_trigPass_DecorKey.at(channel).initialize());
    }

    SG::ReadDecorHandleKey<xAOD::MuonContainer> mu_SLT;
    mu_SLT = m_muonHandle.getNamePattern() + ".trigMatch_SLT";
    m_mu_trigMatch_DecorKey.emplace(HHHBBBBTT::SLT, mu_SLT);
    SG::ReadDecorHandleKey<xAOD::MuonContainer> mu_LTT;
    mu_LTT = m_muonHandle.getNamePattern() + ".trigMatch_LTT";
    m_mu_trigMatch_DecorKey.emplace(HHHBBBBTT::LTT, mu_LTT);
    SG::ReadDecorHandleKey<xAOD::MuonContainer> mu_MTT_2016;
    mu_MTT_2016 = m_muonHandle.getNamePattern() + ".trigMatch_MTT_2016";
    m_mu_trigMatch_DecorKey.emplace(HHHBBBBTT::MTT_2016, mu_MTT_2016);
    SG::ReadDecorHandleKey<xAOD::MuonContainer> mu_MTT_high;
    mu_MTT_high = m_muonHandle.getNamePattern() + ".trigMatch_MTT_high";
    m_mu_trigMatch_DecorKey.emplace(HHHBBBBTT::MTT_high, mu_MTT_high);
    SG::ReadDecorHandleKey<xAOD::MuonContainer> mu_MTT_low;
    mu_MTT_low = m_muonHandle.getNamePattern() + ".trigMatch_MTT_low";
    m_mu_trigMatch_DecorKey.emplace(HHHBBBBTT::MTT_low, mu_MTT_low);

    for (const auto& [channel, key] : m_mu_trigMatch_DecorKey){
      ATH_CHECK(m_mu_trigMatch_DecorKey.at(channel).initialize());
    }
    
    SG::ReadDecorHandleKey<xAOD::ElectronContainer> ele_SLT;
    ele_SLT = m_electronHandle.getNamePattern() + ".trigMatch_SLT";
    m_ele_trigMatch_DecorKey.emplace(HHHBBBBTT::SLT, ele_SLT);
    SG::ReadDecorHandleKey<xAOD::ElectronContainer> ele_LTT;
    ele_LTT = m_electronHandle.getNamePattern() + ".trigMatch_LTT";
    m_ele_trigMatch_DecorKey.emplace(HHHBBBBTT::LTT, ele_LTT);
    SG::ReadDecorHandleKey<xAOD::ElectronContainer> ele_ETT;
    ele_ETT = m_electronHandle.getNamePattern() + ".trigMatch_ETT";
    m_ele_trigMatch_DecorKey.emplace(HHHBBBBTT::ETT, ele_ETT);
    SG::ReadDecorHandleKey<xAOD::ElectronContainer> ele_ETT_4J12;
    ele_ETT_4J12 = m_electronHandle.getNamePattern() + ".trigMatch_ETT_4J12";
    m_ele_trigMatch_DecorKey.emplace(HHHBBBBTT::ETT_4J12, ele_ETT_4J12);
    
    for (const auto& [channel, key] : m_ele_trigMatch_DecorKey){
      ATH_CHECK(m_ele_trigMatch_DecorKey.at(channel).initialize());
    }

    for (const auto& [channel, name] : m_triggerChannels){
      if(channel==HHHBBBBTT::SLT) continue;
      SG::ReadDecorHandleKey<xAOD::TauJetContainer> deco;
      deco = m_tauHandle.getNamePattern() + ".trigMatch_"+name;
      m_tau_trigMatch_DecorKey.emplace(channel, deco);
      ATH_CHECK(m_tau_trigMatch_DecorKey.at(channel).initialize());
    }

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());    
    for ( auto name : m_channel_names){
      if( name == "LepHad") m_channels.push_back(HHHBBBBTT::LepHad);
      else if ( name == "HadHad") m_channels.push_back(HHHBBBBTT::HadHad);
      else if ( name == "ZCR") m_channels.push_back(HHHBBBBTT::ZCR);
      else if ( name == "TopEMuCR") m_channels.push_back(HHHBBBBTT::TopEMuCR);
      else if ( name == "AntiIsoLepHad") m_channels.push_back(HHHBBBBTT::AntiIsoLepHad);
      else{
        ATH_MSG_ERROR("Unknown channel");
        return StatusCode::FAILURE;
      }
    }

    if (m_saveCutFlow) ATH_CHECK (initialiseCutflow());
    return StatusCode::SUCCESS;
  }

    

  StatusCode HHHbbbbttSelectorAlg ::execute()
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
      int n_leptons_looseId_iso = 0;
      int n_leptons_tightId_iso = 0;
      int n_leptons_tightId_antiiso = 0;
      int n_veto_leptons = 0;

      const xAOD::Electron* ele0 = nullptr;
      const xAOD::Electron* ele1 = nullptr;
      for (const xAOD::Electron *electron : *electrons)
      {
        // determine pass/fail ID and isolation:
        bool pass_tightId_looseIso = m_eleWPDecorHandleMap.at(HHHBBBBTT::LepSelWpDeco::tight_iso).get(*electron, sys); 
        bool pass_looseId_looseIso = m_useNonIsoLeptons
                                     ? m_eleWPDecorHandleMap.at(HHHBBBBTT::LepSelWpDeco::loose_iso).get(*electron, sys) 
                                     : true;
        bool pass_tightId_noIso = m_useNonIsoLeptons
                                  ? m_eleWPDecorHandleMap.at(HHHBBBBTT::LepSelWpDeco::tight_noniso).get(*electron, sys) 
                                  : pass_tightId_looseIso;
        bool pass_tightId_antiIso = m_useNonIsoLeptons
                                    ? pass_tightId_noIso && !pass_tightId_looseIso 
                                    : false;

        if (pass_looseId_looseIso) n_leptons_looseId_iso++;

        m_selected_el.set(*electron, false, sys);
        m_selected_el_isIso.set(*electron, false, sys); // slightly misleading default
        if (electron->pt() > m_pt_threshold[HHHBBBBTT::LTT][HHHBBBBTT::ele] &&
            (pass_tightId_looseIso || pass_tightId_antiIso))
        {
          m_selected_el.set(*electron, true, sys);

          // check isolation:
          m_selected_el_isIso.set(*electron, pass_tightId_looseIso, sys);
          if (pass_tightId_looseIso) n_leptons_tightId_iso++;
          else if (pass_tightId_antiIso) n_leptons_tightId_antiiso++;

          if(!ele0) ele0 = electron;
          else if(!ele1) ele1 = electron;
        }
        else
        {
          continue;
        }
      }
      if (ele1) {
        if (ele0->charge() != ele1->charge())
          m_bools.at(HHHBBBBTT::OS_CHARGE_LEPTONS) = true;
      }

      const xAOD::Muon* mu0 = nullptr;
      const xAOD::Muon* mu1 = nullptr;
      for (const xAOD::Muon *muon : *muons)
      {
        // determine pass/fail ID and isolation:
        bool pass_tightId_looseIso = m_muonWPDecorHandleMap.at(HHHBBBBTT::LepSelWpDeco::tight_iso).get(*muon, sys); 
        bool pass_looseId_looseIso = m_useNonIsoLeptons 
                                     ? m_muonWPDecorHandleMap.at(HHHBBBBTT::LepSelWpDeco::loose_iso).get(*muon, sys) 
                                     : true;
        bool pass_tightId_noIso = m_useNonIsoLeptons
                                  ? m_muonWPDecorHandleMap.at(HHHBBBBTT::LepSelWpDeco::tight_noniso).get(*muon, sys) 
                                  : pass_tightId_looseIso;
        bool pass_tightId_antiIso = m_useNonIsoLeptons
                                    ? pass_tightId_noIso && !pass_tightId_looseIso 
                                    : false;
        
        if (pass_looseId_looseIso) n_leptons_looseId_iso++;

        m_selected_mu.set(*muon, false, sys);
        m_selected_mu_isIso.set(*muon, false, sys); // slightly misleading default
        if (std::abs(muon->eta()) < 2.5 &&
	    muon->pt() > m_pt_threshold[HHHBBBBTT::LTT][HHHBBBBTT::mu]
            && (pass_tightId_looseIso || pass_tightId_antiIso) )
        { 
          m_selected_mu.set(*muon, true, sys);
          
          // check isolation:
          m_selected_mu_isIso.set(*muon, pass_tightId_looseIso, sys);
          if (pass_tightId_looseIso) n_leptons_tightId_iso++;
          else if (pass_tightId_antiIso) n_leptons_tightId_antiiso++;

          if(!mu0) mu0 = muon;
          else if(!mu1) mu1 = muon;
        }
        else
        {
          continue;
        }
      }
      if (mu1) {
        if (mu0->charge() != mu1->charge())
          m_bools.at(HHHBBBBTT::OS_CHARGE_LEPTONS) = true;
      } else if (n_leptons_tightId_iso == 2 && mu0) {
        if (ele0->charge() != mu0->charge())
          m_bools.at(HHHBBBBTT::OS_CHARGE_LEPTONS) = true;
      }

      int charge_lepton = 0;
      bool lep_ptcut_SLT = false;
      bool lep_ptcut_LTT = false;
      if(ele0){
	charge_lepton = ele0->charge();
	if (ele0->pt() > m_pt_threshold[HHHBBBBTT::SLT][HHHBBBBTT::ele])
	  lep_ptcut_SLT = true;
	else
	  lep_ptcut_LTT = true;
      }
      if(mu0){
	charge_lepton = mu0->charge();
	if (mu0->pt() > m_pt_threshold[HHHBBBBTT::SLT][HHHBBBBTT::mu])
	  lep_ptcut_SLT = true;
	else
	  lep_ptcut_LTT = true;
      }

      n_veto_leptons = n_leptons_looseId_iso - n_leptons_tightId_iso;

      if (n_leptons_tightId_iso == 1 && n_veto_leptons == 0 && n_leptons_tightId_antiiso == 0)
        m_bools.at(HHHBBBBTT::N_LEPTONS_CUT_LEPHAD) = true;
      else if(n_leptons_tightId_iso == 0 && n_veto_leptons == 0 && n_leptons_tightId_antiiso == 1)
        m_bools.at(HHHBBBBTT::N_LEPTONS_CUT_ANTIISOLEPHAD) = true;

      if (n_leptons_tightId_iso == 0 && n_veto_leptons == 0)
        m_bools.at(HHHBBBBTT::N_LEPTONS_CUT_HADHAD) = true;

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
          passTauWP |= m_antiTauDecorHandle.get(*tau, sys) > 0;
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
	tau0->pt() > m_pt_threshold[HHHBBBBTT::LTT][HHHBBBBTT::leadingtau];
      bool tau_ptcut_STT_lead = n_taus>0 &&
	tau0->pt() > m_pt_threshold[HHHBBBBTT::STT][HHHBBBBTT::leadingtau];
      bool tau_ptcut_STT_sublead = n_taus>1 &&
	tau1->pt() > m_pt_threshold[HHHBBBBTT::STT][HHHBBBBTT::subleadingtau];
      bool tau_ptcut_DTT_lead = n_taus>0 &&
	tau0->pt() > m_pt_threshold[HHHBBBBTT::DTT][HHHBBBBTT::leadingtau];
      bool tau_ptcut_DTT_sublead = n_taus>1 &&
	tau1->pt() > m_pt_threshold[HHHBBBBTT::DTT][HHHBBBBTT::subleadingtau];
      bool tau_ptcut_STT = tau_ptcut_STT_lead && tau_ptcut_STT_sublead;
      bool tau_ptcut_DTT = tau_ptcut_DTT_lead && tau_ptcut_DTT_sublead;



      if (n_taus == 1)
        m_bools.at(HHHBBBBTT::ONE_TAU) = true;

      bool tau_DR_L1Topo = false;
      if (n_taus == 2) {
        m_bools.at(HHHBBBBTT::TWO_TAU) = true;
        tau_DR_L1Topo = tau0->p4().DeltaR(tau1->p4())<2.5;
      }

      TLorentzVector tautau_vis;
      if (n_leptons_tightId_iso+n_leptons_tightId_antiiso == 1 && n_taus == 1) {
        tautau_vis = tau0->p4();
        if(ele0) tautau_vis += ele0->p4();
        else if(mu0) tautau_vis += mu0->p4();
      }
      else if (n_taus == 2) {
        tautau_vis = tau0->p4() + tau1->p4();
      }
      m_bools.at(HHHBBBBTT::MTAUTAU_VIS_MASS) = tautau_vis.M() > 40. * Athena::Units::GeV;

      const xAOD::Jet* jet0 = jets->size()>0 ? jets->at(0) : nullptr;
      const xAOD::Jet* jet1 = jets->size()>1 ? jets->at(1) : nullptr;

      auto eta_lt2p8_jets = std::make_unique<ConstDataVector<xAOD::JetContainer>>(
          SG::VIEW_ELEMENTS);
      auto eta_lt2p5_jets = std::make_unique<ConstDataVector<xAOD::JetContainer>>(
          SG::VIEW_ELEMENTS);

      for (const xAOD::Jet *jet : *jets)
      {
        if(std::abs(jet->eta()) < 2.8)
          eta_lt2p8_jets->push_back(jet);

        if(std::abs(jet->eta()) < 2.5)
          eta_lt2p5_jets->push_back(jet);
      }

      const xAOD::Jet* eta_lt2p8_jet0 = nullptr;
      const xAOD::Jet* eta_lt2p5_jet0 = nullptr;
      const xAOD::Jet* eta_lt2p5_jet1 = nullptr;

      if(eta_lt2p8_jets->size() > 0)
        eta_lt2p8_jet0=eta_lt2p8_jets->at(0);

      if(eta_lt2p5_jets->size() > 0)
        eta_lt2p5_jet0 = eta_lt2p5_jets->at(0);
      
      if(eta_lt2p5_jets->size() > 1)
        eta_lt2p5_jet1 = eta_lt2p5_jets->at(1);
      
      if(m_useTriggerSel){
        applyTriggerSelection(event, trigPass_decos,
                              ele0, ele_trigMatchDecos,
                              mu0, mu_trigMatchDecos,
                              tau0, tau1, tau_trigMatchDecos,
                              jet0, jet1, eta_lt2p5_jet0,
                              eta_lt2p5_jet1,  eta_lt2p8_jet0);
      }
      else{
        m_bools.at(HHHBBBBTT::pass_trigger_SLT) = true;
        m_bools.at(HHHBBBBTT::pass_trigger_LTT) = true;
        m_bools.at(HHHBBBBTT::pass_trigger_STT) = true;
        m_bools.at(HHHBBBBTT::pass_trigger_DTT) = true;
        m_bools.at(HHHBBBBTT::pass_trigger_DTT_2016) = true;
        m_bools.at(HHHBBBBTT::pass_trigger_DTT_4J12) = true;
        m_bools.at(HHHBBBBTT::pass_trigger_DTT_L1Topo) = true;
        m_bools.at(HHHBBBBTT::pass_trigger_DTT_4J12_delayed) = true;
        m_bools.at(HHHBBBBTT::pass_trigger_DTT_L1Topo_delayed) = true;
        m_bools.at(HHHBBBBTT::pass_trigger_DBT) = true;

      }

      m_bools.at(HHHBBBBTT::pass_trigger_SR) =
	(m_bools.at(HHHBBBBTT::pass_trigger_SLT) ||
	 m_bools.at(HHHBBBBTT::pass_trigger_LTT) ||
	 m_bools.at(HHHBBBBTT::pass_trigger_STT) ||
	 m_bools.at(HHHBBBBTT::pass_trigger_DTT) ||
	 m_bools.at(HHHBBBBTT::pass_trigger_DBT));


      //************
      // jet
      //************
      int n_jets = 0;
      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>>(
          SG::VIEW_ELEMENTS);

      for (const xAOD::Jet *jet : *jets)
      {
        if (std::abs(jet->eta()) < 2.5 && jet->pt() > 20. * Athena::Units::GeV)
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

      if (eta_lt2p8_jet0 && eta_lt2p8_jet0->pt() > m_pt_threshold[HHHBBBBTT::DTT_L1Topo][HHHBBBBTT::leadingjet])
        jet_ptcut_DTT_L1Topo = true;

      if (n_jets >= 2)
      {
        m_bools.at(HHHBBBBTT::TWO_JETS) = true;
        if (jet0->pt() > m_pt_threshold[HHHBBBBTT::DTT_2016][HHHBBBBTT::leadingjet])
          jet_ptcut_DTT_2016 = true;
        if (eta_lt2p5_jet0 && eta_lt2p5_jet0->pt() > m_pt_threshold[HHHBBBBTT::DTT_4J12][HHHBBBBTT::leadingjet] &&
            eta_lt2p5_jet1 && eta_lt2p5_jet1->pt() > m_pt_threshold[HHHBBBBTT::DTT_4J12][HHHBBBBTT::subleadingjet])
          jet_ptcut_DTT_4J12 = true;

        if (bjets->size() == 4){
          m_bools.at(HHHBBBBTT::FOUR_BJETS) = true;
        } else if (bjets->size() == 3) {
          m_bools.at(HHHBBBBTT::THREE_BJETS) = true;
        } else if (bjets->size() == 2) {
          m_bools.at(HHHBBBBTT::TWO_BJETS) = true;
        } else if (bjets->size() == 1) {
          m_bools.at(HHHBBBBTT::ONE_BJET) = true;
        } else if (bjets->size() == 0) {
          m_bools.at(HHHBBBBTT::ZERO_BJETS) = true;
        }
      }
      


      //****************
      // event level info
      //****************
      if (n_taus==1 && n_leptons_tightId_iso+n_leptons_tightId_antiiso==1 && tau0->charge() != charge_lepton)
        m_bools.at(HHHBBBBTT::OS_CHARGE_LEPHAD) = true;
      if (n_taus==2 && tau0->charge() == -tau1->charge())
        m_bools.at(HHHBBBBTT::OS_CHARGE_HADHAD) = true;

      if ((m_bools.at(HHHBBBBTT::N_LEPTONS_CUT_LEPHAD) || 
           m_bools.at(HHHBBBBTT::N_LEPTONS_CUT_ANTIISOLEPHAD)) &&
	  m_bools.at(HHHBBBBTT::ONE_TAU) &&
	  m_bools.at(HHHBBBBTT::TWO_JETS)){
        // SLT
        if (lep_ptcut_SLT && tau_ptcut_SLT){
          m_bools.at(pass_baseline_SLT) = true;
          if (m_bools.at(HHHBBBBTT::pass_trigger_SLT)){
            if (m_bools.at(HHHBBBBTT::FOUR_BJETS))
              m_bools.at(HHHBBBBTT::pass_SLT_4B) = true;
            else if (m_bools.at(HHHBBBBTT::THREE_BJETS))
              m_bools.at(HHHBBBBTT::pass_SLT_3B) = true;
            else if (m_bools.at(HHHBBBBTT::TWO_BJETS))
              m_bools.at(HHHBBBBTT::pass_SLT_2B) = true;
            else if (m_bools.at(HHHBBBBTT::ONE_BJET))
              m_bools.at(HHHBBBBTT::pass_SLT_1B) = true;
            else if (m_bools.at(HHHBBBBTT::ZERO_BJETS))
              m_bools.at(HHHBBBBTT::pass_SLT_0B) = true;
          }
        }

        // LTT
        if (lep_ptcut_LTT && tau_ptcut_LTT){
          m_bools.at(HHHBBBBTT::pass_baseline_LTT) = true;
          if (m_bools.at(HHHBBBBTT::pass_trigger_LTT)){
            if (!m_bools.at(HHHBBBBTT::pass_SLT_4B) &&
                m_bools.at(HHHBBBBTT::FOUR_BJETS))
              m_bools.at(HHHBBBBTT::pass_LTT_4B) = true;
            else if (!m_bools.at(HHHBBBBTT::pass_SLT_3B) &&
                m_bools.at(HHHBBBBTT::THREE_BJETS))
              m_bools.at(HHHBBBBTT::pass_LTT_3B) = true;
            else if (!m_bools.at(HHHBBBBTT::pass_SLT_2B) &&
                m_bools.at(HHHBBBBTT::TWO_BJETS))
              m_bools.at(HHHBBBBTT::pass_LTT_2B) = true;
            else if(!m_bools.at(HHHBBBBTT::pass_SLT_1B) &&
                    m_bools.at(HHHBBBBTT::ONE_BJET))
              m_bools.at(HHHBBBBTT::pass_LTT_1B) = true;
              else if(!m_bools.at(HHHBBBBTT::pass_SLT_0B) &&
                    m_bools.at(HHHBBBBTT::ZERO_BJETS))
              m_bools.at(HHHBBBBTT::pass_LTT_0B) = true;
          }
        }
      }

      if (m_bools.at(HHHBBBBTT::N_LEPTONS_CUT_HADHAD) &&
	  m_bools.at(HHHBBBBTT::TWO_TAU) &&
	  m_bools.at(HHHBBBBTT::TWO_JETS)){
        // STT
        if (tau_ptcut_STT){
          m_bools.at(HHHBBBBTT::pass_baseline_STT) = true;
          if (m_bools.at(HHHBBBBTT::pass_trigger_STT)) {
            if (m_bools.at(HHHBBBBTT::FOUR_BJETS)) m_bools.at(HHHBBBBTT::pass_STT_4B) = true;
            else if (m_bools.at(HHHBBBBTT::THREE_BJETS)) m_bools.at(HHHBBBBTT::pass_STT_3B) = true;
            else if (m_bools.at(HHHBBBBTT::TWO_BJETS)) m_bools.at(HHHBBBBTT::pass_STT_2B) = true;
            else if (m_bools.at(HHHBBBBTT::ONE_BJET)) m_bools.at(HHHBBBBTT::pass_STT_1B) = true;
            else if (m_bools.at(HHHBBBBTT::ZERO_BJETS)) m_bools.at(HHHBBBBTT::pass_STT_0B) = true;
          }
        }
        // DTT
        if(!m_bools.at(HHHBBBBTT::pass_baseline_STT) && tau_ptcut_DTT){
          int year = m_year.get(*event, sys);
          if(2015<=year && year<=2016){
            if(jet_ptcut_DTT_2016){
              m_bools.at(HHHBBBBTT::pass_baseline_DTT_2016) = true;
              if (m_bools.at(HHHBBBBTT::pass_trigger_DTT_2016)) {
                if (m_bools.at(HHHBBBBTT::FOUR_BJETS)) m_bools.at(HHHBBBBTT::pass_DTT_2016_4B) = true;
                else if (m_bools.at(HHHBBBBTT::THREE_BJETS)) m_bools.at(HHHBBBBTT::pass_DTT_2016_3B) = true;
                else if (m_bools.at(HHHBBBBTT::TWO_BJETS)) m_bools.at(HHHBBBBTT::pass_DTT_2016_2B) = true;
                else if (m_bools.at(HHHBBBBTT::ONE_BJET)) m_bools.at(HHHBBBBTT::pass_DTT_2016_1B) = true;
                else if (m_bools.at(HHHBBBBTT::ZERO_BJETS)) m_bools.at(HHHBBBBTT::pass_DTT_2016_0B) = true;
              }
            }
          }
	  else{
            if(jet_ptcut_DTT_4J12){
              m_bools.at(HHHBBBBTT::pass_baseline_DTT_4J12) = true;
	      if (m_bools.at(HHHBBBBTT::pass_trigger_DTT_4J12)) {
                if (m_bools.at(HHHBBBBTT::FOUR_BJETS)) m_bools.at(HHHBBBBTT::pass_DTT_4J12_4B) = true;
		            else if (m_bools.at(HHHBBBBTT::THREE_BJETS)) m_bools.at(HHHBBBBTT::pass_DTT_4J12_3B) = true;
                else if (m_bools.at(HHHBBBBTT::TWO_BJETS)) m_bools.at(HHHBBBBTT::pass_DTT_4J12_2B) = true;
		            else if (m_bools.at(HHHBBBBTT::ONE_BJET)) m_bools.at(HHHBBBBTT::pass_DTT_4J12_1B) = true;
                else if (m_bools.at(HHHBBBBTT::ZERO_BJETS)) m_bools.at(HHHBBBBTT::pass_DTT_4J12_0B) = true;
              }
	      if (m_bools.at(HHHBBBBTT::pass_trigger_DTT_4J12_delayed)) {
                if (m_bools.at(HHHBBBBTT::FOUR_BJETS)) m_bools.at(HHHBBBBTT::pass_DTT_4J12_delayed_4B) = true;
		            else if (m_bools.at(HHHBBBBTT::THREE_BJETS)) m_bools.at(HHHBBBBTT::pass_DTT_4J12_delayed_3B) = true;
                else if (m_bools.at(HHHBBBBTT::TWO_BJETS)) m_bools.at(HHHBBBBTT::pass_DTT_4J12_delayed_2B) = true;
		            else if (m_bools.at(HHHBBBBTT::ONE_BJET)) m_bools.at(HHHBBBBTT::pass_DTT_4J12_delayed_1B) = true;
                else if (m_bools.at(HHHBBBBTT::ZERO_BJETS)) m_bools.at(HHHBBBBTT::pass_DTT_4J12_delayed_0B) = true;
              }
            }
	    if(jet_ptcut_DTT_L1Topo && tau_DR_L1Topo){
              m_bools.at(HHHBBBBTT::pass_baseline_DTT_L1Topo) = true;
              if (m_bools.at(HHHBBBBTT::pass_trigger_DTT_L1Topo)) {
                if (m_bools.at(HHHBBBBTT::FOUR_BJETS)) m_bools.at(HHHBBBBTT::pass_DTT_L1Topo_4B) = true;
		            else if (m_bools.at(HHHBBBBTT::THREE_BJETS)) m_bools.at(HHHBBBBTT::pass_DTT_L1Topo_3B) = true;
                else if (m_bools.at(HHHBBBBTT::TWO_BJETS)) m_bools.at(HHHBBBBTT::pass_DTT_L1Topo_2B) = true;
		            else if (m_bools.at(HHHBBBBTT::ONE_BJET)) m_bools.at(HHHBBBBTT::pass_DTT_L1Topo_1B) = true;
                else if (m_bools.at(HHHBBBBTT::ZERO_BJETS)) m_bools.at(HHHBBBBTT::pass_DTT_L1Topo_0B) = true;
              }
              if (m_bools.at(HHHBBBBTT::pass_trigger_DTT_L1Topo_delayed)) {
                if (m_bools.at(HHHBBBBTT::FOUR_BJETS)) m_bools.at(HHHBBBBTT::pass_DTT_L1Topo_delayed_4B) = true;
                else if (m_bools.at(HHHBBBBTT::THREE_BJETS)) m_bools.at(HHHBBBBTT::pass_DTT_L1Topo_delayed_3B) = true;
                else if (m_bools.at(HHHBBBBTT::TWO_BJETS)) m_bools.at(HHHBBBBTT::pass_DTT_L1Topo_delayed_2B) = true;
                else if (m_bools.at(HHHBBBBTT::ONE_BJET)) m_bools.at(HHHBBBBTT::pass_DTT_L1Topo_delayed_1B) = true;
                else if (m_bools.at(HHHBBBBTT::ZERO_BJETS)) m_bools.at(HHHBBBBTT::pass_DTT_L1Topo_delayed_0B) = true;
              }
	    }
          }
        }

        //DBT
        if(!m_bools.at(HHHBBBBTT::pass_baseline_STT)){
          m_bools.at(HHHBBBBTT::pass_baseline_DBT) = true;
          if(m_bools.at(HHHBBBBTT::pass_trigger_DBT)){
            if (m_bools.at(HHHBBBBTT::FOUR_BJETS)) m_bools.at(HHHBBBBTT::pass_DBT_4B) = true;
            else if (m_bools.at(HHHBBBBTT::THREE_BJETS)) m_bools.at(HHHBBBBTT::pass_DBT_3B) = true;
            else if (m_bools.at(HHHBBBBTT::TWO_BJETS)) m_bools.at(HHHBBBBTT::pass_DBT_2B) = true;
            else if (m_bools.at(HHHBBBBTT::ONE_BJET)) m_bools.at(HHHBBBBTT::pass_DBT_1B) = true;
            else if (m_bools.at(HHHBBBBTT::ZERO_BJETS)) m_bools.at(HHHBBBBTT::pass_DBT_0B) = true;
          }
        }
      }

      m_bools.at(HHHBBBBTT::pass_baseline_DTT) =
	      (m_bools.at(HHHBBBBTT::pass_baseline_DTT_2016)  ||
	       m_bools.at(HHHBBBBTT::pass_baseline_DTT_4J12)  ||
               m_bools.at(HHHBBBBTT::pass_baseline_DTT_L1Topo));
      m_bools.at(HHHBBBBTT::pass_DTT_4B) =
	      (m_bools.at(HHHBBBBTT::pass_DTT_2016_4B)  ||
	       m_bools.at(HHHBBBBTT::pass_DTT_4J12_4B)  ||
               m_bools.at(HHHBBBBTT::pass_DTT_L1Topo_4B)  ||
               m_bools.at(HHHBBBBTT::pass_DTT_4J12_delayed_4B)  ||
               m_bools.at(HHHBBBBTT::pass_DTT_L1Topo_delayed_4B));
      m_bools.at(HHHBBBBTT::pass_DTT_3B) =
	      (m_bools.at(HHHBBBBTT::pass_DTT_2016_3B)  ||
	       m_bools.at(HHHBBBBTT::pass_DTT_4J12_3B)  ||
               m_bools.at(HHHBBBBTT::pass_DTT_L1Topo_3B)  ||
               m_bools.at(HHHBBBBTT::pass_DTT_4J12_delayed_3B)  ||
               m_bools.at(HHHBBBBTT::pass_DTT_L1Topo_delayed_3B));
      m_bools.at(HHHBBBBTT::pass_DTT_2B) =
	      (m_bools.at(HHHBBBBTT::pass_DTT_2016_2B)  ||
	       m_bools.at(HHHBBBBTT::pass_DTT_4J12_2B)  ||
               m_bools.at(HHHBBBBTT::pass_DTT_L1Topo_2B)  ||
               m_bools.at(HHHBBBBTT::pass_DTT_4J12_delayed_2B)  ||
               m_bools.at(HHHBBBBTT::pass_DTT_L1Topo_delayed_2B));
      m_bools.at(HHHBBBBTT::pass_DTT_1B) = 
              (m_bools.at(HHHBBBBTT::pass_DTT_2016_1B)  ||
               m_bools.at(HHHBBBBTT::pass_DTT_4J12_1B)  ||
               m_bools.at(HHHBBBBTT::pass_DTT_L1Topo_1B)  ||
               m_bools.at(HHHBBBBTT::pass_DTT_4J12_delayed_1B)  ||
               m_bools.at(HHHBBBBTT::pass_DTT_L1Topo_delayed_1B));
      m_bools.at(HHHBBBBTT::pass_DTT_0B) = 
               (m_bools.at(HHHBBBBTT::pass_DTT_2016_0B)  ||
                m_bools.at(HHHBBBBTT::pass_DTT_4J12_0B)  ||
                m_bools.at(HHHBBBBTT::pass_DTT_L1Topo_0B)  ||
                m_bools.at(HHHBBBBTT::pass_DTT_4J12_delayed_0B)  ||
                m_bools.at(HHHBBBBTT::pass_DTT_L1Topo_delayed_0B));

      m_bools.at(HHHBBBBTT::pass_baseline_SR) =
	(m_bools.at(HHHBBBBTT::pass_baseline_SLT) ||
	 m_bools.at(HHHBBBBTT::pass_baseline_LTT) ||
	 m_bools.at(HHHBBBBTT::pass_baseline_STT) ||
	 m_bools.at(HHHBBBBTT::pass_baseline_DTT) ||
         m_bools.at(HHHBBBBTT::pass_baseline_DBT));

      m_bools.at(HHHBBBBTT::pass_SR_0B) =
  (m_bools.at(HHHBBBBTT::pass_SLT_0B) ||
   m_bools.at(HHHBBBBTT::pass_LTT_0B) ||
   m_bools.at(HHHBBBBTT::pass_STT_0B) ||
   m_bools.at(HHHBBBBTT::pass_DTT_0B) ||
   m_bools.at(HHHBBBBTT::pass_DBT_0B));

      m_bools.at(HHHBBBBTT::pass_SR_1B) =
	(m_bools.at(HHHBBBBTT::pass_SLT_1B) ||
	 m_bools.at(HHHBBBBTT::pass_LTT_1B) ||
	 m_bools.at(HHHBBBBTT::pass_STT_1B) ||
	 m_bools.at(HHHBBBBTT::pass_DTT_1B) ||
	 m_bools.at(HHHBBBBTT::pass_DBT_1B));

      m_bools.at(HHHBBBBTT::pass_SR_2B) =
	(m_bools.at(HHHBBBBTT::pass_SLT_2B) ||
	 m_bools.at(HHHBBBBTT::pass_LTT_2B) ||
	 m_bools.at(HHHBBBBTT::pass_STT_2B) ||
	 m_bools.at(HHHBBBBTT::pass_DTT_2B) ||
	 m_bools.at(HHHBBBBTT::pass_DBT_2B));

      m_bools.at(HHHBBBBTT::pass_SR_3B) =
	(m_bools.at(HHHBBBBTT::pass_SLT_3B) ||
	 m_bools.at(HHHBBBBTT::pass_LTT_3B) ||
	 m_bools.at(HHHBBBBTT::pass_STT_3B) ||
	 m_bools.at(HHHBBBBTT::pass_DTT_3B) ||
	 m_bools.at(HHHBBBBTT::pass_DBT_3B));

      m_bools.at(HHHBBBBTT::pass_SR_4B) =
	(m_bools.at(HHHBBBBTT::pass_SLT_4B) ||
	 m_bools.at(HHHBBBBTT::pass_LTT_4B) ||
	 m_bools.at(HHHBBBBTT::pass_STT_4B) ||
	 m_bools.at(HHHBBBBTT::pass_DTT_4B) ||
	 m_bools.at(HHHBBBBTT::pass_DBT_4B));

      m_bools.at(HHHBBBBTT::pass_baseline_LepHad) =
	(m_bools.at(HHHBBBBTT::pass_baseline_SLT) ||
	 m_bools.at(HHHBBBBTT::pass_baseline_LTT) );
      m_bools.at(HHHBBBBTT::pass_baseline_HadHad) =
	(m_bools.at(HHHBBBBTT::pass_baseline_STT) ||
	 m_bools.at(HHHBBBBTT::pass_baseline_DTT) ||
	 m_bools.at(HHHBBBBTT::pass_baseline_DBT) );
      m_bools.at(HHHBBBBTT::pass_LepHad_4B) =
	(m_bools.at(HHHBBBBTT::pass_SLT_4B) ||
	 m_bools.at(HHHBBBBTT::pass_LTT_4B) );
      m_bools.at(HHHBBBBTT::pass_HadHad_4B) =
	(m_bools.at(HHHBBBBTT::pass_STT_4B) ||
	 m_bools.at(HHHBBBBTT::pass_DTT_4B) ||
	 m_bools.at(HHHBBBBTT::pass_DBT_4B) );
      m_bools.at(HHHBBBBTT::pass_LepHad_3B) =
	(m_bools.at(HHHBBBBTT::pass_SLT_3B) ||
	 m_bools.at(HHHBBBBTT::pass_LTT_3B) );
      m_bools.at(HHHBBBBTT::pass_HadHad_3B) =
	(m_bools.at(HHHBBBBTT::pass_STT_3B) ||
	 m_bools.at(HHHBBBBTT::pass_DTT_3B) ||
	 m_bools.at(HHHBBBBTT::pass_DBT_3B) );
      m_bools.at(HHHBBBBTT::pass_LepHad_2B) =
	(m_bools.at(HHHBBBBTT::pass_SLT_2B) ||
	 m_bools.at(HHHBBBBTT::pass_LTT_2B) );
      m_bools.at(HHHBBBBTT::pass_HadHad_2B) =
	(m_bools.at(HHHBBBBTT::pass_STT_2B) ||
	 m_bools.at(HHHBBBBTT::pass_DTT_2B) ||
	 m_bools.at(HHHBBBBTT::pass_DBT_2B) );
      m_bools.at(HHHBBBBTT::pass_LepHad_1B) =
	(m_bools.at(HHHBBBBTT::pass_SLT_1B) ||
	 m_bools.at(HHHBBBBTT::pass_LTT_1B) );
      m_bools.at(HHHBBBBTT::pass_HadHad_1B) =
	(m_bools.at(HHHBBBBTT::pass_STT_1B) ||
	 m_bools.at(HHHBBBBTT::pass_DTT_1B) ||
	 m_bools.at(HHHBBBBTT::pass_DBT_1B) );
      m_bools.at(HHHBBBBTT::pass_LepHad_0B) =
  (m_bools.at(HHHBBBBTT::pass_SLT_0B) ||
   m_bools.at(HHHBBBBTT::pass_LTT_0B) );
      m_bools.at(HHHBBBBTT::pass_HadHad_0B) =
  (m_bools.at(HHHBBBBTT::pass_STT_0B) ||
   m_bools.at(HHHBBBBTT::pass_DTT_0B) ||
   m_bools.at(HHHBBBBTT::pass_DBT_0B) );
      m_bools.at(HHHBBBBTT::pass_LepHad) =
	(m_bools.at(HHHBBBBTT::pass_LepHad_4B) ||
   m_bools.at(HHHBBBBTT::pass_LepHad_3B) ||
   m_bools.at(HHHBBBBTT::pass_LepHad_2B) ||
	 m_bools.at(HHHBBBBTT::pass_LepHad_1B) ||
	 m_bools.at(HHHBBBBTT::pass_LepHad_0B) );
      m_bools.at(HHHBBBBTT::pass_HadHad) =
	(m_bools.at(HHHBBBBTT::pass_HadHad_4B) ||
   m_bools.at(HHHBBBBTT::pass_HadHad_3B) ||
   m_bools.at(HHHBBBBTT::pass_HadHad_2B) ||
	 m_bools.at(HHHBBBBTT::pass_HadHad_1B) ||
	 m_bools.at(HHHBBBBTT::pass_HadHad_0B) );

      // Z+HF and top (e+mu) control regions
      if (m_bools.at(HHHBBBBTT::pass_trigger_SLT)){
        if(n_leptons_tightId_iso == 2 && n_veto_leptons == 0 && m_bools.at(HHHBBBBTT::TWO_JETS)) {
          float lep1_pt = -999.;
          if (ele1) {
            lep1_pt = ele1->pt();
          }
          else if (mu1) {
            lep1_pt = mu1->pt();
          }
          m_bools.at(HHHBBBBTT::pass_ZCR) = lep1_pt > 27. * Athena::Units::GeV && bjets->size() >= 2;

          // No subleading ele + muon = e+mu event
          m_bools.at(HHHBBBBTT::pass_TopEMuCR) = !ele1 && !mu1 && ele0->pt() > m_pt_threshold[HHHBBBBTT::SLT][HHHBBBBTT::ele] && mu0->pt() > m_pt_threshold[HHHBBBBTT::SLT][HHHBBBBTT::mu] ;
        }
      }

      // fake enriched antiiso-lephad region check:
      m_bools.at(HHHBBBBTT::pass_AntiIsoLepHad) = m_bools.at(HHHBBBBTT::pass_SLT_1B) || m_bools.at(pass_SLT_2B) 
                                               || m_bools.at(HHHBBBBTT::pass_LTT_1B) || m_bools.at(pass_LTT_2B);
      m_bools.at(HHHBBBBTT::pass_AntiIsoLepHad) &= m_bools.at(HHHBBBBTT::N_LEPTONS_CUT_ANTIISOLEPHAD);

      // Channels
      bool pass = false;
      for(const auto& channel : m_channels){
        if(channel == HHHBBBBTT::LepHad){
          pass |= m_bools.at(HHHBBBBTT::pass_LepHad_4B);
          if(m_do0BRegions) pass |= m_bools.at(HHHBBBBTT::pass_LepHad_0B);
          if(m_do1BRegions) pass |= m_bools.at(HHHBBBBTT::pass_LepHad_1B);
          if(m_do2BRegions) pass |= m_bools.at(HHHBBBBTT::pass_LepHad_2B);
          if(m_do3BRegions) pass |= m_bools.at(HHHBBBBTT::pass_LepHad_3B);
        }
        else if(channel == HHHBBBBTT::HadHad){
          pass |= m_bools.at(HHHBBBBTT::pass_HadHad_4B);
          if(m_do0BRegions) pass |= m_bools.at(HHHBBBBTT::pass_HadHad_0B);
          if(m_do1BRegions) pass |= m_bools.at(HHHBBBBTT::pass_HadHad_1B);
          if(m_do2BRegions) pass |= m_bools.at(HHHBBBBTT::pass_HadHad_2B);
          if(m_do3BRegions) pass |= m_bools.at(HHHBBBBTT::pass_HadHad_3B);
        }
        else if(channel == HHHBBBBTT::ZCR) pass |= m_bools.at(HHHBBBBTT::pass_ZCR);
        else if(channel == HHHBBBBTT::TopEMuCR) pass |= m_bools.at(HHHBBBBTT::pass_TopEMuCR);
        else if(channel == HHHBBBBTT::AntiIsoLepHad) pass |= m_bools.at(HHHBBBBTT::pass_AntiIsoLepHad);
      }

      //****************
      // Cutflow
      //****************

      // do the CUTFLOW only with sys="" -> NOSYS
      if (sys.name()=="" && m_saveCutFlow){

        // Compute total_events
        m_total_events+=1;
        if(m_isMC) m_total_mcEventWeight+= m_generatorWeight.get(*event, sys);


        // Count which cuts the event passed
        for (const auto &cut : m_inputCutKeys) {
          if(m_bbbbttCuts.exists(m_boolnames.at(cut))) {
            m_bbbbttCuts(m_boolnames.at(cut)).passed = m_bools.at(cut);
            if (m_bbbbttCuts(m_boolnames.at(cut)).passed) {
              m_bbbbttCuts(m_boolnames.at(cut)).counter += 1;
              if(m_isMC) m_bbbbttCuts(m_boolnames.at(cut)).w_counter += m_generatorWeight.get(*event, sys);
            }
          }
        }

        // Check how many consecutive cuts are passed by the event.
        unsigned int consecutive_cuts = 0;
        for (size_t i = 0; i < m_bbbbttCuts.size(); ++i) {
          if (m_bbbbttCuts[i].passed)
            consecutive_cuts++;
          else
            break;
        }

        // Here we basically increment the  N_events(pass_i  AND pass_i-1  AND ... AND pass_0) for the i-cut.
        for (unsigned int i=0; i<consecutive_cuts; i++) {
          m_bbbbttCuts[i].relativeCounter += 1;
          if(m_isMC) m_bbbbttCuts[i].w_relativeCounter += m_generatorWeight.get(*event, sys);
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

  StatusCode HHHbbbbttSelectorAlg::finalize() {
    ANA_CHECK (m_filterParams.finalize ());

    m_bbbbttCuts.CheckCutResults(); // Print CheckCutResults

    if(m_saveCutFlow) {
      m_bbbbttCuts.DoAbsoluteEfficiency(m_total_events, efficiency("AbsoluteEfficiency"));
      m_bbbbttCuts.DoRelativeEfficiency(m_total_events, efficiency("RelativeEfficiency"));
      m_bbbbttCuts.DoStandardCutFlow(m_total_events, efficiency("StandardCutFlow"));
      if(m_isMC) {
        m_bbbbttCuts.DoWeightedAbsoluteEfficiency(m_total_mcEventWeight, efficiency("WeightedAbsoluteEfficiency"));
        m_bbbbttCuts.DoWeightedRelativeEfficiency(m_total_mcEventWeight, efficiency("WeightedRelativeEfficiency"));
        m_bbbbttCuts.DoWeightedStandardCutFlow(m_total_mcEventWeight, efficiency("WeightedStandardCutFlow"));
      }
      m_bbbbttCuts.DoCutflowLabeling(m_total_events, hist("EventsPassed_BinLabeling"));
    }

    return StatusCode::SUCCESS;
  }

  void HHHbbbbttSelectorAlg::applyTriggerSelection
  (const xAOD::EventInfo* event, const trigPassReadDecoMap& triggerdecos,
   const xAOD::Electron* ele, const eleTrigMatchReadDecoMap& ele_trigMatchDecos,
   const xAOD::Muon* mu, const muTrigMatchReadDecoMap& mu_trigMatchDecos,
   const xAOD::TauJet* tau0, const xAOD::TauJet* tau1,
   const tauTrigMatchReadDecoMap& tau_trigMatchDecos,
   const xAOD::Jet* jet0, const xAOD::Jet* jet1, 
   const xAOD::Jet* eta_lt2p5_jet0, const xAOD::Jet* eta_lt2p5_jet1,
   const xAOD::Jet* eta_lt2p8_jet0){

    // only run trigger selection if in channel
    bool use_SLT = false;
    bool use_LTT = false;
    bool use_STT = false;
    bool use_DTT = false;
    bool use_DBT = false;
    for (const auto &channel : m_channels){
      if (channel == HHHBBBBTT::LepHad || channel == HHHBBBBTT::AntiIsoLepHad){
        use_SLT = true;
        use_LTT = true;
      }
      else if (channel == HHHBBBBTT::HadHad){
        use_STT = true;
        use_DTT = true;
        use_DBT = true;
      }
      else if (channel == HHHBBBBTT::ZCR || channel == HHHBBBBTT::TopEMuCR){
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
				 tau0, tau1, tau_trigMatchDecos, jet0,
         eta_lt2p5_jet0, eta_lt2p5_jet1, eta_lt2p8_jet0);
    }
    if(use_DBT){
      applyDiBJetTriggerSelection(event, triggerdecos,
         eta_lt2p5_jet0, eta_lt2p5_jet1);
    }

  }

  void HHHbbbbttSelectorAlg::applySingleLepTriggerSelection
  (const xAOD::EventInfo* event, const trigPassReadDecoMap& triggerdecos,
   const xAOD::Electron* ele, const eleTrigMatchReadDecoMap& ele_trigMatchDecos,
   const xAOD::Muon* mu, const muTrigMatchReadDecoMap& mu_trigMatchDecos) {

    bool trigPassed_SET = triggerdecos.at(HHHBBBBTT::SLT)(*event);
    if(ele){
      trigPassed_SET &= ele_trigMatchDecos.at(HHHBBBBTT::SLT)(*ele);
      trigPassed_SET &= ele->pt() > m_pt_threshold[HHHBBBBTT::SLT][HHHBBBBTT::ele];
    }
    else trigPassed_SET = false;

    bool trigPassed_SMT = triggerdecos.at(HHHBBBBTT::SLT)(*event);
    if(mu){
      trigPassed_SMT &= mu_trigMatchDecos.at(HHHBBBBTT::SLT)(*mu);
      trigPassed_SMT &= mu->pt() > m_pt_threshold[HHHBBBBTT::SLT][HHHBBBBTT::mu];
    }
    else trigPassed_SMT = false;

    m_bools.at(HHHBBBBTT::pass_trigger_SLT) = (trigPassed_SET || trigPassed_SMT);
  }

  void HHHbbbbttSelectorAlg::applyLepHadTriggerSelection
  (const xAOD::EventInfo* event,  const trigPassReadDecoMap& triggerdecos,
   const xAOD::Electron* ele, const eleTrigMatchReadDecoMap& ele_trigMatchDecos,
   const xAOD::Muon* mu, const muTrigMatchReadDecoMap& mu_trigMatchDecos,
   const xAOD::TauJet* tau, const tauTrigMatchReadDecoMap& tau_trigMatchDecos,
   const xAOD::Jet* jet0, const xAOD::Jet* jet1){

    bool trigPassed_ETT_4J12 = triggerdecos.at(HHHBBBBTT::ETT_4J12)(*event);
    if(ele && tau && jet0 && jet1){
      trigPassed_ETT_4J12 &= ele_trigMatchDecos.at(HHHBBBBTT::ETT_4J12)(*ele);
      trigPassed_ETT_4J12 &= tau_trigMatchDecos.at(HHHBBBBTT::ETT_4J12)(*tau);
      trigPassed_ETT_4J12 &=
	(ele->pt() > m_pt_threshold[HHHBBBBTT::ETT_4J12][HHHBBBBTT::ele] &&
	 tau->pt() > m_pt_threshold[HHHBBBBTT::ETT_4J12][HHHBBBBTT::leadingtau] &&
	 jet0->pt() > m_pt_threshold[HHHBBBBTT::ETT_4J12][HHHBBBBTT::leadingjet] &&
	 jet1->pt() > m_pt_threshold[HHHBBBBTT::ETT_4J12][HHHBBBBTT::subleadingjet]);
    }
    else trigPassed_ETT_4J12 = false;

    bool trigPassed_ETT = triggerdecos.at(HHHBBBBTT::ETT)(*event);
    if(ele && tau && jet0){
      trigPassed_ETT &= ele_trigMatchDecos.at(HHHBBBBTT::ETT)(*ele);
      trigPassed_ETT &= tau_trigMatchDecos.at(HHHBBBBTT::ETT)(*tau);
      trigPassed_ETT &=
	(ele->pt() > m_pt_threshold[HHHBBBBTT::ETT][HHHBBBBTT::ele] &&
	 tau->pt() > m_pt_threshold[HHHBBBBTT::ETT][HHHBBBBTT::leadingtau] &&
	 jet0->pt() > m_pt_threshold[HHHBBBBTT::ETT][HHHBBBBTT::leadingjet]);
    }
    else trigPassed_ETT = false;

    bool trigPassed_MTT_2016 = triggerdecos.at(HHHBBBBTT::MTT_2016)(*event);
    if(mu && tau && jet0){
      trigPassed_MTT_2016 &= mu_trigMatchDecos.at(HHHBBBBTT::MTT_2016)(*mu);
      trigPassed_MTT_2016 &= tau_trigMatchDecos.at(HHHBBBBTT::MTT_2016)(*tau);
      trigPassed_MTT_2016 &=
	(mu->pt() > m_pt_threshold[HHHBBBBTT::MTT_2016][HHHBBBBTT::mu] &&
	 tau->pt() > m_pt_threshold[HHHBBBBTT::MTT_2016][HHHBBBBTT::leadingtau] &&
	 jet0->pt() > m_pt_threshold[HHHBBBBTT::MTT_2016][HHHBBBBTT::leadingjet]);
    }
    else trigPassed_MTT_2016 = false;

    bool trigPassed_MTT_high = triggerdecos.at(HHHBBBBTT::MTT_high)(*event);
    if(mu && tau && jet0){
      trigPassed_MTT_high &= mu_trigMatchDecos.at(HHHBBBBTT::MTT_high)(*mu);
      trigPassed_MTT_high &= tau_trigMatchDecos.at(HHHBBBBTT::MTT_high)(*tau);
      trigPassed_MTT_high &=
	(mu->pt() > m_pt_threshold[HHHBBBBTT::MTT_high][HHHBBBBTT::mu] &&
	 tau->pt() > m_pt_threshold[HHHBBBBTT::MTT_high][HHHBBBBTT::leadingtau] &&
	 jet0->pt() > m_pt_threshold[HHHBBBBTT::MTT_high][HHHBBBBTT::leadingjet]);
    }
    else trigPassed_MTT_high = false;

    bool trigPassed_MTT_low = triggerdecos.at(HHHBBBBTT::MTT_low)(*event);  
    /*Eta cuts not applied for LTT as they are in DTT
    If LTT is included again, this should be revisited*/

    if(mu && tau && jet0 && jet1){
      trigPassed_MTT_low &= mu_trigMatchDecos.at(HHHBBBBTT::MTT_low)(*mu);
      trigPassed_MTT_low &= tau_trigMatchDecos.at(HHHBBBBTT::MTT_low)(*tau);
      trigPassed_MTT_low &=
        (mu->pt() > m_pt_threshold[HHHBBBBTT::MTT_low][HHHBBBBTT::mu] &&
         tau->pt() > m_pt_threshold[HHHBBBBTT::MTT_low][HHHBBBBTT::leadingtau] &&
         tau->pt() < m_pt_threshold[HHHBBBBTT::MTT_low][HHHBBBBTT::leadingtaumax] &&
         jet0->pt() > m_pt_threshold[HHHBBBBTT::MTT_low][HHHBBBBTT::leadingjet] &&
         jet1->pt() > m_pt_threshold[HHHBBBBTT::MTT_low][HHHBBBBTT::subleadingjet]);
    }
    else trigPassed_MTT_low = false;

    m_bools.at(HHHBBBBTT::pass_trigger_LTT) =
      (trigPassed_ETT || trigPassed_ETT_4J12 ||
       trigPassed_MTT_2016 || trigPassed_MTT_low || trigPassed_MTT_high);
  }

  void HHHbbbbttSelectorAlg::applySingleTauTriggerSelection
  (const xAOD::EventInfo* event, const trigPassReadDecoMap& triggerdecos,
   const xAOD::TauJet* tau, const tauTrigMatchReadDecoMap& tau_trigMatchDecos){

    bool trigPassed_STT = triggerdecos.at(HHHBBBBTT::STT)(*event);
    if(tau){
      trigPassed_STT &= tau_trigMatchDecos.at(HHHBBBBTT::STT)(*tau);
      trigPassed_STT &= tau->pt() > m_pt_threshold[HHHBBBBTT::STT][HHHBBBBTT::leadingtau];
    }
    else trigPassed_STT = false;

    m_bools.at(HHHBBBBTT::pass_trigger_STT) = trigPassed_STT;
  }

  void HHHbbbbttSelectorAlg::applyDiTauTriggerSelection
  (const xAOD::EventInfo* event, const trigPassReadDecoMap& triggerdecos,
   const xAOD::TauJet* tau0, const xAOD::TauJet* tau1,
   const tauTrigMatchReadDecoMap& tau_trigMatchDecos, const xAOD::Jet* jet0,
   const xAOD::Jet* eta_lt2p5_jet0, const xAOD::Jet* eta_lt2p5_jet1,
   const xAOD::Jet* eta_lt2p8_jet0){

    bool trigPassed_DTT_2016 = triggerdecos.at(HHHBBBBTT::DTT_2016)(*event);
    if(tau0 && tau1 && jet0){
      trigPassed_DTT_2016 &= tau_trigMatchDecos.at(HHHBBBBTT::DTT_2016)(*tau0);
      trigPassed_DTT_2016 &= tau_trigMatchDecos.at(HHHBBBBTT::DTT_2016)(*tau1);
      trigPassed_DTT_2016 &=
        (tau0->pt() > m_pt_threshold[HHHBBBBTT::DTT][HHHBBBBTT::leadingtau] &&
         tau1->pt() > m_pt_threshold[HHHBBBBTT::DTT][HHHBBBBTT::subleadingtau] &&
         jet0->pt() > m_pt_threshold[HHHBBBBTT::DTT_2016][HHHBBBBTT::leadingjet]);
    }
    else trigPassed_DTT_2016 = false;
    m_bools.at(HHHBBBBTT::pass_trigger_DTT_2016) = trigPassed_DTT_2016;

    bool trigPassed_DTT_4J12 = triggerdecos.at(HHHBBBBTT::DTT_4J12)(*event);
    if(tau0 && tau1 && eta_lt2p5_jet0 && eta_lt2p5_jet1){
      trigPassed_DTT_4J12 &= tau_trigMatchDecos.at(HHHBBBBTT::DTT_4J12)(*tau0);
      trigPassed_DTT_4J12 &= tau_trigMatchDecos.at(HHHBBBBTT::DTT_4J12)(*tau1);
      trigPassed_DTT_4J12 &=
        (tau0->pt() > m_pt_threshold[HHHBBBBTT::DTT][HHHBBBBTT::leadingtau] &&
         tau1->pt() > m_pt_threshold[HHHBBBBTT::DTT][HHHBBBBTT::subleadingtau] &&
         eta_lt2p5_jet0->pt() > m_pt_threshold[HHHBBBBTT::DTT_4J12][HHHBBBBTT::leadingjet] &&
         eta_lt2p5_jet1->pt() > m_pt_threshold[HHHBBBBTT::DTT_4J12][HHHBBBBTT::subleadingjet]);
    }
    else trigPassed_DTT_4J12 = false;
    m_bools.at(HHHBBBBTT::pass_trigger_DTT_4J12) = trigPassed_DTT_4J12;

    bool trigPassed_DTT_L1Topo = triggerdecos.at(HHHBBBBTT::DTT_L1Topo)(*event);
    if(tau0 && tau1 && eta_lt2p8_jet0){
      trigPassed_DTT_L1Topo &= tau_trigMatchDecos.at(HHHBBBBTT::DTT_L1Topo)(*tau0);
      trigPassed_DTT_L1Topo &= tau_trigMatchDecos.at(HHHBBBBTT::DTT_L1Topo)(*tau1);
      trigPassed_DTT_L1Topo &=
        (tau0->pt() > m_pt_threshold[HHHBBBBTT::DTT][HHHBBBBTT::leadingtau] &&
         tau1->pt() > m_pt_threshold[HHHBBBBTT::DTT][HHHBBBBTT::subleadingtau] &&
         tau0->p4().DeltaR(tau1->p4())<2.5 &&
         eta_lt2p8_jet0->pt() > m_pt_threshold[HHHBBBBTT::DTT_L1Topo][HHHBBBBTT::leadingjet]);
    }
    else trigPassed_DTT_L1Topo = false;
    m_bools.at(HHHBBBBTT::pass_trigger_DTT_L1Topo) = trigPassed_DTT_L1Topo;

    bool trigPassed_DTT_4J12_delayed = triggerdecos.at(HHHBBBBTT::DTT_4J12_delayed)(*event);
    if(tau0 && tau1 && eta_lt2p5_jet0 && eta_lt2p5_jet1){
      trigPassed_DTT_4J12_delayed &= tau_trigMatchDecos.at(HHHBBBBTT::DTT_4J12_delayed)(*tau0);
      trigPassed_DTT_4J12_delayed &= tau_trigMatchDecos.at(HHHBBBBTT::DTT_4J12_delayed)(*tau1);
      trigPassed_DTT_4J12_delayed &=
        (tau0->pt() > m_pt_threshold[HHHBBBBTT::DTT][HHHBBBBTT::leadingtau] &&
         tau1->pt() > m_pt_threshold[HHHBBBBTT::DTT][HHHBBBBTT::subleadingtau] &&
         eta_lt2p5_jet0->pt() > m_pt_threshold[HHHBBBBTT::DTT_4J12][HHHBBBBTT::leadingjet] &&
         eta_lt2p5_jet1->pt() > m_pt_threshold[HHHBBBBTT::DTT_4J12][HHHBBBBTT::subleadingjet]);
    }
    else trigPassed_DTT_4J12_delayed = false;
    m_bools.at(HHHBBBBTT::pass_trigger_DTT_4J12_delayed) = trigPassed_DTT_4J12_delayed;

    bool trigPassed_DTT_L1Topo_delayed = triggerdecos.at(HHHBBBBTT::DTT_L1Topo_delayed)(*event);
    if(tau0 && tau1 && eta_lt2p8_jet0){
      trigPassed_DTT_L1Topo_delayed &= tau_trigMatchDecos.at(HHHBBBBTT::DTT_L1Topo_delayed)(*tau0);
      trigPassed_DTT_L1Topo_delayed &= tau_trigMatchDecos.at(HHHBBBBTT::DTT_L1Topo_delayed)(*tau1);
      trigPassed_DTT_L1Topo_delayed &=
        (tau0->pt() > m_pt_threshold[HHHBBBBTT::DTT][HHHBBBBTT::leadingtau] &&
         tau1->pt() > m_pt_threshold[HHHBBBBTT::DTT][HHHBBBBTT::subleadingtau] &&
         tau0->p4().DeltaR(tau1->p4())<2.5 && 
         eta_lt2p8_jet0->pt() > m_pt_threshold[HHHBBBBTT::DTT_L1Topo][HHHBBBBTT::leadingjet]);
    }
    else trigPassed_DTT_L1Topo_delayed = false;
    m_bools.at(HHHBBBBTT::pass_trigger_DTT_L1Topo_delayed) = trigPassed_DTT_L1Topo_delayed;


    m_bools.at(HHHBBBBTT::pass_trigger_DTT) =
      (m_bools.at(HHHBBBBTT::pass_trigger_DTT_2016) ||
       m_bools.at(HHHBBBBTT::pass_trigger_DTT_4J12) ||
       m_bools.at(HHHBBBBTT::pass_trigger_DTT_L1Topo) ||
       m_bools.at(HHHBBBBTT::pass_trigger_DTT_4J12_delayed) ||
       m_bools.at(HHHBBBBTT::pass_trigger_DTT_L1Topo_delayed));
  }

  
  void HHHbbbbttSelectorAlg::applyDiBJetTriggerSelection
  (const xAOD::EventInfo* event, const trigPassReadDecoMap& triggerdecos,
   const xAOD::Jet* eta_lt2p5_jet0, const xAOD::Jet* eta_lt2p5_jet1){

    bool trigPassed_DBT = triggerdecos.at(HHHBBBBTT::DBT)(*event);
    if(eta_lt2p5_jet0 && eta_lt2p5_jet1){
      //TO DO: implement bjet trig-matching
      trigPassed_DBT &=
        (eta_lt2p5_jet0->pt() > m_pt_threshold[HHHBBBBTT::DBT][HHHBBBBTT::leadingjet] &&
         eta_lt2p5_jet1->pt() > m_pt_threshold[HHHBBBBTT::DBT][HHHBBBBTT::subleadingjet]);
    }
    else trigPassed_DBT = false;

    m_bools.at(HHHBBBBTT::pass_trigger_DBT) = trigPassed_DBT;
  }


  StatusCode HHHbbbbttSelectorAlg ::initialiseCutflow()
  {
    // CutFlow
    std::vector<std::string> boolnamelist;
    for (const auto& [key, value]: m_boolnames) {
      boolnamelist.push_back(value);
    }
    m_bbbbttCuts.CheckInputCutList(m_inputCutList,boolnamelist);

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
      m_bbbbttCuts.add(m_boolnames[cut]);
    }

    //After filling the CutManager, book your histograms.
    const unsigned int nbins = m_bbbbttCuts.size() + 1; //  need an extra bin for the total num of events.
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


  void HHHbbbbttSelectorAlg::setThresholds(const xAOD::EventInfo* event,
					const CP::SystematicSet& sys){

    // Global thresholds independent from data-taking period

    // Lepton+tau triggers
    float min_ele_LTT = 18. * Athena::Units::GeV;
    m_pt_threshold[HHHBBBBTT::LTT][HHHBBBBTT::ele] = min_ele_LTT;

    float min_mu_LTT = 15. * Athena::Units::GeV;
    m_pt_threshold[HHHBBBBTT::LTT][HHHBBBBTT::mu] = min_mu_LTT;

    float min_tau_LTT = 30. * Athena::Units::GeV;
    m_pt_threshold[HHHBBBBTT::LTT][HHHBBBBTT::leadingtau] = min_tau_LTT;

    m_pt_threshold[HHHBBBBTT::ETT_4J12][HHHBBBBTT::ele] = min_ele_LTT;
    m_pt_threshold[HHHBBBBTT::ETT_4J12][HHHBBBBTT::leadingtau] = min_tau_LTT;
    m_pt_threshold[HHHBBBBTT::ETT_4J12][HHHBBBBTT::leadingjet] = 45. * Athena::Units::GeV;
    m_pt_threshold[HHHBBBBTT::ETT_4J12][HHHBBBBTT::subleadingjet] = 45. * Athena::Units::GeV;

    m_pt_threshold[HHHBBBBTT::ETT][HHHBBBBTT::ele] = min_ele_LTT;
    m_pt_threshold[HHHBBBBTT::ETT][HHHBBBBTT::leadingtau] = min_tau_LTT;
    m_pt_threshold[HHHBBBBTT::ETT][HHHBBBBTT::leadingjet] = 80. * Athena::Units::GeV;

    m_pt_threshold[HHHBBBBTT::MTT_2016][HHHBBBBTT::mu] = min_mu_LTT;
    m_pt_threshold[HHHBBBBTT::MTT_2016][HHHBBBBTT::leadingtau] = min_tau_LTT;
    m_pt_threshold[HHHBBBBTT::MTT_2016][HHHBBBBTT::leadingjet] = 80. * Athena::Units::GeV;

    float min_tau_MTT_high = 40. * Athena::Units::GeV;
    m_pt_threshold[HHHBBBBTT::MTT_high][HHHBBBBTT::mu] = min_mu_LTT;
    m_pt_threshold[HHHBBBBTT::MTT_high][HHHBBBBTT::leadingtau] = min_tau_MTT_high;
    m_pt_threshold[HHHBBBBTT::MTT_high][HHHBBBBTT::leadingjet] = 45. * Athena::Units::GeV;

    m_pt_threshold[HHHBBBBTT::MTT_low][HHHBBBBTT::mu] = min_mu_LTT;
    m_pt_threshold[HHHBBBBTT::MTT_high][HHHBBBBTT::leadingtau] = min_tau_LTT;
    m_pt_threshold[HHHBBBBTT::MTT_high][HHHBBBBTT::leadingtaumax] = min_tau_MTT_high;
    m_pt_threshold[HHHBBBBTT::MTT_high][HHHBBBBTT::leadingjet] = 45. * Athena::Units::GeV;
    m_pt_threshold[HHHBBBBTT::MTT_high][HHHBBBBTT::subleadingjet] = 45. * Athena::Units::GeV;

    // For data-taking period dependent thresholds 
    int year = m_year.get(*event, sys);

    // Single-tau trigger
    m_pt_threshold[HHHBBBBTT::STT][HHHBBBBTT::subleadingtau] = 20. * Athena::Units::GeV;
    float min_tau_STT = 180. * Athena::Units::GeV;
    if(year==2015 || m_is2016_periodA.get(*event, sys))
      min_tau_STT = 100. * Athena::Units::GeV;
    else if(m_is2016_periodB_D3.get(*event, sys))
      min_tau_STT = 140. * Athena::Units::GeV;
    m_pt_threshold[HHHBBBBTT::STT][HHHBBBBTT::leadingtau] = min_tau_STT;

    // Di-tau triggers
    m_pt_threshold[HHHBBBBTT::DTT_2016][HHHBBBBTT::leadingjet] = 70. * Athena::Units::GeV;
    m_pt_threshold[HHHBBBBTT::DTT_L1Topo][HHHBBBBTT::leadingjet] = 70. * Athena::Units::GeV;
    m_pt_threshold[HHHBBBBTT::DTT_4J12][HHHBBBBTT::leadingjet] = 50. * Athena::Units::GeV;
    m_pt_threshold[HHHBBBBTT::DTT_4J12][HHHBBBBTT::subleadingjet] = 50. * Athena::Units::GeV;
    m_pt_threshold[HHHBBBBTT::DTT][HHHBBBBTT::leadingtau] = 40. * Athena::Units::GeV;
    m_pt_threshold[HHHBBBBTT::DTT][HHHBBBBTT::subleadingtau] = 30. * Athena::Units::GeV;
    if(year >= 2022){
      m_pt_threshold[HHHBBBBTT::DTT][HHHBBBBTT::leadingtau] = 20. * Athena::Units::GeV;
      m_pt_threshold[HHHBBBBTT::DTT][HHHBBBBTT::subleadingtau] = 20. * Athena::Units::GeV;
      m_pt_threshold[HHHBBBBTT::DTT_L1Topo][HHHBBBBTT::leadingjet] = 20. * Athena::Units::GeV;
      m_pt_threshold[HHHBBBBTT::DTT_4J12][HHHBBBBTT::leadingjet] = 20. * Athena::Units::GeV;
      m_pt_threshold[HHHBBBBTT::DTT_4J12][HHHBBBBTT::subleadingjet] = 20. * Athena::Units::GeV;
    }

    // Di-b-jets triggers
    m_pt_threshold[HHHBBBBTT::DBT][HHHBBBBTT::leadingjet] = 20. * Athena::Units::GeV;
    m_pt_threshold[HHHBBBBTT::DBT][HHHBBBBTT::subleadingjet] = 20. * Athena::Units::GeV;
    
    // Single-lepton triggers
    if(year==2015)
      m_pt_threshold[HHHBBBBTT::SLT][HHHBBBBTT::ele] = 25. * Athena::Units::GeV;
    else if(m_is2022_75bunches.get(*event, sys))
      m_pt_threshold[HHHBBBBTT::SLT][HHHBBBBTT::ele] = 18. * Athena::Units::GeV;
    else
      m_pt_threshold[HHHBBBBTT::SLT][HHHBBBBTT::ele] = 27. * Athena::Units::GeV;

    if(year==2015)
      m_pt_threshold[HHHBBBBTT::SLT][HHHBBBBTT::mu] = 21. * Athena::Units::GeV;
    else if(2016<=year && year<=2018)
      m_pt_threshold[HHHBBBBTT::SLT][HHHBBBBTT::mu] = 27. * Athena::Units::GeV;
    else
      m_pt_threshold[HHHBBBBTT::SLT][HHHBBBBTT::mu] = 25. * Athena::Units::GeV;

  }

  void HHHbbbbttSelectorAlg::fillLeptonWpDecoMap(const std::vector<std::string>& wpNames, 
          leptonDecoMap& decoMap){
    for(auto& wp : wpNames){
      CP::SysReadDecorHandle<char> 
        handle{"baselineSelection_"+wp+"_%SYS%", this};
      
      // nottva must be included in the working points used in selecting leptons:
      if(wp.find("nottva") == std::string::npos) continue;

      // TODO: handle more complicated WP lists
      bool isTight = !wp.starts_with("Loose");
      bool isIso = wp.find("NonIso") == std::string::npos;
      
      if (!isTight && isIso){ 
        decoMap.emplace(HHHBBBBTT::LepSelWpDeco::loose_iso, handle);
        ATH_MSG_INFO("found loose iso wp = "<< wp);
      }
      if (isTight && !isIso){ 
        decoMap.emplace(HHHBBBBTT::LepSelWpDeco::tight_noniso, handle);
        ATH_MSG_INFO("found tight noniso wp = "<< wp);
      }
      if (isTight  && isIso){ 
        decoMap.emplace(HHHBBBBTT::LepSelWpDeco::tight_iso, handle);
        ATH_MSG_INFO("found tight iso wp = "<< wp); 
      }
    }
  }

}
