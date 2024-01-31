/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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

namespace HHBBTT
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

    if(m_isMC){
      m_ele_SF = CP::SysReadDecorHandle<float>("el_effSF_"+m_eleWPName+"_%SYS%", this);
    }
    ATH_CHECK (m_ele_SF.initialize(m_systematicsList, m_electronHandle, SG::AllowEmpty));

    if(m_isMC){
      m_mu_SF = CP::SysReadDecorHandle<float>("muon_effSF_"+m_muWPName+"_%SYS%", this);
    }
    ATH_CHECK (m_mu_SF.initialize(m_systematicsList, m_muonHandle, SG::AllowEmpty));

    if(m_isMC){
      m_tau_effSF = CP::SysReadDecorHandle<float>("tau_effSF_"+m_tauWPName+"_%SYS%", this);
    }
    ATH_CHECK (m_tau_effSF.initialize(m_systematicsList, m_tauHandle, SG::AllowEmpty));

    ATH_CHECK (m_selected_el.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK (m_selected_mu.initialize(m_systematicsList, m_muonHandle));
    ATH_CHECK (m_selected_tau.initialize(m_systematicsList, m_tauHandle));

    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }


    for (const std::string &var : m_floatVariables){
      ATH_MSG_DEBUG("initializing float variable: " << var);
      CP::SysWriteDecorHandle<float> whandle{var+"_%SYS%", this};
      m_Fbranches.emplace(var, whandle);
      ATH_CHECK(m_Fbranches.at(var).initialize(m_systematicsList, m_eventHandle));
    };

    for (const std::string &var : m_intVariables){
      ATH_MSG_DEBUG("initializing integer variable: " << var);
      CP::SysWriteDecorHandle<int> whandle{var+"_%SYS%", this};
      m_Ibranches.emplace(var, whandle);
      ATH_CHECK(m_Ibranches.at(var).initialize(m_systematicsList, m_eventHandle));
    };

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

      for (const auto& var: m_floatVariables) {
        m_Fbranches.at(var).set(*event, -99, sys);
      }

      for (const auto& var: m_intVariables) {
        m_Ibranches.at(var).set(*event, -99, sys);
      }

      // selected leptons ; 
      TLorentzVector lepton(0,0,0,0);
      int lepton_charge = -99;
      int lepton_pdgid = -99;
      float lepton_SF = 1.;
      bool found_lepton = false;

      for(const xAOD::Electron* electron : *electrons) {
        if (m_selected_el.get(*electron, sys)){
          lepton = electron->p4();
          lepton_charge = electron->charge();
          lepton_pdgid = electron->charge() > 0 ? -11 : 11;
          if(m_isMC) lepton_SF = m_ele_SF.get(*electron,sys);
          found_lepton = true;
          break; // At most one lepton selected
      	}
      }
      for(const xAOD::Muon* muon : *muons) {
        if(found_lepton) break;
        if (m_selected_mu.get(*muon, sys)){
          lepton = muon->p4();
          lepton_charge = muon->charge();
          lepton_pdgid = muon->charge() > 0 ? -13 : 13;
          if(m_isMC) lepton_SF = m_mu_SF.get(*muon,sys);
          found_lepton = true;
          break; 
        }
      }

      if(found_lepton){
        m_Fbranches.at("Lepton_pt").set(*event, lepton.Pt(), sys);
        m_Fbranches.at("Lepton_eta").set(*event, lepton.Eta(), sys);
        m_Fbranches.at("Lepton_phi").set(*event, lepton.Phi(), sys);
        m_Fbranches.at("Lepton_E").set(*event, lepton.E(), sys);
        if(m_isMC) m_Fbranches.at("Lepton_effSF").set(*event, lepton_SF, sys);
        m_Ibranches.at("Lepton_charge").set(*event, lepton_charge, sys);
        m_Ibranches.at("Lepton_pdgid").set(*event, lepton_pdgid, sys);
      }

      //selected tau
      TLorentzVector lead_tau(0,0,0,0);
      TLorentzVector sublead_tau(0,0,0,0);
      int lead_tau_charge = -99;
      int sublead_tau_charge = -99;
      float lead_tau_effSF = 1;
      float sublead_tau_effSF = 1;
      bool found_lead_tau = false;
      bool found_sublead_tau = false;


      for(const xAOD::TauJet* tau : *taus) {
        if (m_selected_tau.get(*tau, sys)){
          if(!found_lead_tau){
            lead_tau = tau->p4();
            lead_tau_charge = tau->charge();
            if(m_isMC) lead_tau_effSF = m_tau_effSF.get(*tau,sys);
            found_lead_tau = true;
            continue;
          }

          sublead_tau = tau->p4();
          sublead_tau_charge = tau->charge();
          if(m_isMC) sublead_tau_effSF = m_tau_effSF.get(*tau,sys);
          found_sublead_tau = true;
          break; 
        }
      }

      if(found_lead_tau){
        m_Fbranches.at("Tau1_pt").set(*event, lead_tau.Pt(), sys);
        m_Fbranches.at("Tau1_eta").set(*event, lead_tau.Eta(), sys);
        m_Fbranches.at("Tau1_phi").set(*event, lead_tau.Phi(), sys);
        m_Fbranches.at("Tau1_E").set(*event, lead_tau.E(), sys);
        if(m_isMC) m_Fbranches.at("Tau1_effSF").set(*event, lead_tau_effSF, sys);
        m_Ibranches.at("Tau1_charge").set(*event, lead_tau_charge, sys);
      }

      if(found_sublead_tau){
        m_Fbranches.at("Tau2_pt").set(*event, sublead_tau.Pt(), sys);
        m_Fbranches.at("Tau2_eta").set(*event, sublead_tau.Eta(), sys);
        m_Fbranches.at("Tau2_phi").set(*event, sublead_tau.Phi(), sys);
        m_Fbranches.at("Tau2_E").set(*event, sublead_tau.E(), sys);
        if(m_isMC) m_Fbranches.at("Tau2_effSF").set(*event, sublead_tau_effSF, sys);
        m_Ibranches.at("Tau2_charge").set(*event, sublead_tau_charge, sys);
      }

      // DiHiggs mass 
      TLorentzVector bb(0,0,0,0);
      bool found_bb = false;

      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      for(const xAOD::Jet* jet : *jets) {
        if (WPgiven) {
          if (m_isBtag.get(*jet, sys)) bjets->push_back(jet);
        }
      }
     
      if (bjets->size() > 1){
        TLorentzVector b1 = bjets->at(0)->p4();
        TLorentzVector b2 = bjets->at(1)->p4();
        bb = b1+b2;
        found_bb = true;
        m_Fbranches.at("Jet_b1_pt").set(*event,  b1.Pt(), sys);
        m_Fbranches.at("Jet_b1_eta").set(*event, b1.Eta(), sys);
        m_Fbranches.at("Jet_b1_phi").set(*event, b1.Phi(), sys);
        m_Fbranches.at("Jet_b1_E").set(*event,   b1.E(), sys);
        m_Fbranches.at("Jet_b2_pt").set(*event,  b2.Pt(), sys);
        m_Fbranches.at("Jet_b2_eta").set(*event, b2.Eta(), sys);
        m_Fbranches.at("Jet_b2_phi").set(*event, b2.Phi(), sys);
        m_Fbranches.at("Jet_b2_E").set(*event,   b2.E(), sys);


        m_Fbranches.at("H_bb_pt").set(*event, bb.Pt(), sys);
        m_Fbranches.at("H_bb_eta").set(*event, bb.Eta(), sys);
        m_Fbranches.at("H_bb_phi").set(*event, bb.Phi(), sys);
        m_Fbranches.at("H_bb_m").set(*event,  bb.M(), sys);
      }

      TLorentzVector tautau_vis(0,0,0,0);
      bool found_tautau_vis = false;
      if(found_lead_tau){
        if(found_lepton){
          tautau_vis = lead_tau + lepton;
          found_tautau_vis = true;
        }else if(found_sublead_tau){
          tautau_vis = lead_tau + sublead_tau;
          found_tautau_vis = true;
        }
        m_Fbranches.at("H_vis_tautau_pt").set(*event, tautau_vis.Pt(), sys);
        m_Fbranches.at("H_vis_tautau_eta").set(*event, tautau_vis.Eta(), sys);
        m_Fbranches.at("H_vis_tautau_phi").set(*event, tautau_vis.Phi(), sys);
        m_Fbranches.at("H_vis_tautau_m").set(*event,  tautau_vis.M(), sys);
      }

      if(found_bb && found_tautau_vis){

        TLorentzVector HH_vis = bb+tautau_vis;

        m_Fbranches.at("HH_vis_pt").set(*event, HH_vis.Pt(), sys);
        m_Fbranches.at("HH_vis_eta").set(*event, HH_vis.Eta(), sys);
        m_Fbranches.at("HH_vis_phi").set(*event, HH_vis.Phi(), sys);
        m_Fbranches.at("HH_vis_m").set(*event, HH_vis.M(), sys);

        TLorentzVector mmc_vec(0,0,0,0);
        mmc_vec.SetPtEtaPhiM(m_mmc_pt.get(*event, sys),
			     m_mmc_eta.get(*event, sys),
			     m_mmc_phi.get(*event, sys),
			     m_mmc_m.get(*event, sys));
        TLorentzVector HH = bb+mmc_vec;

        m_Fbranches.at("HH_pt").set(*event, HH.Pt(), sys);
        m_Fbranches.at("HH_eta").set(*event, HH.Eta(), sys);
        m_Fbranches.at("HH_phi").set(*event, HH.Phi(), sys);
        m_Fbranches.at("HH_m").set(*event, HH.M(), sys);

        if(m_storeHighLevelVariables){
          m_Fbranches.at("HH_delta_phi").set(*event, bb.DeltaPhi(mmc_vec),sys);
          m_Fbranches.at("HH_vis_delta_phi").set(*event, bb.DeltaPhi(tautau_vis),sys);
        }
      }
    }

    return StatusCode::SUCCESS;
  }
}
