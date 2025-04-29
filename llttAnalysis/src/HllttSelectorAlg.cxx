/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Weiming Yao

#include "HllttSelectorAlg.h"

#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>
#include <AthContainers/ConstDataVector.h>
#include <AthenaKernel/Units.h>

namespace HLLTT
{
  HllttSelectorAlg ::HllttSelectorAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
    : EL::AnaAlgorithm(name, pSvcLocator)
  {

  }

  StatusCode HllttSelectorAlg ::initialize()
  {

    // Initialise global event filter
    ATH_CHECK (m_filterParams.initialize(m_systematicsList));

    // Read syst-aware input handles
    ATH_CHECK (m_tauHandle.initialize(m_systematicsList));
    ATH_CHECK (m_mrmtauHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    ATH_CHECK(m_year.initialize(m_systematicsList, m_eventHandle));
    
    // Intialise syst-aware output decorators
    for (const std::string &var : m_Bvarnames){
      CP::SysWriteDecorHandle<bool> whandle{var+"_%SYS%", this};
      m_Bbranches.emplace(var, whandle);
      ATH_CHECK(m_Bbranches.at(var).initialize(m_systematicsList, m_eventHandle));
    };

    m_tauWPDecorHandle = CP::SysReadDecorHandle<char>
      ("baselineSelection_" + m_tauWPName+"_%SYS%", this);
    m_eleWPDecorHandle = CP::SysReadDecorHandle<char>
      ("baselineSelection_" + m_eleWPName+"_%SYS%", this);
    m_muonWPDecorHandle = CP::SysReadDecorHandle<char>
      ("baselineSelection_"+m_muonWPName+"_%SYS%", this);

    if(m_tauWPName=="RNNLoose_eleid") m_tauIDWP = xAOD::TauJetParameters::JetRNNSigLoose;
    else if(m_tauWPName=="RNNMedium_eleid") m_tauIDWP = xAOD::TauJetParameters::JetRNNSigMedium;
    else if(m_tauWPName=="RNNTight_eleid") m_tauIDWP = xAOD::TauJetParameters::JetRNNSigTight;
    else if(m_tauWPName=="Baseline_eleid") m_tauIDWP = xAOD::TauJetParameters::EleRNNLoose;
    else if(m_tauWPName=="RNNVeryLoose_eleid") m_tauIDWP = xAOD::TauJetParameters::JetRNNSigVeryLoose;
    else{
      ATH_MSG_ERROR("Unknown Tau ID WP ");
      return StatusCode::FAILURE;
    }

    ATH_CHECK(m_tauWPDecorHandle.initialize(m_systematicsList, m_tauHandle));
    ATH_CHECK(m_eleWPDecorHandle.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK(m_muonWPDecorHandle.initialize(m_systematicsList, m_muonHandle));
    
    ATH_CHECK(m_selected_el.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK(m_selected_mu.initialize(m_systematicsList, m_muonHandle));
    ATH_CHECK(m_selected_tau.initialize(m_systematicsList, m_tauHandle));
    ATH_CHECK(m_istauID.initialize(m_systematicsList, m_tauHandle));

    // make trigger decorators
    for (auto trig : m_triggers){
      CP::SysReadDecorHandle<bool> deco {this, "trig"+trig, trig, "Name of trigger"};
      m_triggerdecos.emplace(trig, deco);
      ATH_CHECK(m_triggerdecos.at(trig).initialize(m_systematicsList, m_eventHandle));
    }

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());    
    for ( auto name : m_channel_names){
      if( name == "LepLep") m_channels.push_back(HLLTT::LepLep);
      else if( name == "LepHad") m_channels.push_back(HLLTT::LepHad);
      else if ( name == "HadHad") m_channels.push_back(HLLTT::HadHad);
      else{
        ATH_MSG_ERROR("Unknown channel");
        return StatusCode::FAILURE;
      }
    }

    return StatusCode::SUCCESS;
  }

  StatusCode HllttSelectorAlg ::execute()
  {
    // Global filter originally false
    CP::SysFilterReporterCombiner filterCombiner (m_filterParams, false);

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector()){

      CP::SysFilterReporter filter (filterCombiner, sys);

      // Retrive inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      const xAOD::TauJetContainer *taus = nullptr;
      ANA_CHECK (m_tauHandle.retrieve (taus, sys));

      const xAOD::TauJetContainer *mrmtaus = nullptr;
      ANA_CHECK (m_mrmtauHandle.retrieve (mrmtaus, sys));      

      applyTriggerSelection(event, sys);
      // flags for leplep                                              
      m_Bbranches.at("pass_trigger_SLT").set(*event, trigPassed_SLT, sys);
      m_Bbranches.at("pass_trigger_DLT").set(*event, trigPassed_DLT, sys);
      // flags for leplep
      bool N_LEPTONS_CUT_LEPLEP = false;
      bool pass_baseline_LepLep = false;
      bool pass_LepLep = false;
      // flags for lephad
      bool N_LEPTONS_CUT_LEPHAD = false;
      bool pass_baseline_LepHad = false;
      bool pass_LepHad = false;
      // flags for hadhad
      bool N_LEPTONS_CUT_HADHAD = false;
      bool pass_baseline_HadHad = false;
      bool pass_HadHad = false;

      bool pass_baseline_DLT = false;
      bool pass_DLT = false;

      //************
      // lepton
      //************
      int n_looseele = 0;
      int n_loosemuo = 0;
      bool lep_ptcut_DLT = false;
      int n_lep = 0;
      TLorentzVector p4lep[4]; 

      for (const xAOD::Muon *muon : *muons)
      {
        bool passMuonWP = m_muonWPDecorHandle.get(*muon, sys);
        m_selected_mu.set(*muon, false, sys);
        if (passMuonWP && std::abs(muon->eta()) < 2.5
	    && muon->pt() > m_pt_threshold[HLLTT::DLT][HLLTT::subleadinglep])
        {
          if (muon->pt() >  m_pt_threshold[HLLTT::DLT][HLLTT::leadinglep])
            lep_ptcut_DLT = true;
          m_selected_mu.set(*muon, true, sys);
	  if(n_lep<4){ 
	    p4lep[n_lep] = muon->p4(); 
	    ++n_lep; 
	  }
        }
        else
          n_loosemuo += 1;
      }

      for (const xAOD::Electron *electron : *electrons)
	{
	  bool passElectronWP = m_eleWPDecorHandle.get(*electron, sys);
	  m_selected_el.set(*electron, false, sys);
	  if (passElectronWP && electron->pt() > m_pt_threshold[HLLTT::DLT][HLLTT::subleadinglep])
	    {
	      if (electron->pt() > m_pt_threshold[HLLTT::DLT][HLLTT::leadinglep])
		lep_ptcut_DLT = true;
	      m_selected_el.set(*electron, true, sys);
	      if(n_lep<4){
		p4lep[n_lep] = electron->p4();
		++n_lep;
	      }
	    }
	  else
	    n_looseele += 1;
	}

      // try to save both eetautau, mmtautau 
      if (n_lep == 4)
	N_LEPTONS_CUT_LEPLEP = true;
      else if (n_lep == 3)
	N_LEPTONS_CUT_LEPHAD = true;	      
      else if (n_lep == 2)
	N_LEPTONS_CUT_HADHAD = true;

      //************
      // taujet
      //************
      int n_mrmtaus = 0;
      int n_mrmtausnocut = 0;
      TLorentzVector p4mrm;
      for(const xAOD::TauJet* mrmtau : *mrmtaus) {
	bool passTauWP = m_doAntiTau?mrmtau->discriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans)>0.01:mrmtau->isTau(m_tauIDWP);
	bool passTaueleid = mrmtau->isTau(xAOD::TauJetParameters::EleRNNLoose);
	++n_mrmtausnocut;
	if (passTauWP && passTaueleid && mrmtau->pt() > 20. * Athena::Units::GeV){
	  if (std::abs(mrmtau->eta()) < 2.5&&(std::abs(mrmtau->eta()) <1.37||std::abs(mrmtau->eta()) >1.52)&&(mrmtau->nTracksCharged()==1||mrmtau->nTracksCharged()==3)){
	    p4mrm=mrmtau->p4();
	    ++n_mrmtaus;
	  }
	  ATH_MSG_DEBUG("Dump MuonRM TauJets: event "<<event->eventNumber()<<" n_mrmtausnocut "<<n_mrmtausnocut<<" n_mrmtaus "<<n_mrmtaus<<" pt "<<mrmtau->pt()
			<<" eta "<<mrmtau->eta()<<" phi "<<mrmtau->phi()<<" passTauRNNVeryLoose "<<passTauWP<<" passTaueleid "<<passTaueleid<<" RNN "
			<<mrmtau->discriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans)<<" ntrk "<<mrmtau->nTracksCharged()<<" taus size "<<taus->size());
	}
      }
      int n_taus = 0;
      for (const xAOD::TauJet *tau : *taus)
      {
        bool passTauWP = m_tauWPDecorHandle.get(*tau, sys);
	bool passTauWPloose = m_doAntiTau?tau->discriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans)>0.01:passTauWP;
        m_selected_tau.set(*tau, false, sys);
	m_istauID.set(*tau, 0, sys);
        if (passTauWPloose&& tau->pt() > 20. * Athena::Units::GeV)
        {
	  //MRMTaus are preferred and other taus ignored
	  if((n_mrmtaus==1&&p4mrm.DeltaR(tau->p4())<0.001)||n_mrmtaus!=1){
	    m_selected_tau.set(*tau, true, sys);
	    int tauid=0;
	    if(tau->isTau(xAOD::TauJetParameters::JetRNNSigLoose))tauid = 1;
	    if(tau->isTau(xAOD::TauJetParameters::JetRNNSigMedium))tauid = 2;
	    m_istauID.set(*tau, tauid, sys);
	    n_taus += 1;
	    ATH_MSG_DEBUG(" Dump combined TauJets after selection: event "<<event->eventNumber()<<" n_taus "<<n_taus<<" pt "<<tau->pt()<<" eta "<<tau->eta()<<" phi "<<tau->phi()
			  <<" passTauWP "<<passTauWP<<" RNN "<<tau->discriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans)<<" tauid "<<tauid);
	  }
        }
      }

      if (N_LEPTONS_CUT_LEPLEP){
        // DLT
        if (lep_ptcut_DLT){
	  pass_baseline_LepLep = true;
	  if (trigPassed_DLT)
	    pass_LepLep = true;
        }
      }

      if (N_LEPTONS_CUT_LEPHAD && n_taus>=1){
        // DLT
        if (lep_ptcut_DLT){
           pass_baseline_LepHad = true;
           if (trigPassed_DLT)
              pass_LepHad = true;
        }
      }

      if (N_LEPTONS_CUT_HADHAD && n_taus >=2 ){
        // DLT
	if (lep_ptcut_DLT){
	  pass_baseline_HadHad = true;
	  if (trigPassed_DLT)
	    pass_HadHad = true;
        }
      } 

      pass_baseline_DLT = false;
      pass_DLT = false;
 
      for(const auto& channel : m_channels){
	if(channel == HLLTT::LepLep) {
	  pass_baseline_DLT |= (pass_baseline_LepLep);
	  pass_DLT |= (pass_LepLep);
	}
	else if(channel == HLLTT::LepHad){
	  pass_baseline_DLT |= (pass_baseline_LepHad);
	  pass_DLT |= (pass_LepHad);
	} 
	else if(channel == HLLTT::HadHad){
	  pass_baseline_DLT |= (pass_baseline_HadHad);
	  pass_DLT |= (pass_HadHad);
	}
      }
      m_Bbranches.at("pass_baseline_DLT").set(*event, pass_baseline_DLT, sys);
      m_Bbranches.at("pass_DLT").set(*event, pass_DLT, sys);
      m_Bbranches.at("pass_baseline_LepLep").set(*event, pass_baseline_LepLep, sys);
      m_Bbranches.at("pass_LepLep").set(*event, pass_LepLep, sys);
      m_Bbranches.at("pass_baseline_LepHad").set(*event, pass_baseline_LepHad, sys);
      m_Bbranches.at("pass_LepHad").set(*event, pass_LepHad, sys);
      m_Bbranches.at("pass_baseline_HadHad").set(*event, pass_baseline_HadHad, sys);
      m_Bbranches.at("pass_HadHad").set(*event, pass_HadHad, sys);
      m_Bbranches.at("pass_Looseele").set(*event, n_looseele==0, sys);
      m_Bbranches.at("pass_Loosemuo").set(*event, n_loosemuo==0, sys);
      m_Bbranches.at("pass_mrmtaus").set(*event, n_mrmtaus>0, sys);

      if (!m_bypass && !pass_baseline_DLT) continue;

      // Global event filter true if any syst passes and controls
      // if event is passed to output writing or not
      filter.setPassed(true);
    }

    return StatusCode::SUCCESS;
  }

  StatusCode HllttSelectorAlg::finalize() {
    ANA_CHECK (m_filterParams.finalize ());
    return StatusCode::SUCCESS;
  }

  void HllttSelectorAlg::applyTriggerSelection(const xAOD::EventInfo* event, const CP::SystematicSet& sys){
    //************
    // trigger selecton
    //************

    trigPassed_SLT = false;
    trigPassed_DLT = false;
    applySLTTriggerSelection(event, sys);
    applyDiLepTriggerSelection(event, sys);
  }


  void HllttSelectorAlg ::applySLTTriggerSelection(const xAOD::EventInfo *event,
						   const CP::SystematicSet &sys)
  {
    bool trigPassed_SET = false;
    bool trigPassed_SMT = false;
    trigPassed_SLT = false;

    std::vector<std::string> single_ele_paths;
    std::vector<std::string> single_mu_paths;

    int year = m_year.get(*event, sys);

    // SLT
    if(year==2015){
      m_pt_threshold[HLLTT::SLT][HLLTT::ele] = 25. * Athena::Units::GeV;
      m_pt_threshold[HLLTT::SLT][HLLTT::mu] = 21. * Athena::Units::GeV;
      single_ele_paths = {"trigPassed_HLT_e24_lhmedium_L1EM20VH", "trigPassed_HLT_e60_lhmedium", "trigPassed_HLT_e120_lhloose"};
      single_mu_paths = {"trigPassed_HLT_mu20_iloose_L1MU15", "trigPassed_HLT_mu50"};
    }
    else if(2016<=year && year<=2018){
      m_pt_threshold[HLLTT::SLT][HLLTT::ele] = 27. * Athena::Units::GeV;
      m_pt_threshold[HLLTT::SLT][HLLTT::mu] = 27. * Athena::Units::GeV;
      single_ele_paths = {"trigPassed_HLT_e26_lhtight_nod0_ivarloose", "trigPassed_HLT_e60_lhmedium_nod0", "trigPassed_HLT_e140_lhloose_nod0"};
      single_mu_paths = {"trigPassed_HLT_mu26_ivarmedium", "trigPassed_HLT_mu50"};
    }
    else if(year==2022) {
      m_pt_threshold[HLLTT::SLT][HLLTT::ele] = 27. * Athena::Units::GeV;
      m_pt_threshold[HLLTT::SLT][HLLTT::mu] = 27. * Athena::Units::GeV;
      single_ele_paths = {"trigPassed_HLT_e26_lhtight_ivarloose_L1EM22VHI", "trigPassed_HLT_e60_lhmedium_L1EM22VHI", "trigPassed_HLT_e140_lhloose_L1EM22VHI"
      };
      single_mu_paths = {"trigPassed_HLT_mu24_ivarmedium_L1MU14FCH", "trigPassed_HLT_mu50_L1MU14FCH"};
    }
    else if(year==2023) {
      m_pt_threshold[HLLTT::SLT][HLLTT::ele] = 27. * Athena::Units::GeV;
      m_pt_threshold[HLLTT::SLT][HLLTT::mu] = 27. * Athena::Units::GeV;
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
  }

  void HllttSelectorAlg ::applyDiLepTriggerSelection(
      const xAOD::EventInfo *event,
      const CP::SystematicSet &sys)
  {
    trigPassed_DLT = false;

    std::vector<std::string> dilep_paths;
    int year = m_year.get(*event, sys);
    
    // SLT
    m_pt_threshold[HLLTT::DLT][HLLTT::leadinglep] = 20. * Athena::Units::GeV;
    m_pt_threshold[HLLTT::DLT][HLLTT::subleadinglep] = 7. * Athena::Units::GeV;
    if(year==2015){
      dilep_paths = {"trigPassed_HLT_2e12_lhvloose_L12EM10VH","trigPassed_HLT_mu18_mu8noL1","trigPassed_HLT_e17_lhloose_mu14"};
    }
    else if(year==2016){
      dilep_paths = {"trigPassed_HLT_2e17_lhvloose_nod0","trigPassed_HLT_mu22_mu8noL1","trigPassed_HLT_e17_lhloose_nod0_mu14"};
    }
    else if(2017<=year && year<=2018){
      dilep_paths = {"trigPassed_HLT_2e17_lhvloose_nod0_L12EM15VHI","trigPassed_HLT_mu22_mu8noL1","trigPassed_HLT_e17_lhloose_nod0_mu14"};
    }
    else if(year==2022){
      dilep_paths = {"trigPassed_HLT_2e17_lhvloose_L12EM15VHI","trigPassed_HLT_mu22_mu8noL1_L1MU14FCH","trigPassed_HLT_e17_lhloose_mu14_L1EM15VH_MU8F"};
    }
    else if(year==2023){
      dilep_paths = {"trigPassed_HLT_2e17_lhvloose_L12eEM18M","trigPassed_HLT_mu22_mu8noL1_L1MU14FCH","trigPassed_HLT_e17_lhloose_mu14_L1EM15VH_MU8F"};
    }

    // Pass single electron trigger
    for(const auto& path : dilep_paths){
      trigPassed_DLT |= m_triggerdecos.at(path).get(*event, sys);
      if(trigPassed_DLT) break;
    }
  }
}
