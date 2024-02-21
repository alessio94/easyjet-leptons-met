/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Carl Gwilliam

#include "HHbbttSelectorAlg.h"
#include <AsgDataHandles/ReadDecorHandle.h>

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
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    ATH_CHECK (m_runNumber.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_rdmRunNumber.initialize(m_systematicsList, m_eventHandle));

    // Intialise booleans with value false. Also initialise syst-aware output decorators
    for (auto& [key, value] : m_boolnames) {
      m_bools.emplace(key, false);
      CP::SysWriteDecorHandle<bool> whandle{value+"_%SYS%", this};
      m_Bbranches.emplace(key, whandle);
      ATH_CHECK(m_Bbranches.at(key).initialize(m_systematicsList, m_eventHandle));
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

    ATH_CHECK(m_matchingTool.retrieve());

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
    m_bools.at(HHBBTT::is15) = std::find(m_years.begin(), m_years.end(), 2015) != m_years.end();
    m_bools.at(HHBBTT::is16) = std::find(m_years.begin(), m_years.end(), 2016) != m_years.end();
    m_bools.at(HHBBTT::is17) = std::find(m_years.begin(), m_years.end(), 2017) != m_years.end();
    m_bools.at(HHBBTT::is18) = std::find(m_years.begin(), m_years.end(), 2018) != m_years.end();
    m_bools.at(HHBBTT::is22) = std::find(m_years.begin(), m_years.end(), 2022) != m_years.end();
    m_bools.at(HHBBTT::is23) = std::find(m_years.begin(), m_years.end(), 2023) != m_years.end();

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
    m_pt_threshold[HHBBTT::STT][HHBBTT::subleadingtau] = 25. * Athena::Units::GeV;

    // Di-tau triggers
    m_pt_threshold[HHBBTT::DTT][HHBBTT::leadingtau] = 40. * Athena::Units::GeV;
    m_pt_threshold[HHBBTT::DTT][HHBBTT::subleadingtau] = 30. * Athena::Units::GeV;

    m_pt_threshold[HHBBTT::DTT_2016][HHBBTT::leadingjet] = 80. * Athena::Units::GeV;
    m_pt_threshold[HHBBTT::DTT_L1Topo][HHBBTT::leadingjet] = 80. * Athena::Units::GeV;
    m_pt_threshold[HHBBTT::DTT_4J12][HHBBTT::leadingjet] = 45. * Athena::Units::GeV;
    m_pt_threshold[HHBBTT::DTT_4J12][HHBBTT::subleadingjet] = 45. * Athena::Units::GeV;

    ATH_CHECK (initialiseCutflow());

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

      // Retrieve inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      // Set run number dependent quantities for nominal systematics
      if(sys.name()==""){
        unsigned int rdmNumber = m_isMC ? m_rdmRunNumber.get(*event, sys) : m_runNumber.get(*event,sys);
        setRunNumberQuantities(rdmNumber);
      }

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

      //trigger flags
      m_bools.at(HHBBTT::pass_trigger_SLT) = false;
      m_bools.at(HHBBTT::pass_trigger_LTT) = false;
      m_bools.at(HHBBTT::pass_trigger_STT) = false;
      m_bools.at(HHBBTT::pass_trigger_DTT) = false;
      m_bools.at(HHBBTT::pass_trigger_DTT_2016) = false;
      m_bools.at(HHBBTT::pass_trigger_DTT_4J12) = false;
      m_bools.at(HHBBTT::pass_trigger_DTT_L1Topo) = false;

      // Reset event specific booleans to false.
      m_bools.at(HHBBTT::TWO_JETS) = false;
      m_bools.at(HHBBTT::TWO_BJETS) = false;
      m_bools.at(HHBBTT::MBB_MASS) = false;
      // flags specific for lephad
      m_bools.at(HHBBTT::N_LEPTONS_CUT_LEPHAD) = false;
      m_bools.at(HHBBTT::ONE_TAU) = false;
      m_bools.at(HHBBTT::OS_CHARGE_LEPHAD) = false;
      m_bools.at(HHBBTT::pass_baseline_SLT) = false;
      m_bools.at(HHBBTT::pass_baseline_LTT) = false;
      m_bools.at(HHBBTT::pass_SLT) = false;
      m_bools.at(HHBBTT::pass_LTT) = false;
      // flags specific for hadhad
      m_bools.at(HHBBTT::N_LEPTONS_CUT_HADHAD) = false;
      m_bools.at(HHBBTT::TWO_TAU) = false;
      m_bools.at(HHBBTT::OS_CHARGE_HADHAD) = false;
      m_bools.at(HHBBTT::pass_baseline_STT) = false;
      m_bools.at(HHBBTT::pass_baseline_DTT_2016) = false;
      m_bools.at(HHBBTT::pass_baseline_DTT_4J12) = false;
      m_bools.at(HHBBTT::pass_baseline_DTT_L1Topo) = false;
      m_bools.at(HHBBTT::pass_baseline_DTT) = false;
      m_bools.at(HHBBTT::pass_STT) = false;
      m_bools.at(HHBBTT::pass_DTT_2016) = false;
      m_bools.at(HHBBTT::pass_DTT_4J12) = false;
      m_bools.at(HHBBTT::pass_DTT_L1Topo) = false;
      m_bools.at(HHBBTT::pass_DTT) = false;
    

      //************
      // lepton
      //************
      int n_leptons = 0;
      int n_looseleptons = 0;

      const xAOD::Electron* ele0 = nullptr;
      for (const xAOD::Electron *electron : *electrons)
      {
        bool passElectronWP = eleWPDecorHandle(*electron);
        m_selected_el.set(*electron, false, sys);
        if (passElectronWP &&
	    electron->pt() > m_pt_threshold[HHBBTT::LTT][HHBBTT::ele])
	{
          m_selected_el.set(*electron, true, sys);
          if(!ele0) ele0 = electron;
          n_leptons += 1;
        }
        else
          n_looseleptons += 1;
      }

      const xAOD::Muon* mu0 = nullptr;
      for (const xAOD::Muon *muon : *muons)
      {
        bool passMuonWP = muonWPDecorHandle(*muon);
        m_selected_mu.set(*muon, false, sys);
        if (passMuonWP && std::abs(muon->eta()) < 2.5 &&
	    muon->pt() > m_pt_threshold[HHBBTT::LTT][HHBBTT::mu])
        {
          m_selected_mu.set(*muon, true, sys);
          if(!mu0) mu0 = muon;
          n_leptons += 1;
        }
        else
          n_looseleptons += 1;
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
        bool isTauID = tauWPDecorHandle(*tau);
        m_selected_tau.set(*tau, false, sys);
        if (isTauID && tau->pt() > 20. * Athena::Units::GeV)
        {
          m_selected_tau.set(*tau, true, sys);
          n_taus += 1;
          if(n_taus==1) tau0 = tau;
          else if(n_taus==2) tau1 = tau;
        }
      }

      bool tau_ptcut_SLT = n_taus>0 && std::abs(tau0->eta());
      bool tau_ptcut_LTT = n_taus>0 && std::abs(tau0->eta()) &&
	tau0->pt() > m_pt_threshold[HHBBTT::LTT][HHBBTT::leadingtau];
      bool tau_ptcut_STT_lead = n_taus>0 &&
	tau0->pt() > m_pt_threshold[HHBBTT::STT][HHBBTT::leadingtau];
      bool tau_ptcut_STT_sublead = n_taus>1 &&
	tau1->pt() > m_pt_threshold[HHBBTT::STT][HHBBTT::subleadingtau];
      bool tau_ptcut_DTT_lead = n_taus>0 &&
	tau0->pt() > m_pt_threshold[HHBBTT::DTT][HHBBTT::leadingtau];
      bool tau_ptcut_DTT_sublead = n_taus>1 &&
	tau1->pt() > m_pt_threshold[HHBBTT::STT][HHBBTT::subleadingtau];
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

      applyTriggerSelection(event, ele0, mu0, tau0, tau1, jet0, jet1, sys);

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
          m_bools.at(HHBBTT::TWO_BJETS) = (bjets->at(0)->pt() > 45. * Athena::Units::GeV &&
          bjets->at(1)->pt() > 20. * Athena::Units::GeV);
          bb = bjets->at(0)->p4() + bjets->at(1)->p4();
          mbb = bb.M();
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
          if (m_bools.at(HHBBTT::pass_trigger_SLT) &&
	      m_bools.at(HHBBTT::TWO_BJETS) &&
	      m_bools.at(HHBBTT::MBB_MASS) &&
	      m_bools.at(HHBBTT::OS_CHARGE_LEPHAD))
            m_bools.at(HHBBTT::pass_SLT) = true;
        }
        // LTT
        if (lep_ptcut_LTT && tau_ptcut_LTT){
          m_bools.at(HHBBTT::pass_baseline_LTT) = true;
          if (!m_bools.at(HHBBTT::pass_SLT) &&
	      m_bools.at(HHBBTT::pass_trigger_LTT) &&
	      m_bools.at(HHBBTT::TWO_BJETS) &&
	      m_bools.at(HHBBTT::MBB_MASS) &&
	      m_bools.at(HHBBTT::OS_CHARGE_LEPHAD))
            m_bools.at(HHBBTT::pass_LTT) = true;
        }
      }

      if (m_bools.at(HHBBTT::N_LEPTONS_CUT_HADHAD) &&
	  m_bools.at(HHBBTT::TWO_TAU) &&
	  m_bools.at(HHBBTT::TWO_JETS)){
        // STT
        if (tau_ptcut_STT){
          m_bools.at(HHBBTT::pass_baseline_STT) = true;
          if (m_bools.at(HHBBTT::pass_trigger_STT) &&
	      m_bools.at(HHBBTT::TWO_BJETS) &&
	      m_bools.at(HHBBTT::OS_CHARGE_HADHAD))
            m_bools.at(HHBBTT::pass_STT) = true;
        }
        // DTT
        if(!m_bools.at(HHBBTT::pass_STT) && tau_ptcut_DTT){
          if(m_bools.at(HHBBTT::is15) || m_bools.at(HHBBTT::is16)){
            if(jet_ptcut_DTT_2016){
              m_bools.at(HHBBTT::pass_baseline_DTT_2016) = true;
              if (m_bools.at(HHBBTT::pass_trigger_DTT_2016) &&
		  m_bools.at(HHBBTT::TWO_BJETS) &&
		  m_bools.at(HHBBTT::OS_CHARGE_HADHAD))
                m_bools.at(HHBBTT::pass_DTT_2016) = true;
            }
          }
          else if(jet_ptcut_DTT_4J12){
            m_bools.at(HHBBTT::pass_baseline_DTT_4J12) = true;
            if (m_bools.at(HHBBTT::pass_trigger_DTT_4J12) &&
		m_bools.at(HHBBTT::TWO_BJETS) &&
		m_bools.at(HHBBTT::OS_CHARGE_HADHAD))
              m_bools.at(HHBBTT::pass_DTT_4J12) = true;
          }
          else if(jet_ptcut_DTT_L1Topo && tau_DR_L1Topo){
            m_bools.at(HHBBTT::pass_baseline_DTT_L1Topo) = true;
            if (m_bools.at(HHBBTT::pass_trigger_DTT_L1Topo) &&
		m_bools.at(HHBBTT::TWO_BJETS) &&
		m_bools.at(HHBBTT::OS_CHARGE_HADHAD))
              m_bools.at(HHBBTT::pass_DTT_L1Topo) = true;
          }
        }
      }

      m_bools.at(HHBBTT::pass_baseline_DTT) =
	(m_bools.at(HHBBTT::pass_baseline_DTT_2016) ||
	 m_bools.at(HHBBTT::pass_baseline_DTT_4J12) ||
	 m_bools.at(HHBBTT::pass_baseline_DTT_L1Topo));
      m_bools.at(HHBBTT::pass_DTT) =
	(m_bools.at(HHBBTT::pass_DTT_2016) ||
	 m_bools.at(HHBBTT::pass_DTT_4J12) ||
	 m_bools.at(HHBBTT::pass_DTT_L1Topo));

      bool pass = false;
      for(const auto& channel : m_channels){
       if(channel == HHBBTT::LepHad) pass |= (m_bools.at(HHBBTT::pass_SLT) || m_bools.at(HHBBTT::pass_LTT));
       else if(channel == HHBBTT::HadHad) pass |= (m_bools.at(HHBBTT::pass_STT) || m_bools.at(HHBBTT::pass_DTT));
      }

      //****************
      // Cutflow
      //****************

      // do the CUTFLOW only with sys="" -> NOSYS
      if (sys.name()==""){

        // Compute total_events
        m_total_events+=1; 

        // Count which cuts the event passed
        for (const auto &cut : m_inputCutKeys) {
          if(m_bbttCuts.exists(m_boolnames.at(cut))) {
            m_bbttCuts(m_boolnames.at(cut)).passed = m_bools.at(cut);
            if (m_bbttCuts(m_boolnames.at(cut)).passed)
              m_bbttCuts(m_boolnames.at(cut)).counter+=1;
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
          m_bbttCuts[i].relativeCounter+=1;
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
      m_bbttCuts.DoCutflowLabeling(m_total_events, hist("EventsPassed_BinLabeling"));
    }
    else {
      delete efficiency("AbsoluteEfficiency");
      delete efficiency("RelativeEfficiency");
      delete efficiency("StandardCutFlow");
      delete hist("EventsPassed_BinLabeling");
    }

    return StatusCode::SUCCESS;
  }

  void HHbbttSelectorAlg::applyTriggerSelection
  (const xAOD::EventInfo* event,
   const xAOD::Electron* ele, const xAOD::Muon* mu,
   const xAOD::TauJet* tau0, const xAOD::TauJet* tau1,
   const xAOD::Jet* jet0, const xAOD::Jet* jet1,
   const CP::SystematicSet& sys){
    //************
    // trigger selecton
    //************

    // only run trigger selection if in channel
    for (const auto &channel : m_channels){
      if (channel == HHBBTT::LepHad)
      {
        applySingleLepTriggerSelection(event, ele, mu, sys);
        applyLepHadTriggerSelection(event, ele, mu, tau0, jet0, jet1, sys);
      }
      else if (channel == HHBBTT::HadHad)
      {
        applySingleTauTriggerSelection(event, tau0, sys);
        applyDiTauTriggerSelection(event, tau0, tau1, jet0, jet1, sys);
      }
    }
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
    for (const auto &cut : m_inputCutKeys)  { 
      m_bbttCuts.add(m_boolnames[cut]);
    }

    //After filling the CutManager, book your histograms.
    const unsigned int nbins = m_bbttCuts.size() + 1; //  need an extra bin for the total num of events.
    ANA_CHECK (book (TEfficiency("AbsoluteEfficiency","Absolute Efficiency of HH->bbyy cuts;Cuts;#epsilon", 
                                  nbins, 0.5, nbins + 0.5))); 
    ANA_CHECK (book (TEfficiency("RelativeEfficiency","Relative Efficiency of HH->bbyy cuts;Cuts;#epsilon", 
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("StandardCutFlow","StandardCutFlow of HH->bbyy cuts;Cuts;#epsilon", 
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TH1F("EventsPassed_BinLabeling", "Events passed by each cut / Bin labeling", nbins, 0.5, nbins + 0.5)));    
    return StatusCode::SUCCESS;
  }

  void HHbbttSelectorAlg ::applySingleLepTriggerSelection(
      const xAOD::EventInfo *event,
      const xAOD::Electron* ele,
      const xAOD::Muon* mu,
      const CP::SystematicSet &sys)
  {
    bool trigPassed_SET = false;
    bool trigPassed_SMT = false;

    std::vector<std::string> single_ele_paths;
    std::vector<std::string> single_mu_paths;

    // SLT
    if(m_bools.at(HHBBTT::is15)){
      single_ele_paths = {"trigPassed_HLT_e24_lhmedium_L1EM20VH",
			  "trigPassed_HLT_e60_lhmedium",
			  "trigPassed_HLT_e120_lhloose"};
      single_mu_paths = {"trigPassed_HLT_mu20_iloose_L1MU15",
			 "trigPassed_HLT_mu50"};
    }
    else if(m_bools.at(HHBBTT::is16) || m_bools.at(HHBBTT::is17) ||
	    m_bools.at(HHBBTT::is18)){
      single_ele_paths = {"trigPassed_HLT_e26_lhtight_nod0_ivarloose",
			  "trigPassed_HLT_e60_lhmedium_nod0",
			  "trigPassed_HLT_e140_lhloose_nod0"};
      single_mu_paths = {"trigPassed_HLT_mu26_ivarmedium",
			 "trigPassed_HLT_mu50"};
    }
    else if(m_bools.at(HHBBTT::is22)){
      single_ele_paths = {"trigPassed_HLT_e26_lhtight_ivarloose_L1EM22VHI",
			  "trigPassed_HLT_e60_lhmedium_L1EM22VHI",
			  "trigPassed_HLT_e140_lhloose_L1EM22VHI"};
      single_mu_paths = {"trigPassed_HLT_mu24_ivarmedium_L1MU14FCH",
			 "trigPassed_HLT_mu50_L1MU14FCH"};
    }
    else if(m_bools.at(HHBBTT::is23)){
      single_ele_paths = {"trigPassed_HLT_e26_lhtight_ivarloose_L1eEM26M",
			  "trigPassed_HLT_e60_lhmedium_L1eEM26M",
			  "trigPassed_HLT_e140_lhloose_L1eEM26M"};
      single_mu_paths = {"trigPassed_HLT_mu24_ivarmedium_L1MU14FCH",
			 "trigPassed_HLT_mu50_L1MU14FCH"};
    }

    // Pass single electron trigger
    for(const auto& path : single_ele_paths){
      // Drop trigPassed_ prefix
      std::string trig = path.substr(11,path.size());
      trigPassed_SET |= (m_triggerdecos.at(path).get(*event, sys) &&
			 ele && m_matchingTool->match(*ele, trig));
      if(trigPassed_SET) break;
    }

    if(!ele) trigPassed_SET = false;
    if(trigPassed_SET)
      trigPassed_SET &= ele->pt() > m_pt_threshold[HHBBTT::SLT][HHBBTT::ele];

    // Pass single muon trigger
    for(const auto& path : single_mu_paths){
      // Drop trigPassed_ prefix
      std::string trig = path.substr(11,path.size());
      trigPassed_SMT |= (m_triggerdecos.at(path).get(*event, sys) &&
			 mu && m_matchingTool->match(*mu, trig));
      if(trigPassed_SMT) break;
    }

    if(!mu) trigPassed_SMT = false;
    if(trigPassed_SMT)
      trigPassed_SMT &= mu->pt() > m_pt_threshold[HHBBTT::SLT][HHBBTT::mu];

    m_bools.at(HHBBTT::pass_trigger_SLT) = (trigPassed_SET || trigPassed_SMT);
  }

  void HHbbttSelectorAlg ::applyLepHadTriggerSelection(
      const xAOD::EventInfo *event,
      const xAOD::Electron* ele, const xAOD::Muon* mu,
      const xAOD::TauJet* tau,
      const xAOD::Jet* jet0, const xAOD::Jet* jet1,
      const CP::SystematicSet &sys)
  {
    bool trigPassed_ETT = false;
    bool trigPassed_ETT_4J12 = false;
    bool trigPassed_MTT_2016 = false;
    bool trigPassed_MTT_low  = false;
    bool trigPassed_MTT_high = false;

    std::vector<std::string> ele_tau_paths;
    std::vector<std::string> ele_tau_paths_4J12;
    std::vector<std::string> mu_tau_paths_2016;
    std::vector<std::string> mu_tau_paths_low;
    std::vector<std::string> mu_tau_paths_high;

    // LTT
    if(m_bools.at(HHBBTT::is15) || m_bools.at(HHBBTT::is16PeriodA)){
      ele_tau_paths = {"trigPassed_HLT_e17_lhmedium_nod0_tau25_medium1_tracktwo"};
      mu_tau_paths_2016 = {"trigPassed_HLT_mu14_tau25_medium1_tracktwo"};
    }
    else if(m_bools.at(HHBBTT::is16PeriodB_D3) ||
	    m_bools.at(HHBBTT::is16PeriodD4_end)){
      ele_tau_paths = {"trigPassed_HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwo"};
      mu_tau_paths_2016 = {"trigPassed_HLT_mu14_ivarloose_tau25_medium1_tracktwo"};
    }
    else if(m_bools.at(HHBBTT::is17PeriodB1_B4) ||
	    m_bools.at(HHBBTT::is17PeriodB5_B7) ||
	    m_bools.at(HHBBTT::is17PeriodB8_end)){
      ele_tau_paths = {"trigPassed_HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwo"};
      ele_tau_paths_4J12 = {"trigPassed_HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwo_L1EM15VHI_2TAU12IM_4J12"};
      mu_tau_paths_low = {"trigPassed_HLT_mu14_ivarloose_tau25_medium1_tracktwo_L1MU10_TAU12IM_3J12"};
      mu_tau_paths_high = {"trigPassed_HLT_mu14_ivarloose_tau35_medium1_tracktwo"};
    }
    else if(m_bools.at(HHBBTT::is18PeriodB_end)){
      ele_tau_paths = {"trigPassed_HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwoEF"};
      ele_tau_paths_4J12 = {"trigPassed_HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwoEF_L1EM15VHI_2TAU12IM_4J12"};
      mu_tau_paths_low = {"trigPassed_HLT_mu14_ivarloose_tau25_medium1_tracktwoEF_L1MU10_TAU12IM_3J12"};
      mu_tau_paths_high = {"trigPassed_HLT_mu14_ivarloose_tau35_medium1_tracktwoEF"};
      if(m_bools.at(HHBBTT::is18PeriodK_end)){
        ele_tau_paths.push_back("trigPassed_HLT_e17_lhmedium_nod0_ivarloose_tau25_mediumRNN_tracktwoMVA");
        ele_tau_paths_4J12.push_back("trigPassed_HLT_e17_lhmedium_nod0_ivarloose_tau25_mediumRNN_tracktwoMVA_L1EM15VHI_2TAU12IM_4J12");
        mu_tau_paths_low.push_back("trigPassed_HLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA_L1MU10_TAU12IM_3J12");
        mu_tau_paths_high.push_back("trigPassed_HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA");
      }
    }
    else if(m_bools.at(HHBBTT::is22)){
      ele_tau_paths = {"trigPassed_HLT_e24_lhmedium_ivarloose_tau20_mediumRNN_tracktwoMVA_03dRAB_L1EM22VHI"};
      ele_tau_paths_4J12 = {"trigPassed_HLT_e17_lhmedium_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1EM15VHI_2TAU12IM_4J12"};
      mu_tau_paths_low = {"trigPassed_HLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1MU8F_TAU12IM_3J12"};
      mu_tau_paths_high = {"trigPassed_HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA_03dRAB_L1MU8F_TAU20IM"};
    }
    else if(m_bools.at(HHBBTT::is23)){
      ele_tau_paths = {"trigPassed_HLT_e24_lhmedium_ivarloose_tau20_mediumRNN_tracktwoMVA_03dRAB_L1eEM26M"};
      ele_tau_paths_4J12 = {"trigPassed_HLT_e17_lhmedium_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1EM15VHI_2TAU12IM_4J12"};
      mu_tau_paths_low = {"trigPassed_HLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1MU8F_TAU12IM_3J12"};
      mu_tau_paths_high = {"trigPassed_HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA_03dRAB_L1MU8F_TAU20IM"};
    }

    // Pass electron + tau trigger
    for(const auto& path : ele_tau_paths_4J12){
      // Drop trigPassed_ prefix
      std::string trig = path.substr(11,path.size());
      trigPassed_ETT_4J12 |= (m_triggerdecos.at(path).get(*event, sys) &&
			      ele && m_matchingTool->match(*ele, trig) &&
			      tau && m_matchingTool->match(*tau, trig, 0.2));
      if(trigPassed_ETT_4J12){
	ele_tau_paths = {}; // Skip other triggers
	break;
      }
    }

    if(!ele || !tau || !jet0 || !jet1) trigPassed_ETT_4J12 = false;
    if(trigPassed_ETT_4J12)
      trigPassed_ETT_4J12 &=
	(ele->pt() > m_pt_threshold[HHBBTT::ETT_4J12][HHBBTT::ele] &&
	 tau->pt() > m_pt_threshold[HHBBTT::ETT_4J12][HHBBTT::leadingtau] &&
	 jet0->pt() > m_pt_threshold[HHBBTT::ETT_4J12][HHBBTT::leadingjet] &&
	 jet1->pt() > m_pt_threshold[HHBBTT::ETT_4J12][HHBBTT::subleadingjet]);

    for(const auto& path : ele_tau_paths){
      // Drop trigPassed_ prefix
      std::string trig = path.substr(11,path.size());
      trigPassed_ETT |= (m_triggerdecos.at(path).get(*event, sys) &&
			 ele && m_matchingTool->match(*ele, trig) &&
			 tau && m_matchingTool->match(*tau, trig, 0.2));
      if(trigPassed_ETT) break;
    }

    if(!ele || !tau || !jet0) trigPassed_ETT = false;
    if(trigPassed_ETT)
      trigPassed_ETT &=
	(ele->pt() > m_pt_threshold[HHBBTT::ETT][HHBBTT::ele] &&
	 tau->pt() > m_pt_threshold[HHBBTT::ETT][HHBBTT::leadingtau] &&
	 jet0->pt() > m_pt_threshold[HHBBTT::ETT][HHBBTT::leadingjet]);

    // Pass muon + tau trigger
    for(const auto& path : mu_tau_paths_2016){
      // Drop trigPassed_ prefix
      std::string trig = path.substr(11,path.size());
      trigPassed_MTT_2016 |= (m_triggerdecos.at(path).get(*event, sys) &&
			      mu && m_matchingTool->match(*mu, trig) &&
			      tau && m_matchingTool->match(*tau, trig, 0.2));
      if(trigPassed_MTT_2016) break;
    }

    if(!mu || !tau || !jet0) trigPassed_MTT_2016 = false;
    if(trigPassed_MTT_2016)
      trigPassed_MTT_2016 &=
	(mu->pt() > m_pt_threshold[HHBBTT::MTT_2016][HHBBTT::mu] &&
	 tau->pt() > m_pt_threshold[HHBBTT::MTT_2016][HHBBTT::leadingtau] &&
	 jet0->pt() > m_pt_threshold[HHBBTT::MTT_2016][HHBBTT::leadingjet]);

    for(const auto& path : mu_tau_paths_high){
      // Drop trigPassed_ prefix
      std::string trig = path.substr(11,path.size());
      trigPassed_MTT_high |= (m_triggerdecos.at(path).get(*event, sys) &&
			      mu && m_matchingTool->match(*mu, trig) &&
			      tau && m_matchingTool->match(*tau, trig, 0.2));
      if(trigPassed_MTT_high) break;
    }

    if(!mu || !tau || !jet0) trigPassed_MTT_high = false;
    if(trigPassed_MTT_high)
      trigPassed_MTT_high &=
	(mu->pt() > m_pt_threshold[HHBBTT::MTT_high][HHBBTT::mu] &&
	 tau->pt() > m_pt_threshold[HHBBTT::MTT_high][HHBBTT::leadingtau] &&
	 jet0->pt() > m_pt_threshold[HHBBTT::MTT_high][HHBBTT::leadingjet]);

    for(const auto& path : mu_tau_paths_low){
      // Drop trigPassed_ prefix
      std::string trig = path.substr(11,path.size());
      trigPassed_MTT_low |= (m_triggerdecos.at(path).get(*event, sys) &&
			     mu && m_matchingTool->match(*mu, trig) &&
			     tau && m_matchingTool->match(*tau, trig, 0.2));
      if(trigPassed_MTT_low) break;
    }

    if(!mu || !tau || !jet0 || !jet1) trigPassed_MTT_low = false;
    if(trigPassed_MTT_low)
      trigPassed_MTT_low &=
	(mu->pt() > m_pt_threshold[HHBBTT::MTT_low][HHBBTT::mu] &&
	 tau->pt() > m_pt_threshold[HHBBTT::MTT_low][HHBBTT::leadingtau] &&
	 tau->pt() < m_pt_threshold[HHBBTT::MTT_low][HHBBTT::leadingtaumax] &&
	 jet0->pt() > m_pt_threshold[HHBBTT::MTT_low][HHBBTT::leadingjet] &&
	 jet1->pt() > m_pt_threshold[HHBBTT::MTT_low][HHBBTT::subleadingjet]);

    m_bools.at(HHBBTT::pass_trigger_LTT) =
      (trigPassed_ETT || trigPassed_ETT_4J12 ||
       trigPassed_MTT_2016 || trigPassed_MTT_low || trigPassed_MTT_high);
  }

  void HHbbttSelectorAlg ::applySingleTauTriggerSelection(
      const xAOD::EventInfo *event,
      const xAOD::TauJet* tau,
      const CP::SystematicSet &sys)
  {

    std::vector<std::string> single_tau_paths;

    if(m_bools.at(HHBBTT::is15) || m_bools.at(HHBBTT::is16PeriodA)){
      single_tau_paths = {"trigPassed_HLT_tau80_medium1_tracktwo_L1TAU60"};
    }
    else if(m_bools.at(HHBBTT::is16PeriodB_D3)){
      single_tau_paths = {"trigPassed_HLT_tau125_medium1_tracktwo"};
    }
    else if(m_bools.at(HHBBTT::is16PeriodD4_end) ||
	    m_bools.at(HHBBTT::is17PeriodB1_B4)){
      single_tau_paths = {"trigPassed_HLT_tau160_medium1_tracktwo"};
    }
    else if(m_bools.at(HHBBTT::is17PeriodB5_B7) ||
	    m_bools.at(HHBBTT::is17PeriodB8_end)){
      single_tau_paths = {"trigPassed_HLT_tau160_medium1_tracktwo_L1TAU100"};
    }
    else if(m_bools.at(HHBBTT::is18)){
      single_tau_paths = {"trigPassed_HLT_tau160_medium1_tracktwoEF_L1TAU100"};
      if(m_bools.at(HHBBTT::is18PeriodK_end)){
        single_tau_paths.push_back("trigPassed_HLT_tau160_mediumRNN_tracktwoMVA_L1TAU100");
      }
    }
    else if(m_bools.at(HHBBTT::is22)){
      single_tau_paths = {"trigPassed_HLT_tau160_mediumRNN_tracktwoMVA_L1TAU100"};
    }
    else if(m_bools.at(HHBBTT::is23)){
      single_tau_paths = {"trigPassed_HLT_tau160_mediumRNN_tracktwoMVA_L1eTAU140"};
    }
    // Pass single tau trigger
    for(const auto& path : single_tau_paths){
      // Drop trigPassed_ prefix
      std::string trig = path.substr(11,path.size());
      m_bools.at(HHBBTT::pass_trigger_STT) |=
	(m_triggerdecos.at(path).get(*event, sys) &&
	 (!m_useDiTauTrigMatch || (tau && m_matchingTool->match(*tau, trig, 0.2))));
     if(m_bools.at(HHBBTT::pass_trigger_STT)) break;
    }

    if(!tau) m_bools.at(HHBBTT::pass_trigger_STT) = false;
    if(m_bools.at(HHBBTT::pass_trigger_STT))
      m_bools.at(HHBBTT::pass_trigger_STT) &=
	tau->pt() > m_pt_threshold[HHBBTT::STT][HHBBTT::leadingtau];

  }

  void HHbbttSelectorAlg::applyDiTauTriggerSelection(
      const xAOD::EventInfo *event,
      const xAOD::TauJet* tau0, const xAOD::TauJet* tau1,
      const xAOD::Jet* jet0, const xAOD::Jet* jet1,
      const CP::SystematicSet &sys)
  {

    std::vector<std::string> ditau_paths_2016;
    std::vector<std::string> ditau_paths_L1Topo;
    std::vector<std::string> ditau_paths_4J12;

    if(m_bools.at(HHBBTT::is15)){
      ditau_paths_2016 = {"trigPassed_HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1TAU20IM_2TAU12IM"};
    }
    else if(m_bools.at(HHBBTT::is16) || m_bools.at(HHBBTT::is17)){
      if(m_bools.at(HHBBTT::is16PeriodA) ||
	 m_bools.at(HHBBTT::is16PeriodB_D3) ||
	 m_bools.at(HHBBTT::is16PeriodD4_end)){
        ditau_paths_2016 = {"trigPassed_HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo"};
      }
      else if(m_bools.at(HHBBTT::l1topo_disabled)){
        ditau_paths_2016 = {"trigPassed_HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo"};
      }

      if(m_bools.at(HHBBTT::is17)){
        ditau_paths_4J12 = {"trigPassed_HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1TAU20IM_2TAU12IM_4J12"};
      }

      if(m_bools.at(HHBBTT::is17PeriodB1_B4)){// For Period B1 to B4 in 2017, should use this trigger but go to L1Topo selection
        ditau_paths_L1Topo = {"trigPassed_HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo"};
      }
      else if(!m_bools.at(HHBBTT::l1topo_disabled) &&
	      (m_bools.at(HHBBTT::is17PeriodB5_B7) || m_bools.at(HHBBTT::is17PeriodB8_end))){
        ditau_paths_L1Topo = {"trigPassed_HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1DR_TAU20ITAU12I_J25"};
      }
    }
    else if(m_bools.at(HHBBTT::is18)){
      ditau_paths_L1Topo = {"trigPassed_HLT_tau35_medium1_tracktwoEF_tau25_medium1_tracktwoEF_L1DR_TAU20ITAU12I_J25"};
      ditau_paths_4J12 = {"trigPassed_HLT_tau35_medium1_tracktwoEF_tau25_medium1_tracktwoEF_L1TAU20IM_2TAU12IM_4J12p0ETA23"};
      if(m_bools.at(HHBBTT::is18PeriodK_end)){
        ditau_paths_L1Topo.push_back("trigPassed_HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_L1DR_TAU20ITAU12I_J25");
        ditau_paths_4J12.push_back("trigPassed_HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_L1TAU20IM_2TAU12IM_4J12p0ETA23");
      }
    }
    else if(m_bools.at(HHBBTT::is22) || m_bools.at(HHBBTT::is23)){
      ditau_paths_L1Topo = {"trigPassed_HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1DR_TAU20ITAU12I_J25"};
      ditau_paths_4J12 = {"trigPassed_HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25"};
    }

    for(const auto& path : ditau_paths_2016){
      // Drop trigPassed_ prefix
      std::string trig = path.substr(11,path.size());
      m_bools.at(HHBBTT::pass_trigger_DTT_2016) |=
	(m_triggerdecos.at(path).get(*event, sys) &&
	 (!m_useDiTauTrigMatch || (tau0 && tau1 && m_matchingTool->match({tau0, tau1}, trig, 0.2))));
      if(m_bools.at(HHBBTT::pass_trigger_DTT_2016)) break;
    }

    if(!tau0 || !tau1 || !jet0) m_bools.at(HHBBTT::pass_trigger_DTT_2016) = false;
    if(m_bools.at(HHBBTT::pass_trigger_DTT_2016))
      m_bools.at(HHBBTT::pass_trigger_DTT_2016) &=
	(tau0->pt() > m_pt_threshold[HHBBTT::DTT][HHBBTT::leadingtau] &&
	 tau1->pt() > m_pt_threshold[HHBBTT::DTT][HHBBTT::subleadingtau] &&
	 jet0->pt() > m_pt_threshold[HHBBTT::DTT_2016][HHBBTT::leadingjet]);

    for(const auto& path : ditau_paths_4J12){
      // Drop trigPassed_ prefix
      std::string trig = path.substr(11,path.size());
      // Naming altered for matching
      trig = std::regex_replace(trig, std::regex("4J12p0ETA"), "4J12_0ETA");
      m_bools.at(HHBBTT::pass_trigger_DTT_4J12) |=
	(m_triggerdecos.at(path).get(*event, sys) &&
	 (!m_useDiTauTrigMatch || (tau0 && tau1 && m_matchingTool->match({tau0, tau1}, trig, 0.2))));
      if(m_bools.at(HHBBTT::pass_trigger_DTT_4J12)) break;
    }

    if(!tau0 || !tau1 || !jet0 || !jet1) m_bools.at(HHBBTT::pass_trigger_DTT_4J12) = false;
    if(m_bools.at(HHBBTT::pass_trigger_DTT_4J12))
      m_bools.at(HHBBTT::pass_trigger_DTT_4J12) &=
	(tau0->pt() > m_pt_threshold[HHBBTT::DTT][HHBBTT::leadingtau] &&
	 tau1->pt() > m_pt_threshold[HHBBTT::DTT][HHBBTT::subleadingtau] &&
	 jet0->pt() > m_pt_threshold[HHBBTT::DTT_4J12][HHBBTT::leadingjet] &&
	 jet1->pt() > m_pt_threshold[HHBBTT::DTT_4J12][HHBBTT::subleadingjet]);

    for(const auto& path : ditau_paths_L1Topo){
      // Drop trigPassed_ prefix
      std::string trig = path.substr(11,path.size());
      // Naming altered for matching
      trig = std::regex_replace(trig, std::regex("L1DR_TAU20ITAU12I_J25"),
				"L1DR-TAU20ITAU12I-J25");
      m_bools.at(HHBBTT::pass_trigger_DTT_L1Topo) |=
	(m_triggerdecos.at(path).get(*event, sys) &&
	 (!m_useDiTauTrigMatch || (tau0 && tau1 && m_matchingTool->match({tau0, tau1}, trig, 0.2))));
      if(m_bools.at(HHBBTT::pass_trigger_DTT_L1Topo)) break;
    }

    if(!tau0 || !tau1 || !jet0) m_bools.at(HHBBTT::pass_trigger_DTT_L1Topo) = false;
    if(m_bools.at(HHBBTT::pass_trigger_DTT_L1Topo))
      m_bools.at(HHBBTT::pass_trigger_DTT_L1Topo) &=
	(tau0->pt() > m_pt_threshold[HHBBTT::DTT][HHBBTT::leadingtau] &&
	 tau1->pt() > m_pt_threshold[HHBBTT::DTT][HHBBTT::subleadingtau] &&
	 tau0->p4().DeltaR(tau1->p4())<2.5 &&
	 jet0->pt() > m_pt_threshold[HHBBTT::DTT_L1Topo][HHBBTT::leadingjet]);

    m_bools.at(HHBBTT::pass_trigger_DTT) =
      (m_bools.at(HHBBTT::pass_trigger_DTT_2016) ||
       m_bools.at(HHBBTT::pass_trigger_DTT_4J12) ||
       m_bools.at(HHBBTT::pass_trigger_DTT_L1Topo));
  }

  void HHbbttSelectorAlg::setRunNumberQuantities(unsigned int rdmNumber){
    // References:
    // https://atlas-tagservices.cern.ch/tagservices/RunBrowser/runBrowserReport/rBR_Period_Report.php
    // https://twiki.cern.ch/twiki/bin/view/Atlas/LowestUnprescaled

    m_bools.at(HHBBTT::is15) = 266904 <= rdmNumber && rdmNumber <= 284484;
    m_bools.at(HHBBTT::is16) = 296939 <= rdmNumber && rdmNumber <= 311481;
    m_bools.at(HHBBTT::is16PeriodA) = 296939 <= rdmNumber && rdmNumber <= 300287;
    m_bools.at(HHBBTT::is16PeriodB_D3) = 300345 <= rdmNumber && rdmNumber <= 302872;
    m_bools.at(HHBBTT::is16PeriodD4_end) = 302919 <= rdmNumber && rdmNumber <= 311481;
    m_bools.at(HHBBTT::is17PeriodB1_B4) = 325713 <= rdmNumber && rdmNumber <= 326695;
    m_bools.at(HHBBTT::is17PeriodB5_B7) = 326834 <= rdmNumber && rdmNumber <= 327490;
    m_bools.at(HHBBTT::is17PeriodB8_end) = 327582 <= rdmNumber && rdmNumber <= 341649;
    m_bools.at(HHBBTT::is18PeriodB_end) = 348885 <= rdmNumber && rdmNumber <= 364485;
    m_bools.at(HHBBTT::is18PeriodK_end) = 355529 <= rdmNumber && rdmNumber <= 364485;

    // Runs in which the L1Topo was mistakingly disabled
    m_bools.at(HHBBTT::l1topo_disabled) = (rdmNumber == 336506) || (rdmNumber == 336548) || (rdmNumber == 336567);

    // Single-lepton triggers
    m_pt_threshold[HHBBTT::SLT][HHBBTT::ele] = (m_bools.at(HHBBTT::is15) ? 25. : 27.) * Athena::Units::GeV;
    m_pt_threshold[HHBBTT::SLT][HHBBTT::mu] = (m_bools.at(HHBBTT::is15) ? 21. : 27.) * Athena::Units::GeV;

    // Single tau triggers
    float min_tau_STT = 180. * Athena::Units::GeV;
    if(m_bools.at(HHBBTT::is15) || m_bools.at(HHBBTT::is16PeriodA)) min_tau_STT = 100. * Athena::Units::GeV;
    else if(m_bools.at(HHBBTT::is16PeriodB_D3)) min_tau_STT = 140. * Athena::Units::GeV;
    m_pt_threshold[HHBBTT::STT][HHBBTT::leadingtau] = min_tau_STT;
  }

}
