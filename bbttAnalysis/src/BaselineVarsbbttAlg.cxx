/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "BaselineVarsbbttAlg.h"

#include "AthContainers/AuxElement.h"
#include <AthContainers/ConstDataVector.h>

#include "TriggerUtils.h"

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
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    ATH_CHECK (m_mmc_pt.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_mmc_eta.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_mmc_phi.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_mmc_m.initialize(m_systematicsList, m_eventHandle));

    if(m_isMC){
      for(const auto& wp : m_eleWPNames){
        m_ele_SF.emplace_back("el_effSF_"+wp+"_%SYS%", this);
      }
    }
    for(auto& handle : m_ele_SF)
      ATH_CHECK (handle.initialize(m_systematicsList, m_electronHandle, SG::AllowEmpty));

    if(m_isMC){
      for(const auto& wp : m_muonWPNames){
        m_mu_SF.emplace_back("muon_effSF_"+wp+"_%SYS%", this);
      }
    }
    for(auto& handle : m_mu_SF)
      ATH_CHECK (handle.initialize(m_systematicsList, m_muonHandle, SG::AllowEmpty));

    if(m_isMC){
      m_tau_effSF = CP::SysReadDecorHandle<float>("tau_effSF_"+m_tauWPName+"_%SYS%", this);
    }
    ATH_CHECK (m_tau_effSF.initialize(m_systematicsList, m_tauHandle, SG::AllowEmpty));

    ATH_CHECK (m_selected_el.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK (m_selected_el_isIso.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK (m_selected_mu.initialize(m_systematicsList, m_muonHandle));
    ATH_CHECK (m_selected_mu_isIso.initialize(m_systematicsList, m_muonHandle));
    ATH_CHECK (m_selected_tau.initialize(m_systematicsList, m_tauHandle));

    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }
    for (const std::string &var : m_PCBTnames){
      ATH_MSG_DEBUG("initializing PCBT: " << var);
      CP::SysReadDecorHandle<int> rhandle{var, this};
      m_PCBTs.emplace(var, rhandle);
      ATH_CHECK(m_PCBTs.at(var).initialize(m_systematicsList, m_jetHandle));
    };
    if (m_isMC) {
      ATH_CHECK (m_truthFlav.initialize(m_systematicsList, m_jetHandle));
    }
    ATH_CHECK (m_nmuons.initialize(m_systematicsList, m_jetHandle));

    ATH_CHECK (m_IDTau.initialize(m_systematicsList, m_tauHandle));
    ATH_CHECK (m_antiTau.initialize(m_systematicsList, m_tauHandle));

    if (m_isMC) {
      ATH_CHECK (m_truthTypeTau.initialize(m_systematicsList, m_tauHandle));
      ATH_CHECK (m_tauTruthJetLabel.initialize(m_systematicsList, m_tauHandle));
    }

    ATH_CHECK(m_year.initialize(m_systematicsList, m_eventHandle));

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

      for (const auto& var: m_floatVariables) {
        m_Fbranches.at(var).set(*event, -99, sys);
      }

      for (const auto& var: m_intVariables) {
        m_Ibranches.at(var).set(*event, -99, sys);
      }

      static const SG::AuxElement::ConstAccessor<char> cacc_EleRNNLoose("EleRNNLoose_v1");
      static const SG::AuxElement::ConstAccessor<char> cacc_EleRNNMedium("EleRNNMedium_v1");
      static const SG::AuxElement::ConstAccessor<char> cacc_EleRNNTight("EleRNNTight_v1");

      // selected leptons ;
      const xAOD::Electron* ele0 = nullptr;
      const xAOD::Electron* ele1 = nullptr;

      for(const xAOD::Electron* electron : *electrons) {
        if (m_selected_el.get(*electron, sys)){
          if(!ele0) ele0 = electron;
          else{
            ele1 = electron;
            break;
          }
        }
      }

      const xAOD::Muon* mu0 = nullptr;
      const xAOD::Muon* mu1 = nullptr;
      for(const xAOD::Muon* muon : *muons) {
        if (m_selected_mu.get(*muon, sys)){
          if(!mu0) mu0 = muon;
          else{
            mu1 = muon;
            break;
          }
        }
      }

      std::vector<std::pair<const xAOD::IParticle*, int>> leptons;
      if(ele0) leptons.emplace_back(ele0, -11*ele0->charge());
      if(mu0) leptons.emplace_back(mu0, -13*mu0->charge());
      if(ele1) leptons.emplace_back(ele1, -11*ele1->charge());
      if(mu1) leptons.emplace_back(mu1, -13*mu1->charge());

      std::sort(leptons.begin(), leptons.end(),
		[](const std::pair<const xAOD::IParticle*, int>& a,
		   const std::pair<const xAOD::IParticle*, int>& b) {
		  return a.first->pt() > b.first->pt(); });

      for(unsigned int i=0; i<std::min(size_t(2),leptons.size()); i++){
        std::string prefix = "Lepton"+std::to_string(i+1);
        TLorentzVector tlv = leptons[i].first->p4();
        m_Fbranches.at(prefix+"_pt").set(*event, tlv.Pt(), sys);
        m_Fbranches.at(prefix+"_eta").set(*event, tlv.Eta(), sys);
        m_Fbranches.at(prefix+"_phi").set(*event, tlv.Phi(), sys);
        m_Fbranches.at(prefix+"_E").set(*event, tlv.E(), sys);
        int charge = leptons[i].second>0 ? -1 : 1;
        m_Ibranches.at(prefix+"_charge").set(*event, charge, sys);
        m_Ibranches.at(prefix+"_pdgid").set(*event, leptons[i].second, sys);
        bool lep_is_isolated = (std::abs(leptons[i].second) == 13) 
                               ? m_selected_mu_isIso.get(*leptons[i].first, sys) 
                               : m_selected_el_isIso.get(*leptons[i].first, sys);
        m_Ibranches.at(prefix+"_isIso").set(*event, (int)lep_is_isolated, sys);
        if(m_isMC){
          int sfWpIndex = (m_muonWPNames.size() > 1 && !lep_is_isolated) ? 2 : 0; // TODO: fixme
          float SF = std::abs(leptons[i].second)==11 ?
            m_ele_SF[sfWpIndex].get(*leptons[i].first,sys) :
            m_mu_SF[sfWpIndex].get(*leptons[i].first,sys);
          m_Fbranches.at(prefix+"_effSF").set(*event, SF, sys);
        }
      }

      //selected tau
      const xAOD::TauJet* tau0 = nullptr;
      const xAOD::TauJet* tau1 = nullptr;

      for(const xAOD::TauJet* tau : *taus) {
        if (m_selected_tau.get(*tau, sys)){
          if(!tau0) tau0 = tau;
	  else if(!tau1){
	    tau1 = tau;
	    break;
	  }
        }
      }

      std::vector<const xAOD::TauJet*> sel_taus = {tau0, tau1};

      for(unsigned int i=0; i<2; i++){
	const xAOD::TauJet* tau = sel_taus[i];
	if(!tau) break;
	std::string prefix = "Tau"+std::to_string(i+1);
	TLorentzVector tlv = tau->p4();
	m_Fbranches.at(prefix+"_pt").set(*event, tlv.Pt(), sys);
	m_Fbranches.at(prefix+"_eta").set(*event, tlv.Eta(), sys);
	m_Fbranches.at(prefix+"_phi").set(*event, tlv.Phi(), sys);
	m_Fbranches.at(prefix+"_E").set(*event, tlv.E(), sys);
	m_Fbranches.at(prefix+"_RNN").set(*event, tau->discriminant(xAOD::TauJetParameters::RNNJetScoreSigTrans), sys);

	m_Ibranches.at(prefix+"_charge").set(*event, tau->charge(), sys);
	m_Ibranches.at(prefix+"_nProng").set(*event, tau->nTracks(), sys);
	int decayMode=-1;
	tau->panTauDetail(xAOD::TauJetParameters::PanTau_DecayMode, decayMode);
	m_Ibranches.at(prefix+"_decayMode").set(*event, decayMode, sys);
	m_Ibranches.at(prefix+"_isTauID").set(*event, m_IDTau.get(*tau, sys), sys);
	m_Ibranches.at(prefix+"_isAntiTau").set(*event, m_antiTau.get(*tau, sys), sys);
	int tau_EleRNN_WP = 0;
	if(cacc_EleRNNTight(*tau)) tau_EleRNN_WP = 3;
	else if(cacc_EleRNNMedium(*tau)) tau_EleRNN_WP = 2;
	else if(cacc_EleRNNLoose(*tau)) tau_EleRNN_WP = 1;
	m_Ibranches.at(prefix+"_EleRNN_WP").set(*event, tau_EleRNN_WP, sys);

	if(m_isMC){
	  m_Fbranches.at(prefix+"_effSF").set(*event, m_tau_effSF.get(*tau, sys), sys);
	  m_Ibranches.at(prefix+"_truthType").set(*event, m_truthTypeTau.get(*tau, sys), sys);
          m_Ibranches.at(prefix+"_tauTruthJetLabel").set(*event, m_tauTruthJetLabel.get(*tau, sys), sys);
	}
      }

      // DiHiggs mass 
      TLorentzVector bb(0,0,0,0);
      bool found_bb = false;

      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      for(const xAOD::Jet* jet : *jets) {
        if (WPgiven) {
          if (m_isBtag.get(*jet, sys) && std::abs(jet->eta())<2.5)
	    bjets->push_back(jet);
        }
      }

      // Expand b-jets with extra jet for 1-btag events
      if(bjets->size()==1){
        for(const xAOD::Jet* jet : *jets){
          if(jet!=bjets->at(0) && std::abs(jet->eta())<2.5){
            if(jet->pt() > bjets->at(0)->pt()) bjets->insert(bjets->begin(),jet);
            else bjets->push_back(jet);
            break;
          }
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
          for (const auto& var: m_PCBTnames) {
            std::string new_var = var;
            new_var.erase(0, 14); // remove 'ftag_quantile_' from var name
            new_var.erase(new_var.length() - 11, new_var.length()); // remove '_Continuous' from var name
            m_Ibranches.at(prefix+"_pcbt_"+new_var).set(*event, m_PCBTs.at(var).get(*bjets->at(i), sys), sys);
          }
          m_Ibranches.at(prefix+"_nmuons").set
	    (*event, m_nmuons.get(*bjets->at(i), sys), sys);
          float uncorrPt = bjets->at(i)->jetP4("NoBJetCalibMomentum").Pt();
          m_Fbranches.at(prefix+"_uncorrPt").set(*event, uncorrPt, sys);
          float muonCorrPt = bjets->at(i)->jetP4("MuonCorrMomentum").Pt();
          m_Fbranches.at(prefix+"_muonCorrPt").set(*event, muonCorrPt, sys);
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
      if(tau0){
        if(leptons.size()>0){
          tautau_vis = tau0->p4() + leptons[0].first->p4();
          found_tautau_vis = true;
        }else if(tau1){
          tautau_vis = tau0->p4() + tau1->p4();
          found_tautau_vis = true;
        }
        if(found_tautau_vis){
          m_Fbranches.at("H_vis_tautau_pt").set(*event, tautau_vis.Pt(), sys);
          m_Fbranches.at("H_vis_tautau_eta").set(*event, tautau_vis.Eta(), sys);
          m_Fbranches.at("H_vis_tautau_phi").set(*event, tautau_vis.Phi(), sys);
          m_Fbranches.at("H_vis_tautau_m").set(*event,  tautau_vis.M(), sys);
        }
      }

      if(found_bb && found_tautau_vis){

        TLorentzVector HH_vis = bb+tautau_vis;

        m_Fbranches.at("HH_vis_pt").set(*event, HH_vis.Pt(), sys);
        m_Fbranches.at("HH_vis_eta").set(*event, HH_vis.Eta(), sys);
        m_Fbranches.at("HH_vis_phi").set(*event, HH_vis.Phi(), sys);
        m_Fbranches.at("HH_vis_m").set(*event, HH_vis.M(), sys);

        float MMC_m = m_mmc_m.get(*event, sys);
	TLorentzVector mmc_vec(0,0,0,0);
        if(MMC_m>0){
          mmc_vec.SetPtEtaPhiM(m_mmc_pt.get(*event, sys),
                               m_mmc_eta.get(*event, sys),
                               m_mmc_phi.get(*event, sys),
                               MMC_m);

          TLorentzVector HH = bb+mmc_vec;
          m_Fbranches.at("HH_pt").set(*event, HH.Pt(), sys);
          m_Fbranches.at("HH_eta").set(*event, HH.Eta(), sys);
          m_Fbranches.at("HH_phi").set(*event, HH.Phi(), sys);
          m_Fbranches.at("HH_m").set(*event, HH.M(), sys);
        }

        if(MMC_m>0) m_Fbranches.at("HH_delta_phi").set(*event, bb.DeltaPhi(mmc_vec),sys);
        m_Fbranches.at("HH_vis_delta_phi").set(*event, bb.DeltaPhi(tautau_vis),sys);
      }
    }

    return StatusCode::SUCCESS;
  }

}
