/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "BaselineVarsbbbbttAlg.h"

#include "AthContainers/AuxElement.h"
#include <AthContainers/ConstDataVector.h>

#include "TriggerUtils.h"

#include "TLorentzVector.h"

namespace HHHBBBBTT
{
  BaselineVarsbbbbttAlg::BaselineVarsbbbbttAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {

  }

  StatusCode BaselineVarsbbbbttAlg::initialize()
  {

    // Read syst-aware input handles
    ATH_CHECK (m_bbbbttJetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_bbbbttTauHandle.initialize(m_systematicsList));
    ATH_CHECK (m_bbbbttElectronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_bbbbttMuonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    if(m_doMMC){
      ATH_CHECK (m_mmc_pt.initialize(m_systematicsList, m_eventHandle));
      ATH_CHECK (m_mmc_eta.initialize(m_systematicsList, m_eventHandle));
      ATH_CHECK (m_mmc_phi.initialize(m_systematicsList, m_eventHandle));
      ATH_CHECK (m_mmc_m.initialize(m_systematicsList, m_eventHandle));
    }

    if(m_isMC){
      ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
      fillLeptonSfDecoMap(m_eleWPNames, m_ele_SF_decoMap);
      for(auto& [k, handle] : m_ele_SF_decoMap)
	ATH_CHECK (handle.initialize(m_systematicsList, m_electronHandle));

      ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
      fillLeptonSfDecoMap(m_muonWPNames, m_muon_SF_decoMap);
      for(auto& [k, handle] : m_muon_SF_decoMap)
        ATH_CHECK (handle.initialize(m_systematicsList, m_muonHandle));

      ATH_CHECK (m_tauHandle.initialize(m_systematicsList));
      m_tau_effSF = CP::SysReadDecorHandle<float>("effSF_"+m_tauWPName+"_%SYS%", this);
      ATH_CHECK (m_tau_effSF.initialize(m_systematicsList, m_tauHandle));
    }

    ATH_CHECK (m_selected_el.initialize(m_systematicsList, m_bbbbttElectronHandle));
    ATH_CHECK (m_selected_el_isIso.initialize(m_systematicsList, m_bbbbttElectronHandle));
    ATH_CHECK (m_selected_mu.initialize(m_systematicsList, m_bbbbttMuonHandle));
    ATH_CHECK (m_selected_mu_isIso.initialize(m_systematicsList, m_bbbbttMuonHandle));
    ATH_CHECK (m_selected_tau.initialize(m_systematicsList, m_bbbbttTauHandle));

    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_bbbbttJetHandle));
    }
    for (const std::string &var : m_PCBTnames){
      ATH_MSG_DEBUG("initializing PCBT: " << var);
      CP::SysReadDecorHandle<int> rhandle{var, this};
      m_PCBTs.emplace(var, rhandle);
      ATH_CHECK(m_PCBTs.at(var).initialize(m_systematicsList, m_bbbbttJetHandle));
    };
    if (m_isMC) {
      ATH_CHECK (m_truthFlav.initialize(m_systematicsList, m_bbbbttJetHandle));
    }
    ATH_CHECK (m_nmuons.initialize(m_systematicsList, m_bbbbttJetHandle));

    ATH_CHECK (m_IDTau.initialize(m_systematicsList, m_bbbbttTauHandle));
    ATH_CHECK (m_antiTau.initialize(m_systematicsList, m_bbbbttTauHandle));

    ATH_CHECK (m_EleRNNLoose.initialize(m_systematicsList, m_bbbbttTauHandle));
    ATH_CHECK (m_EleRNNMedium.initialize(m_systematicsList, m_bbbbttTauHandle));
    ATH_CHECK (m_EleRNNTight.initialize(m_systematicsList, m_bbbbttTauHandle));

    if (m_isMC) {
      ATH_CHECK (m_truthTypeTau.initialize(m_systematicsList, m_bbbbttTauHandle));
      ATH_CHECK (m_tauTruthJetLabel.initialize(m_systematicsList, m_bbbbttTauHandle));
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

  StatusCode BaselineVarsbbbbttAlg::execute()
  {
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {

      // Retrieve inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_bbbbttJetHandle.retrieve (jets, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_bbbbttMuonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_bbbbttElectronHandle.retrieve (electrons, sys));

      const xAOD::TauJetContainer *taus = nullptr;
      ANA_CHECK (m_bbbbttTauHandle.retrieve (taus, sys));

      for (const auto& var: m_floatVariables) {
        m_Fbranches.at(var).set(*event, -99, sys);
      }

      for (const auto& var: m_intVariables) {
        m_Ibranches.at(var).set(*event, -99, sys);
      }

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
          auto idx = m_useNonIsoLeptons 
                    ? HHHBBBBTT::LepSelWpDeco::tight_noniso 
                    : HHHBBBBTT::LepSelWpDeco::tight_iso;
          float SF = 1.;
          if(std::abs(leptons[i].second)==13)
            SF = m_muon_SF_decoMap.at(idx).get(*leptons[i].first,sys);
          else
            SF = m_ele_SF_decoMap.at(idx).get(*leptons[i].first,sys);
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
        if(m_EleRNNTight.get(*tau, sys)) tau_EleRNN_WP = 3;
        else if(m_EleRNNMedium.get(*tau, sys)) tau_EleRNN_WP = 2;
        else if(m_EleRNNLoose.get(*tau, sys)) tau_EleRNN_WP = 1;
        m_Ibranches.at(prefix+"_EleRNN_WP").set(*event, tau_EleRNN_WP, sys);

        if(m_isMC){
          m_Fbranches.at(prefix+"_effSF").set(*event, m_tau_effSF.get(*tau, sys), sys);
          m_Ibranches.at(prefix+"_truthType").set(*event, m_truthTypeTau.get(*tau, sys), sys);
                m_Ibranches.at(prefix+"_tauTruthJetLabel").set(*event, m_tauTruthJetLabel.get(*tau, sys), sys);
        }
      }

      // HH->4b contribution 
      TLorentzVector bbbb(0,0,0,0);
      bool found_bbbb = false;

      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      for(const xAOD::Jet* jet : *jets) {
        if (WPgiven) {
          if (m_isBtag.get(*jet, sys) && std::abs(jet->eta())<2.5)
	    bjets->push_back(jet);
        }
      }

      // Adding non-b-tagged jets if not having enough b-tagged jets
      if(bjets->size() < 4){
        for(const xAOD::Jet* jet : *jets){
          if(std::abs(jet->eta())<2.5) continue; // Only central jets
          bool isNonBTaggedJet = true;
          int insertPos = 0; 
          for(const xAOD::Jet* bjet : *bjets){
            if(jet==bjet){
              isNonBTaggedJet = false;
              break;
            }
            if(jet->pt() < bjet->pt()){
              // to keep bjets pt-ordered 
              ++insertPos;
            }
          }
          if(isNonBTaggedJet){
            bjets->insert(bjets->begin()+insertPos, jet);
          }
          if(bjets->size()==4) break;
        }
      }
      
      
     
      if (bjets->size() > 3){

        for(unsigned int i=0; i<4; i++){
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
        TLorentzVector b3 = bjets->at(2)->p4();
        TLorentzVector b4 = bjets->at(3)->p4();
        bbbb = b1+b2+b3+b4;
        found_bbbb = true;
        m_Fbranches.at("HH_bbbb_pt").set(*event, bbbb.Pt(), sys);
        m_Fbranches.at("HH_bbbb_eta").set(*event, bbbb.Eta(), sys);
        m_Fbranches.at("HH_bbbb_phi").set(*event, bbbb.Phi(), sys);
        m_Fbranches.at("HH_bbbb_m").set(*event,  bbbb.M(), sys);
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

      if(found_bbbb && found_tautau_vis){

        TLorentzVector HHH_vis = bbbb+tautau_vis;

        m_Fbranches.at("HHH_vis_pt").set(*event, HHH_vis.Pt(), sys);
        m_Fbranches.at("HHH_vis_eta").set(*event, HHH_vis.Eta(), sys);
        m_Fbranches.at("HHH_vis_phi").set(*event, HHH_vis.Phi(), sys);
        m_Fbranches.at("HHH_vis_m").set(*event, HHH_vis.M(), sys);

        float MMC_m = m_doMMC ? m_mmc_m.get(*event, sys) : 0.;
        if(MMC_m>0){
          TLorentzVector mmc_vec(0,0,0,0);
          mmc_vec.SetPtEtaPhiM(m_mmc_pt.get(*event, sys),
                               m_mmc_eta.get(*event, sys),
                               m_mmc_phi.get(*event, sys),
                               MMC_m);
          TLorentzVector HHH = bbbb+mmc_vec;
          m_Fbranches.at("HHH_pt").set(*event, HHH.Pt(), sys);
          m_Fbranches.at("HHH_eta").set(*event, HHH.Eta(), sys);
          m_Fbranches.at("HHH_phi").set(*event, HHH.Phi(), sys);
          m_Fbranches.at("HHH_m").set(*event, HHH.M(), sys);

        }        
      }
    }

    return StatusCode::SUCCESS;
  }

  void BaselineVarsbbbbttAlg::fillLeptonSfDecoMap(const std::vector<std::string>& wpNames,
						leptonSfDecoMap& decoMap){
    for(auto& wp : wpNames){
      CP::SysReadDecorHandle<float> handle{"effSF_"+wp+"_%SYS%", this};
      
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
      if (isTight && isIso){
        decoMap.emplace(HHHBBBBTT::LepSelWpDeco::tight_iso, handle);
        ATH_MSG_INFO("found tight iso wp = "<< wp);
      }
    }
  }

}
