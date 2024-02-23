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
    if (!m_PCBT.empty()) {
      ATH_CHECK (m_PCBT.initialize(m_systematicsList, m_jetHandle));
    }
    if (m_isMC) {
      ATH_CHECK (m_truthFlav.initialize(m_systematicsList, m_jetHandle));
    }

    // tau ID
    m_IDTauDecorKey = m_tauHandle.getNamePattern() + "." + m_IDTauDecorName;
    ATH_CHECK (m_IDTauDecorKey.initialize());

    m_antiTauDecorKey = m_tauHandle.getNamePattern() + "." + m_antiTauDecorName;
    ATH_CHECK (m_antiTauDecorKey.initialize());

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
    SG::ReadDecorHandle<xAOD::TauJetContainer, char> idTauDecorHandle(m_IDTauDecorKey);
    SG::ReadDecorHandle<xAOD::TauJetContainer, char> antiTauDecorHandle(m_antiTauDecorKey);

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {

      // Retrieve inputs
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
      TLorentzVector lead_lep(0,0,0,0);
      TLorentzVector sublead_lep(0,0,0,0);
      int lead_lep_charge = -99;
      int sublead_lep_charge = -99;
      int lead_lep_pdgid = -99;
      int sublead_lep_pdgid = -99;
      float lead_lep_SF = 1.;
      float sublead_lep_SF = 1.;
      bool found_lead_lep = false;
      bool found_sublead_lep = false;

      for(const xAOD::Electron* electron : *electrons) {
        if (m_selected_el.get(*electron, sys) && !found_lead_lep){
          lead_lep = electron->p4();
          lead_lep_charge = electron->charge();
          lead_lep_pdgid = electron->charge() > 0 ? -11 : 11;
          if(m_isMC) lead_lep_SF = m_ele_SF.get(*electron,sys);
          found_lead_lep = true;
      	}
        else if (m_selected_el.get(*electron, sys) && found_lead_lep){
          sublead_lep = electron->p4();
          sublead_lep_charge = electron->charge();
          sublead_lep_pdgid = electron->charge() > 0 ? -11 : 11;
          if(m_isMC) sublead_lep_SF = m_ele_SF.get(*electron,sys);
          found_sublead_lep = true;
          break; // At most two leptons selected
        }
      }
      for(const xAOD::Muon* muon : *muons) {
        if (m_selected_mu.get(*muon, sys) && !found_lead_lep){
          lead_lep = muon->p4();
          lead_lep_charge = muon->charge();
          lead_lep_pdgid = muon->charge() > 0 ? -13 : 13;
          if(m_isMC) lead_lep_SF = m_mu_SF.get(*muon,sys);
          found_lead_lep = true;
          break; 
        }
        if (m_selected_mu.get(*muon, sys) && found_lead_lep){
          if (found_sublead_lep) {
            if (sublead_lep.Pt() > muon->p4().Pt()) break;
          }
          sublead_lep = muon->p4();
          sublead_lep_charge = muon->charge();
          sublead_lep_pdgid = muon->charge() > 0 ? -13 : 13;
          if(m_isMC) sublead_lep_SF = m_mu_SF.get(*muon,sys);
          found_sublead_lep = true;
          break;
        }
      }
      // the muon might actually be the leading lepton
      if (found_sublead_lep) {
        if (lead_lep.Pt() < sublead_lep.Pt()) {
          TLorentzVector temp(0,0,0,0);
          int temp_charge = -99;
          int temp_pdgid = -99;
          float temp_SF = 1.;

          temp = lead_lep;
          temp_charge = lead_lep_charge;
          temp_pdgid = lead_lep_pdgid;
          if(m_isMC) temp_SF = lead_lep_SF;

          lead_lep = sublead_lep;
          lead_lep_charge = sublead_lep_charge;
          lead_lep_pdgid = sublead_lep_pdgid;
          if(m_isMC) lead_lep_SF = sublead_lep_SF;

          sublead_lep = temp;
          sublead_lep_charge = temp_charge;
          sublead_lep_pdgid = temp_pdgid;
          if(m_isMC) sublead_lep_SF = temp_SF;
        }
      }

      if(found_lead_lep){
        m_Fbranches.at("Lepton1_pt").set(*event, lead_lep.Pt(), sys);
        m_Fbranches.at("Lepton1_eta").set(*event, lead_lep.Eta(), sys);
        m_Fbranches.at("Lepton1_phi").set(*event, lead_lep.Phi(), sys);
        m_Fbranches.at("Lepton1_E").set(*event, lead_lep.E(), sys);
        if(m_isMC) m_Fbranches.at("Lepton1_effSF").set(*event, lead_lep_SF, sys);
        m_Ibranches.at("Lepton1_charge").set(*event, lead_lep_charge, sys);
        m_Ibranches.at("Lepton1_pdgid").set(*event, lead_lep_pdgid, sys);
      }
      if(found_sublead_lep){
        m_Fbranches.at("Lepton2_pt").set(*event, sublead_lep.Pt(), sys);
        m_Fbranches.at("Lepton2_eta").set(*event, sublead_lep.Eta(), sys);
        m_Fbranches.at("Lepton2_phi").set(*event, sublead_lep.Phi(), sys);
        m_Fbranches.at("Lepton2_E").set(*event, sublead_lep.E(), sys);
        if(m_isMC) m_Fbranches.at("Lepton2_effSF").set(*event, sublead_lep_SF, sys);
        m_Ibranches.at("Lepton2_charge").set(*event, sublead_lep_charge, sys);
        m_Ibranches.at("Lepton2_pdgid").set(*event, sublead_lep_pdgid, sys);
      }

      //selected tau
      TLorentzVector lead_tau(0,0,0,0);
      TLorentzVector sublead_tau(0,0,0,0);
      int lead_tau_charge = -99;
      int sublead_tau_charge = -99;
      int lead_tau_nTracks = -99;
      int sublead_tau_nTracks = -99.;
      float lead_tau_effSF = 1;
      float sublead_tau_effSF = 1;
      int lead_tau_isTauID = -99;
      int sublead_tau_isTauID = -99;
      int lead_tau_isAntiTau = -99;
      int sublead_tau_isAntiTau = -99;

      bool found_lead_tau = false;
      bool found_sublead_tau = false;

      for(const xAOD::TauJet* tau : *taus) {
        if (m_selected_tau.get(*tau, sys)){
          if(!found_lead_tau){
            lead_tau = tau->p4();
            lead_tau_charge = tau->charge();
            lead_tau_nTracks = tau->nTracks();
            lead_tau_isTauID = static_cast<int>(idTauDecorHandle(*tau));
            lead_tau_isAntiTau = static_cast<int>(antiTauDecorHandle(*tau));
            if(m_isMC) lead_tau_effSF = m_tau_effSF.get(*tau,sys);
            found_lead_tau = true;
            continue;
          }

          sublead_tau = tau->p4();
          sublead_tau_charge = tau->charge();
          sublead_tau_nTracks = tau->nTracks();
          sublead_tau_isTauID = static_cast<int>(idTauDecorHandle(*tau));
          sublead_tau_isAntiTau = static_cast<int>(antiTauDecorHandle(*tau));
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
        m_Ibranches.at("Tau1_nProng").set(*event, lead_tau_nTracks, sys);
        m_Ibranches.at("Tau1_isTauID").set(*event, lead_tau_isTauID, sys);
        m_Ibranches.at("Tau1_isAntiTau").set(*event, lead_tau_isAntiTau, sys);
      }

      if(found_sublead_tau){
        m_Fbranches.at("Tau2_pt").set(*event, sublead_tau.Pt(), sys);
        m_Fbranches.at("Tau2_eta").set(*event, sublead_tau.Eta(), sys);
        m_Fbranches.at("Tau2_phi").set(*event, sublead_tau.Phi(), sys);
        m_Fbranches.at("Tau2_E").set(*event, sublead_tau.E(), sys);
        if(m_isMC) m_Fbranches.at("Tau2_effSF").set(*event, sublead_tau_effSF, sys);
        m_Ibranches.at("Tau2_charge").set(*event, sublead_tau_charge, sys);
        m_Ibranches.at("Tau2_nProng").set(*event, sublead_tau_nTracks, sys);
        m_Ibranches.at("Tau2_isTauID").set(*event, sublead_tau_isTauID, sys);
        m_Ibranches.at("Tau2_isAntiTau").set(*event, sublead_tau_isAntiTau, sys);
      }

      // DiHiggs mass 
      TLorentzVector bb(0,0,0,0);
      bool found_bb = false;

      bool WPgiven = !m_isBtag.empty();
      bool PCBTgiven = !m_PCBT.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      for(const xAOD::Jet* jet : *jets) {
        if (WPgiven) {
          if (m_isBtag.get(*jet, sys)) bjets->push_back(jet);
        }
      }
     
      if (bjets->size() > 1){

        for(unsigned int i=0; i<2; i++){
          std::string prefix = "Jet_b"+std::to_string(i+1);
          TLorentzVector tlv = bjets->at(i)->p4();
          m_Fbranches.at(prefix+"_pt").set(*event,  tlv.Pt(), sys);
          m_Fbranches.at(prefix+"_eta").set(*event, tlv.Eta(), sys);
          m_Fbranches.at(prefix+"_phi").set(*event, tlv.Phi(), sys);
          m_Fbranches.at(prefix+"_E").set(*event,   tlv.E(), sys);
          if(m_isMC) m_Ibranches.at(prefix+"_truthLabel").set
                       (*event, m_truthFlav.get(*bjets->at(i), sys), sys);
          if(PCBTgiven) m_Ibranches.at(prefix+"_pcbt").set
                          (*event, m_PCBT.get(*bjets->at(i), sys), sys);
        }

        TLorentzVector b1 = bjets->at(0)->p4();
        TLorentzVector b2 = bjets->at(1)->p4();
        bb = b1+b2;
        found_bb = true;
        m_Fbranches.at("H_bb_pt").set(*event, bb.Pt(), sys);
        m_Fbranches.at("H_bb_eta").set(*event, bb.Eta(), sys);
        m_Fbranches.at("H_bb_phi").set(*event, bb.Phi(), sys);
        m_Fbranches.at("H_bb_m").set(*event,  bb.M(), sys);
      }

      TLorentzVector tautau_vis(0,0,0,0);
      bool found_tautau_vis = false;
      if(found_lead_tau){
        if(found_lead_lep){
          tautau_vis = lead_tau + lead_lep;
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
