/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


#include "AthContainers/AuxElement.h"
#include "BaselineVarsbbttAlg.h"
#include <FourMomUtils/xAODP4Helpers.h>
#include <AthContainers/ConstDataVector.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODTau/TauJetContainer.h>
#include <AthContainers/ConstDataVector.h>

#include "TLorentzVector.h"

namespace HH4B
{
  BaselineVarsbbttAlg::BaselineVarsbbttAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {

  }

  StatusCode BaselineVarsbbttAlg::initialize()
  {

    // Read syst-aware input handles
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_tauHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    ATH_CHECK (m_mmc_pt.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_mmc_eta.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_mmc_phi.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_mmc_m.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK (m_selected_el.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK (m_selected_mu.initialize(m_systematicsList, m_muonHandle));
    ATH_CHECK (m_selected_tau.initialize(m_systematicsList, m_tauHandle));

    // Intialise syst-aware output decorators
    ATH_CHECK(m_HH_pt.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_HH_eta.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_HH_phi.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_HH_m.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_HH_vis_pt.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_HH_vis_eta.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_HH_vis_phi.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_HH_vis_m.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK(m_selected_lepton_pt.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_selected_lepton_eta.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_selected_lepton_phi.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_selected_lepton_charge.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_selected_lepton_pdgid.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_selected_tau_pt.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_selected_tau_eta.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_selected_tau_phi.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_selected_tau_charge.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());    

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsbbttAlg::execute()
  {

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {

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

      // Calculate vars

      // selected leptons ; 
      float lepton_pt = -99;
      float lepton_eta = -99;
      float lepton_phi = -99;
      int lepton_charge = -99;
      int lepton_pdgid = -99;

      for(const xAOD::Electron* electron : *electrons) {
        if (m_selected_el.get(*electron, sys)){
          lepton_pt = electron->pt();
          lepton_eta = electron->eta();
          lepton_phi = electron->phi();
          lepton_charge = electron->charge();
	  lepton_pdgid = electron->charge() > 0 ? -11 : 11;
          break; // At most one lepton selected
	}
      }
      for(const xAOD::Muon* muon : *muons) {
        if (m_selected_mu.get(*muon, sys)){
          lepton_pt = muon->pt();
          lepton_eta = muon->eta();
          lepton_phi = muon->phi();
          lepton_charge = muon->charge();
	  lepton_pdgid = muon->charge() > 0 ? -13 : 13;
          break; 
	}
      }
      m_selected_lepton_pt.set(*event, lepton_pt, sys);
      m_selected_lepton_eta.set(*event, lepton_eta, sys);
      m_selected_lepton_phi.set(*event, lepton_phi, sys);
      m_selected_lepton_charge.set(*event, lepton_charge, sys);
      m_selected_lepton_pdgid.set(*event, lepton_pdgid, sys);

      //selected tau
      float tau_pt = -99;
      float tau_eta = -99;
      float tau_phi = -99;
      int tau_charge = -99;

      for(const xAOD::TauJet* tau : *taus) {
        if (m_selected_tau.get(*tau, sys)){
          tau_pt = tau->pt();
          tau_eta = tau->eta();
          tau_phi = tau->phi();
          tau_charge = tau->charge();
          break; 
	}
      }
      m_selected_tau_pt.set(*event, tau_pt, sys);
      m_selected_tau_eta.set(*event, tau_eta, sys);
      m_selected_tau_phi.set(*event, tau_phi, sys);
      m_selected_tau_charge.set(*event, tau_charge, sys);

      // DiHiggs mass 
      TLorentzVector bb(0,0,0,0);
      TLorentzVector tautau(0,0,0,0);
      TLorentzVector HH(0,0,0,0);
      TLorentzVector HH_vis(0,0,0,0);
      TLorentzVector mmc_vec(0,0,0,0);

      if (jets->size() > 1) bb = jets->at(0)->p4() + jets->at(1)->p4();
      if (taus->size() > 1) tautau = taus->at(0)->p4() + taus->at(1)->p4();

      HH_vis=bb+tautau;

      mmc_vec.SetPtEtaPhiM(m_mmc_pt.get(*event, sys),
                           m_mmc_eta.get(*event, sys),
                           m_mmc_phi.get(*event, sys),
                           m_mmc_m.get(*event, sys));
      HH=bb+mmc_vec;

      m_HH_pt.set(*event, HH.Pt(), sys);
      m_HH_eta.set(*event, HH.Eta(), sys);
      m_HH_phi.set(*event, HH.Phi(), sys);
      m_HH_m.set(*event, HH.M(), sys);
      m_HH_vis_pt.set(*event, HH_vis.Pt(), sys);
      m_HH_vis_eta.set(*event, HH_vis.Eta(), sys);
      m_HH_vis_phi.set(*event, HH_vis.Phi(), sys);
      m_HH_vis_m.set(*event, HH_vis.M(), sys);
    }

    return StatusCode::SUCCESS;
  }
}
