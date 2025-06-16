/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "BaselineVarsSemiLepAlg.h"
#include "AthContainers/AuxElement.h"
#include "TLorentzVector.h"
#include <AthContainers/ConstDataVector.h>

namespace VBSHIGGS{
    BaselineVarsSemiLepAlg::BaselineVarsSemiLepAlg(const std::string &name,
                                            ISvcLocator *pSvcLocator)
    : AthHistogramAlgorithm(name, pSvcLocator){

    }

    StatusCode BaselineVarsSemiLepAlg::initialize(){
      ATH_MSG_INFO("*********************************\n");
      ATH_MSG_INFO("       SemiLepBaselineVarsAlg    \n");
      ATH_MSG_INFO("*********************************\n");

      // Read syst-aware input handles
      ATH_CHECK (m_vbsLRJetHandle.initialize(m_systematicsList));
      ATH_CHECK (m_signaljetHandle.initialize(m_systematicsList));
      ATH_CHECK (m_HCandHandle.initialize(m_systematicsList));
      ATH_CHECK (m_vbsJetHandle.initialize(m_systematicsList));
      ATH_CHECK (m_vbsElectronHandle.initialize(m_systematicsList));
      ATH_CHECK (m_vbsMuonHandle.initialize(m_systematicsList));
      ATH_CHECK (m_metHandle.initialize(m_systematicsList));
      ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

      if(m_isMC){
        ATH_CHECK (m_ele_truthOrigin.initialize(m_systematicsList, m_vbsElectronHandle));
        ATH_CHECK (m_ele_truthType.initialize(m_systematicsList, m_vbsElectronHandle));
        ATH_CHECK (m_mu_truthOrigin.initialize(m_systematicsList, m_vbsMuonHandle));
        ATH_CHECK (m_mu_truthType.initialize(m_systematicsList, m_vbsMuonHandle));

	ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
	m_ele_SF = CP::SysReadDecorHandle<float>("effSF_"+m_eleWPName+"_%SYS%", this);
	ATH_CHECK (m_ele_SF.initialize(m_systematicsList, m_electronHandle));

        ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
        m_mu_SF = CP::SysReadDecorHandle<float>("effSF_"+m_muWPName+"_%SYS%", this);
        ATH_CHECK (m_mu_SF.initialize(m_systematicsList, m_muonHandle));
      }

      if (!m_isBtag.empty()) {
        ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_signaljetHandle));
      }

      if (!m_PCBT.empty()) {
        ATH_CHECK (m_PCBT.initialize(m_systematicsList, m_HCandHandle));
      }

      // Intialise syst-aware output decorators
      for (const std::string &var : m_floatVariables) {
        CP::SysWriteDecorHandle<float> whandle{var+"_%SYS%", this};
        m_Fbranches.emplace(var, whandle);
        ATH_CHECK (m_Fbranches.at(var).initialize(m_systematicsList, m_eventHandle));
      }

      for (const std::string &var : m_intVariables){
        ATH_MSG_DEBUG("initializing integer variable: " << var);
        CP::SysWriteDecorHandle<int> whandle{var+"_%SYS%", this};
        m_Ibranches.emplace(var, whandle);
        ATH_CHECK(m_Ibranches.at(var).initialize(m_systematicsList, m_eventHandle));
      };

      if (m_isMC) {
        ATH_CHECK (m_truthFlav.initialize(m_systematicsList, m_HCandHandle));
      }

      ATH_CHECK (m_eleECIDS.initialize(m_systematicsList, m_vbsElectronHandle));
      
      ATH_CHECK (m_METSig.initialize(m_systematicsList, m_metHandle));

      ATH_CHECK (m_GN2Xv01_phbb.initialize(m_systematicsList, m_vbsLRJetHandle));
      ATH_CHECK (m_GN2Xv01_phcc.initialize(m_systematicsList, m_vbsLRJetHandle));
      ATH_CHECK (m_GN2Xv01_pqcd.initialize(m_systematicsList, m_vbsLRJetHandle));
      ATH_CHECK (m_GN2Xv01_ptop.initialize(m_systematicsList, m_vbsLRJetHandle));

      // Intialise syst list (must come after all syst-aware inputs and outputs)
      ATH_CHECK (m_systematicsList.initialize());
      return StatusCode::SUCCESS;
    }

    StatusCode BaselineVarsSemiLepAlg::execute(){
      // Loop over all systs
      for (const auto& sys : m_systematicsList.systematicsVector()){
        // Retrieve inputs
        const xAOD::EventInfo *event = nullptr;
        ANA_CHECK (m_eventHandle.retrieve (event, sys));

        const xAOD::JetContainer *largeJets = nullptr;
        ANA_CHECK (m_vbsLRJetHandle.retrieve (largeJets, sys));

        const xAOD::JetContainer *signalJets = nullptr;
        ANA_CHECK (m_signaljetHandle.retrieve (signalJets, sys));
        
        const xAOD::JetContainer *HJets = nullptr;
        ANA_CHECK (m_HCandHandle.retrieve (HJets, sys));

        const xAOD::JetContainer *vbsjets = nullptr;
        ANA_CHECK (m_vbsJetHandle.retrieve (vbsjets, sys));

        const xAOD::MuonContainer *muons = nullptr;
        ANA_CHECK (m_vbsMuonHandle.retrieve (muons, sys));

        const xAOD::ElectronContainer *electrons = nullptr;
        ANA_CHECK (m_vbsElectronHandle.retrieve (electrons, sys));

        const xAOD::MissingETContainer *metCont = nullptr;
        ANA_CHECK (m_metHandle.retrieve (metCont, sys));
        const xAOD::MissingET* met = (*metCont)["Final"];
        if (!met) {
          ATH_MSG_ERROR("Could not retrieve MET");
          return StatusCode::FAILURE;	
        }
        for (const std::string &string_var: m_floatVariables) {
          m_Fbranches.at(string_var).set(*event, -99., sys);
        }

        for (const auto& var: m_intVariables) {
          m_Ibranches.at(var).set(*event, -99, sys);
        }
        
        int n_jets = signalJets->size() + vbsjets->size();
        int n_largeJets = largeJets->size();
        int nCentralJets = 0;
        int nForwardJets = 0;

        int n_electrons = electrons->size();
        int n_muons = muons->size();

        // b-jet sector
        bool WPgiven = !m_isBtag.empty();
        bool PCBTgiven = !m_PCBT.empty();
        auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);

        // number of central jets
        for(const xAOD::Jet* jet : *signalJets) {
          // count central jets
          if (std::abs(jet->eta())<2.5) nCentralJets++;
          else nForwardJets++;

          if (WPgiven) {
            if (m_isBtag.get(*jet, sys) && std::abs(jet->eta())<2.5) bjets->push_back(jet);
          }
        }

        for(const xAOD::Jet* vbsjet : *vbsjets) {
          if (std::abs(vbsjet->eta())<2.5) nCentralJets++;
          else nForwardJets++;
        }

        int n_bjets = bjets->size();

        m_Ibranches.at("nJets").set(*event, n_jets, sys);
        m_Ibranches.at("nLargeJets").set(*event, n_largeJets, sys);
        m_Ibranches.at("nElectrons").set(*event, n_electrons, sys);
        m_Ibranches.at("nMuons").set(*event, n_muons, sys);
        m_Ibranches.at("nBJets").set(*event, n_bjets, sys);
        m_Ibranches.at("nCentralJets").set(*event, nCentralJets, sys);
        m_Ibranches.at("nForwardJets").set(*event, nForwardJets, sys);
        m_Ibranches.at("nLeptons").set(*event, n_muons + n_electrons, sys);

        // selected leptons ;
        const xAOD::Electron* ele0 = n_electrons>=1 ? electrons->at(0) : nullptr;

        const xAOD::Muon* mu0 = n_muons>=1 ? muons->at(0) : nullptr;

        std::vector<std::pair<const xAOD::IParticle*, int>> leptons;
        if(ele0) leptons.emplace_back(ele0, -11*ele0->charge());
        if(mu0) leptons.emplace_back(mu0, -13*mu0->charge());

        std::sort(leptons.begin(), leptons.end(),
            [](const std::pair<const xAOD::IParticle*, int>& a,
               const std::pair<const xAOD::IParticle*, int>& b) {
              return a.first->pt() > b.first->pt(); });

        TLorentzVector metVec;
        metVec.SetPtEtaPhiE(met->met(), 0, met->phi(), met->met());
        
        if( !leptons.empty() ){
          TLorentzVector tlv = leptons[0].first->p4();
          int lep_pdgid = leptons[0].second;
          m_Fbranches.at("Lepton_pt").set(*event,  tlv.Pt(), sys);
          m_Fbranches.at("Lepton_eta").set(*event, tlv.Eta(), sys);
          m_Fbranches.at("Lepton_phi").set(*event, tlv.Phi(), sys);
          m_Fbranches.at("Lepton_E").set(*event, tlv.E(), sys);

          m_Fbranches.at("dPhilMET").set(*event, tlv.DeltaPhi(metVec), sys);
	        
          float mt_lept_met = std::sqrt(2 * met->met() * tlv.Pt() * (1 - std::cos(tlv.DeltaPhi(metVec))));
          m_Fbranches.at("Lepton_MET_mT").set(*event, mt_lept_met, sys);

          if(m_isMC){
            float SF = -99;
            if(std::abs(leptons[0].second)==11 ){
              SF = m_ele_SF.get(*leptons[0].first,sys);

              int ele_ECIDS = m_eleECIDS.get(*leptons[0].first, sys);
              m_Ibranches.at("Lepton_ele_ECIDS").set(*event, ele_ECIDS, sys);
            }
            else{
              SF = m_mu_SF.get(*leptons[0].first,sys);
            }
            m_Fbranches.at("Lepton_effSF").set(*event, SF, sys);
          }
          int charge = leptons[0].second>0 ? -1 : 1;
          m_Ibranches.at("Lepton_charge").set(*event, charge, sys);
          m_Ibranches.at("Lepton_pdgid").set(*event, lep_pdgid, sys);
        
          // leptons truth information
          if (m_isMC){
            int lep_truthOrigin = std::abs(lep_pdgid)==11 ?
              m_ele_truthOrigin.get(*leptons[0].first,sys) :
              m_mu_truthOrigin.get(*leptons[0].first,sys);
            m_Ibranches.at("Lepton_truthOrigin").set(*event, lep_truthOrigin, sys);
            int lep_truthType = std::abs(lep_pdgid)==11 ?
              m_ele_truthType.get(*leptons[0].first,sys) :
              m_mu_truthType.get(*leptons[0].first,sys);
            m_Ibranches.at("Lepton_truthType").set(*event, lep_truthType, sys);
            int lep_isPrompt = 0;
            if (std::abs(lep_pdgid)==13){ // simplistic
              if (lep_truthType==6) lep_isPrompt=1; // isolated prompts
            } else if (std::abs(lep_pdgid)==11){
              if (lep_truthType==2) lep_isPrompt=1; // isolated prompts
            }
            
            m_Ibranches.at("Lepton_isPrompt").set(*event, lep_isPrompt, sys);
          }
          
        }

        //MET Significance 
        float METSig = m_METSig.get(*met, sys);
        m_Fbranches.at("METSig").set(*event, METSig, sys);

        //jet sector
        for (std::size_t i=0; i<std::min(signalJets->size(),(std::size_t)2); i++){
          m_Fbranches.at("Jet"+std::to_string(i+1)+"_pt").set(*event, signalJets->at(i)->pt(), sys);
          m_Fbranches.at("Jet"+std::to_string(i+1)+"_eta").set(*event, signalJets->at(i)->eta(), sys);
          m_Fbranches.at("Jet"+std::to_string(i+1)+"_phi").set(*event, signalJets->at(i)->phi(), sys);
          m_Fbranches.at("Jet"+std::to_string(i+1)+"_E").set(*event, signalJets->at(i)->e(), sys); 
        }

        //b-jet sector
        for (std::size_t i=0; i<std::min(HJets->size(),(std::size_t)2); i++){
          std::string prefix = "Jet_Higgs_candidate"+std::to_string(i+1);
          m_Fbranches.at(prefix+"_m").set(*event, HJets->at(i)->m(), sys);
          m_Fbranches.at(prefix+"_pt").set(*event, HJets->at(i)->pt(), sys);
          m_Fbranches.at(prefix+"_eta").set(*event, HJets->at(i)->eta(), sys);
          m_Fbranches.at(prefix+"_phi").set(*event, HJets->at(i)->phi(), sys);
          m_Fbranches.at(prefix+"_E").set(*event, HJets->at(i)->e(), sys);

          if(PCBTgiven)
            m_Ibranches.at(prefix+"_pcbt").set(*event, m_PCBT.get(*HJets->at(i), sys),sys);

          if (m_isMC) {
            m_Ibranches.at(prefix+"_truthLabel").set(*event, m_truthFlav.get(*HJets->at(i), sys), sys);
          }
        }

        if (HJets->size() >=2){
          TLorentzVector HDijet = HJets->at(0)->p4()+ HJets->at(1)->p4();
          TLorentzVector HJet1 = HJets->at(0)->p4();
          TLorentzVector HJet2 = HJets->at(1)->p4();
          m_Fbranches.at("Hdijet_m").set(*event, HDijet.M(), sys);
          m_Fbranches.at("Hdijet_pt").set(*event, HDijet.Pt(), sys);
          m_Fbranches.at("Hdijet_eta").set(*event, HDijet.Eta(), sys);
          m_Fbranches.at("Hdijet_phi").set(*event, HDijet.Phi(), sys);
          m_Fbranches.at("dRHjj").set(*event, HJet1.DeltaR(HJet2), sys);
          m_Fbranches.at("dPhiHjj").set(*event, HJet1.DeltaPhi(HJet2), sys);
          m_Fbranches.at("dEtaHjj").set(*event, HJet1.Eta() - HJet2.Eta(), sys);
        }

        //min Delta R (bjet, lepton)
        std::vector<double> deltaRs;
        if( !leptons.empty() ){
          TLorentzVector tlv = leptons[0].first->p4();
          for (const auto& HJet : *HJets) {
            deltaRs.push_back(HJet->p4().DeltaR(tlv));
          }
        }
        if (!deltaRs.empty()) {
          auto minDeltaR = *std::min_element(std::begin(deltaRs), std::end(deltaRs));
          m_Fbranches.at("dRbl_min").set(*event, minDeltaR, sys);
        }

        // Large R jets kinematics
        if ( n_largeJets >= 1 ){
          const xAOD::Jet* largeJet = largeJets->at(0);
          float phbb = m_GN2Xv01_phbb.get(*largeJet, sys);
          float phcc = m_GN2Xv01_phcc.get(*largeJet, sys);
          float pqcd = m_GN2Xv01_pqcd.get(*largeJet, sys);
          float ptop = m_GN2Xv01_ptop.get(*largeJet, sys);
          float fcc = 0.02;
          float ftop = 0.25;
          float XbbScore= log (phbb / (fcc*phcc + ftop*ptop + pqcd*(1-fcc-ftop)));
          m_Fbranches.at("LargeJet1_m").set(*event, largeJet->m(), sys);
          m_Fbranches.at("LargeJet1_pt").set(*event, largeJet->pt(), sys);
          m_Fbranches.at("LargeJet1_eta").set(*event, largeJet->eta(), sys);
          m_Fbranches.at("LargeJet1_phi").set(*event, largeJet->phi(), sys);

          m_Fbranches.at("LargeJet1_phbb").set(*event, phbb, sys);
          m_Fbranches.at("LargeJet1_phcc").set(*event, phcc, sys);
          m_Fbranches.at("LargeJet1_pqcd").set(*event, pqcd, sys);
          m_Fbranches.at("LargeJet1_ptop").set(*event, ptop, sys);
          m_Fbranches.at("LargeJet1_DXbb").set(*event, XbbScore, sys);
        }

        // kinematics of vbs jets
        if ( vbsjets->size() >= 2 ){
          const xAOD::Jet* vbsJet1 = vbsjets->at(0);
          const xAOD::Jet* vbsJet2 = vbsjets->at(1);

          TLorentzVector vbs_jj = vbsJet1->p4() + vbsJet2->p4();
          
          m_Fbranches.at("VBSJ1_m").set(*event, vbsJet1->m(), sys);
          m_Fbranches.at("VBSJ1_pt").set(*event, vbsJet1->pt(), sys);
          m_Fbranches.at("VBSJ1_eta").set(*event, vbsJet1->eta(), sys);
          m_Fbranches.at("VBSJ1_phi").set(*event, vbsJet1->phi(), sys);

          m_Fbranches.at("VBSJ2_m").set(*event, vbsJet2->m(), sys);
          m_Fbranches.at("VBSJ2_pt").set(*event, vbsJet2->pt(), sys);
          m_Fbranches.at("VBSJ2_eta").set(*event, vbsJet2->eta(), sys);
          m_Fbranches.at("VBSJ2_phi").set(*event, vbsJet2->phi(), sys);

          m_Fbranches.at("VBSdijet_m").set(*event, vbs_jj.M(), sys);
          m_Fbranches.at("VBSdijet_pt").set(*event, vbs_jj.Pt(), sys);
          m_Fbranches.at("VBSdijet_eta").set(*event, vbs_jj.Eta(), sys);
          m_Fbranches.at("VBSdijet_phi").set(*event, vbs_jj.Phi(), sys);
          m_Fbranches.at("dRVBSjj").set(*event, vbsJet1->p4().DeltaR(vbsJet2->p4()), sys);
          m_Fbranches.at("dEtaVBSjj").set(*event, vbsJet1->eta()-vbsJet2->eta(), sys);
          m_Fbranches.at("dPhiVBSjj").set(*event, vbsJet1->p4().DeltaPhi(vbsJet2->p4()), sys);
          
        }
      }
      
      return StatusCode::SUCCESS;
    }
}
