/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Carl Gwilliam

#include "HHbbttSelectorAlg.h"
#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

//#include <EventBookkeeperTools/FilterReporter.h>

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

    ATH_CHECK (m_mmc_m.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK(m_pass_SLT.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_pass_LTT.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_pass_STT.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_pass_DTT.initialize(m_systematicsList, m_eventHandle));

    m_IDTauDecorKey = m_tauHandle.getNamePattern() + "." + m_IDTauDecorName;
    m_eleIdDecorKey = m_electronHandle.getNamePattern() + "." + m_eleIdDecorName;
    m_muonIdDecorKey = m_muonHandle.getNamePattern() + "." + m_muonIdDecorName;
    m_muonPreselDecorKey = m_muonHandle.getNamePattern() + "." + m_muonPreselDecorName;

    ATH_CHECK (m_IDTauDecorKey.initialize());
    ATH_CHECK (m_eleIdDecorKey.initialize());
    ATH_CHECK (m_muonIdDecorKey.initialize());
    ATH_CHECK (m_muonPreselDecorKey.initialize());

    ATH_CHECK(m_selected_el.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK(m_selected_mu.initialize(m_systematicsList, m_muonHandle));
    ATH_CHECK(m_selected_tau.initialize(m_systematicsList, m_tauHandle));

    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    // Intialise syst-aware output decorators
    ATH_CHECK(m_pass_sr.initialize(m_systematicsList, m_eventHandle));

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

    return StatusCode::SUCCESS;
  }

  StatusCode HHbbttSelectorAlg ::execute()
  {

    // Global filter originally false
    CP::SysFilterReporterCombiner filterCombiner (m_filterParams, false);

    SG::ReadDecorHandle<xAOD::TauJetContainer, char> idTauDecorHandle(m_IDTauDecorKey);
    SG::ReadDecorHandle<xAOD::ElectronContainer, char> eleIdDecorHandle(m_eleIdDecorKey);
    SG::ReadDecorHandle<xAOD::MuonContainer, char> muonIdDecorHandle(m_muonIdDecorKey);
    SG::ReadDecorHandle<xAOD::MuonContainer, char> muonPreselDecorHandle(m_muonPreselDecorKey);

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
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

      // flags
      TWO_JETS = false;
      TWO_BJETS = false;
      LEADJET_PT = false;
      MMC_MASS = false;
      MBB_MASS = false;
      // flags for lephad
      N_LEPTONS_CUT_LEPHAD = false;
      ONE_TAU = false;
      OS_CHARGE_LEPHAD = false;
      pass_SLT = false;
      pass_LTT = false;
      // flags for hadhad
      N_LEPTONS_CUT_HADHAD = false;
      TWO_TAU = false;
      OS_CHARGE_HADHAD = false;
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
        bool passElectronTight = 0;
        passElectronTight = eleIdDecorHandle(*electron);
        m_selected_el.set(*electron, false, sys);
        if (passElectronTight && electron->pt() > 18000)
        {
          if (electron->pt() > 18000 && electron->pt() < 25000)
            lep_ptcut_LTT = true;
          else if (electron->pt() > 25000)
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
        bool passMuonMedium = 0;
        passMuonMedium =
            muonIdDecorHandle(*muon) && muonPreselDecorHandle(*muon);
        m_selected_mu.set(*muon, false, sys);
        if (passMuonMedium && abs(muon->eta()) < 2.5 && muon->pt() > 15000)
        {
          if (muon->pt() > 15000 && muon->pt() < 21000)
            lep_ptcut_LTT = true;
          else if (muon->pt() > 21000)
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
        bool isTauID = idTauDecorHandle(*tau);
        m_selected_tau.set(*tau, false, sys);
        if (isTauID && tau->pt() > 20000)
        {
          if (abs(tau->eta()) < 2.3) {
            if (tau->pt() > 20000)
              tau_ptcut_SLT = true;
            if (tau->pt() > 30000)
              tau_ptcut_LTT = true;
          }
          if (tau->pt() > 180000)
            tau_ptcut_STT_lead = true;
          if (tau->pt() > 25000)
            tau_ptcut_STT_sublead++;
          if (tau->pt() > 40000)
            tau_ptcut_DTT_lead = true;
          if (tau->pt() > 30000)
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
        if (jets->at(0)->pt() > 45000)
          LEADJET_PT = true;
        if (bjets->size() == 2)
        {
          TWO_BJETS = true;
          bb = bjets->at(0)->p4() + bjets->at(1)->p4();
          mbb = bb.M();
        }
      }

      //****************
      // event level info
      //****************
      if (m_mmc_m.get(*event, sys) > 60000)
        MMC_MASS = true;
      if (mbb < 150000)
        MBB_MASS = true;
      if (charge_tau0 != charge_lepton)
        OS_CHARGE_LEPHAD = true;
      if (charge_tau0 == - charge_tau1)
        OS_CHARGE_HADHAD = true;

      // SLT
      if (N_LEPTONS_CUT_LEPHAD && lep_ptcut_SLT && ONE_TAU && tau_ptcut_SLT &&
          TWO_JETS && TWO_BJETS && LEADJET_PT && MMC_MASS && MBB_MASS &&
          OS_CHARGE_LEPHAD)
        pass_SLT = true;

      // LTT
      if (N_LEPTONS_CUT_LEPHAD && lep_ptcut_LTT && ONE_TAU && tau_ptcut_LTT &&
          TWO_JETS && TWO_BJETS && LEADJET_PT && MMC_MASS && MBB_MASS &&
          OS_CHARGE_LEPHAD)
        pass_LTT = true;

      // STT
      if (N_LEPTONS_CUT_HADHAD && TWO_TAU && tau_ptcut_STT &&
          TWO_JETS && TWO_BJETS && LEADJET_PT && MMC_MASS &&
          OS_CHARGE_HADHAD)
        pass_STT = true;
      // DTT
      if (N_LEPTONS_CUT_HADHAD && TWO_TAU && tau_ptcut_DTT &&
          TWO_JETS && TWO_BJETS && LEADJET_PT && MMC_MASS &&
          OS_CHARGE_HADHAD)
        pass_DTT = true;

      m_pass_SLT.set(*event, pass_SLT, sys);
      m_pass_LTT.set(*event, pass_LTT, sys);
      m_pass_STT.set(*event, pass_STT, sys);
      m_pass_DTT.set(*event, pass_DTT, sys);

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
    //ATH_MSG_INFO(m_filterParams.summary());
    return StatusCode::SUCCESS;
  }

}

