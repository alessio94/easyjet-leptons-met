/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "BaselineVarsFullLepAlg.h"
#include "AthContainers/AuxElement.h"
#include "TLorentzVector.h"

namespace VBSHIGGS{
    BaselineVarsFullLepAlg::BaselineVarsFullLepAlg(const std::string &name,
                                                ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator){

    }

    StatusCode BaselineVarsFullLepAlg::initialize(){
      ATH_MSG_INFO("*********************************\n");
      ATH_MSG_INFO("       FullLepBaselineVarsAlg    \n");
      ATH_MSG_INFO("*********************************\n");

      // Read syst-aware input handles
      ATH_CHECK (m_largejetHandle.initialize(m_systematicsList));
      ATH_CHECK (m_signaljetHandle.initialize(m_systematicsList));
      ATH_CHECK (m_HCandHandle.initialize(m_systematicsList));
      ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
      ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
      ATH_CHECK (m_metHandle.initialize(m_systematicsList));
      ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

      if( !m_UseVBFRNN ){
        ATH_CHECK (m_vbsjetHandle.initialize(m_systematicsList));
      }
      else {
        ATH_CHECK (m_RNNjetBoosted20GeVHandle.initialize(m_systematicsList));
        ATH_CHECK (m_RNNjetBoosted30GeVHandle.initialize(m_systematicsList));
        ATH_CHECK (m_RNNjetResolved20GeVHandle.initialize(m_systematicsList));
        ATH_CHECK (m_RNNjetResolved30GeVHandle.initialize(m_systematicsList));
      }


      if(m_isMC){
        m_ele_SF = CP::SysReadDecorHandle<float>("el_effSF_"+m_eleWPName+"_%SYS%", this);
	ATH_CHECK (m_ele_SF.initialize(m_systematicsList, m_electronHandle));
	ATH_CHECK (m_ele_truthOrigin.initialize(m_systematicsList, m_electronHandle));
	ATH_CHECK (m_ele_truthType.initialize(m_systematicsList, m_electronHandle));
      }

      if(m_isMC){
        m_mu_SF = CP::SysReadDecorHandle<float>("muon_effSF_"+m_muWPName+"_%SYS%", this);
	ATH_CHECK (m_mu_SF.initialize(m_systematicsList, m_muonHandle));
	ATH_CHECK (m_mu_truthOrigin.initialize(m_systematicsList, m_muonHandle));
	ATH_CHECK (m_mu_truthType.initialize(m_systematicsList, m_muonHandle));
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

      ATH_CHECK (m_eleECIDS.initialize(m_systematicsList, m_electronHandle));
      
      ATH_CHECK (m_METSig.initialize(m_systematicsList, m_metHandle));

      ATH_CHECK (m_GN2Xv01_phbb.initialize(m_systematicsList, m_largejetHandle));
      ATH_CHECK (m_GN2Xv01_phcc.initialize(m_systematicsList, m_largejetHandle));
      ATH_CHECK (m_GN2Xv01_pqcd.initialize(m_systematicsList, m_largejetHandle));
      ATH_CHECK (m_GN2Xv01_ptop.initialize(m_systematicsList, m_largejetHandle));

      // Intialise syst list (must come after all syst-aware inputs and outputs)
      ATH_CHECK (m_systematicsList.initialize());
      return StatusCode::SUCCESS;
    }

    StatusCode BaselineVarsFullLepAlg::execute(){
      // Loop over all systs
      for (const auto& sys : m_systematicsList.systematicsVector()){
        // Retrieve inputs
        const xAOD::EventInfo *event = nullptr;
        ANA_CHECK (m_eventHandle.retrieve (event, sys));

        const xAOD::JetContainer *largeJets = nullptr;
        ANA_CHECK (m_largejetHandle.retrieve (largeJets, sys));

        const xAOD::JetContainer *signalJets = nullptr;
        ANA_CHECK (m_signaljetHandle.retrieve (signalJets, sys));
        
        const xAOD::JetContainer *HJets = nullptr;
        ANA_CHECK (m_HCandHandle.retrieve (HJets, sys));

        const xAOD::JetContainer *vbsjets = nullptr;
        const xAOD::JetContainer *RNNJets_boosted_20gev = nullptr;
        const xAOD::JetContainer *RNNJets_boosted_30gev = nullptr;
        const xAOD::JetContainer *RNNJets_resolved_20gev = nullptr;
        const xAOD::JetContainer *RNNJets_resolved_30gev = nullptr;
        if(!m_UseVBFRNN) {
          ANA_CHECK (m_vbsjetHandle.retrieve (vbsjets, sys));
        }
        else {
          ANA_CHECK (m_RNNjetBoosted20GeVHandle.retrieve (RNNJets_boosted_20gev, sys));
          ANA_CHECK (m_RNNjetBoosted30GeVHandle.retrieve (RNNJets_boosted_30gev, sys));
          ANA_CHECK (m_RNNjetResolved20GeVHandle.retrieve (RNNJets_resolved_20gev, sys));
          ANA_CHECK (m_RNNjetResolved30GeVHandle.retrieve (RNNJets_resolved_30gev, sys));
        }

        const xAOD::MuonContainer *muons = nullptr;
        ANA_CHECK (m_muonHandle.retrieve (muons, sys));

        const xAOD::ElectronContainer *electrons = nullptr;
        ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

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
        
        int n_jets = signalJets->size();
        if(!m_UseVBFRNN) n_jets += vbsjets->size();
        
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

        if(!m_UseVBFRNN){
          for(const xAOD::Jet* vbsjet : *vbsjets) {
            if (std::abs(vbsjet->eta())<2.5) nCentralJets++;
            else nForwardJets++;
          }
        }

        int n_bjets = bjets->size();

        m_Ibranches.at("nJets").set(*event, n_jets, sys);
        m_Ibranches.at("nLargeJets").set(*event, n_largeJets, sys);
        m_Ibranches.at("nElectrons").set(*event, n_electrons, sys);
        m_Ibranches.at("nMuons").set(*event, n_muons, sys);
        m_Ibranches.at("nBJets").set(*event, n_bjets, sys);
        m_Ibranches.at("nCentralJets").set(*event, nCentralJets, sys);
        m_Ibranches.at("nForwardJets").set(*event, nForwardJets, sys);

        // selected leptons ;
        const xAOD::Electron* ele0 = nullptr;
        const xAOD::Electron* ele1 = nullptr;

        for(const xAOD::Electron* electron : *electrons) {
          if(!ele0) ele0 = electron;
          else{
            ele1 = electron;
            break;
          }
        }

        const xAOD::Muon* mu0 = nullptr;
        const xAOD::Muon* mu1 = nullptr;
        for(const xAOD::Muon* muon : *muons) {
          if(!mu0) mu0 = muon;
          else{
            mu1 = muon;
            break;
          
          }
        }

        std::vector<std::pair<const xAOD::IParticle*, int>> leptons;
        if(ele0) leptons.emplace_back(ele0, -11*ele0->charge());
        if(mu0) leptons.emplace_back(mu0, -13*mu0->charge());
        if(ele1) leptons.emplace_back(ele1, -11*ele1->charge());
        if(mu1) leptons.emplace_back(mu1, -13*mu1->charge());

        int nLeptons = muons->size() + electrons->size();
        m_Ibranches.at("nLeptons").set(*event, nLeptons, sys);

        std::sort(leptons.begin(), leptons.end(),
            [](const std::pair<const xAOD::IParticle*, int>& a,
               const std::pair<const xAOD::IParticle*, int>& b) {
              return a.first->pt() > b.first->pt(); });

        TLorentzVector metVec;
        metVec.SetPtEtaPhiE(met->met(), 0, met->phi(), met->met());

        for(unsigned int i=0; i<std::min(size_t(2),leptons.size()); i++){
          std::string prefix = "Lepton"+std::to_string(i+1);
          TLorentzVector tlv = leptons[i].first->p4();
          int lep_pdgid = leptons[i].second;
          m_Fbranches.at(prefix+"_pt").set(*event, tlv.Pt(), sys);
          m_Fbranches.at(prefix+"_eta").set(*event, tlv.Eta(), sys);
          m_Fbranches.at(prefix+"_phi").set(*event, tlv.Phi(), sys);
          m_Fbranches.at(prefix+"_E").set(*event, tlv.E(), sys);

          float mt_lept_met = std::sqrt(2 * met->met() * tlv.Pt() * (1 - std::cos(tlv.DeltaPhi(metVec))));
	        m_Fbranches.at(prefix+"_MET"+"_mT").set(*event, mt_lept_met, sys);

          if(m_isMC){
            float SF = -99;
            if(std::abs(leptons[i].second)==11 ){
              SF = m_ele_SF.get(*leptons[i].first,sys);

              int ele_ECIDS = m_eleECIDS.get(*leptons[i].first, sys);
              m_Ibranches.at(prefix+"_ele_ECIDS").set(*event, ele_ECIDS, sys);
            }
            else{
              SF = m_mu_SF.get(*leptons[i].first,sys);
            }
            m_Fbranches.at(prefix+"_effSF").set(*event, SF, sys);
          }
          int charge = leptons[i].second>0 ? -1 : 1;
          m_Ibranches.at(prefix+"_charge").set(*event, charge, sys);
          m_Ibranches.at(prefix+"_pdgid").set(*event, lep_pdgid, sys);
        
          // leptons truth information
          if (m_isMC){
            int lep_truthOrigin = std::abs(lep_pdgid)==11 ?
              m_ele_truthOrigin.get(*leptons[i].first,sys) :
              m_mu_truthOrigin.get(*leptons[i].first,sys);
            m_Ibranches.at(prefix + "_truthOrigin").set(*event, lep_truthOrigin, sys);
            int lep_truthType = std::abs(lep_pdgid)==11 ?
              m_ele_truthType.get(*leptons[i].first,sys) :
              m_mu_truthType.get(*leptons[i].first,sys);
            m_Ibranches.at(prefix + "_truthType").set(*event, lep_truthType, sys);
            int lep_isPrompt = 0;
            if (std::abs(lep_pdgid)==13){ // simplistic
              if (lep_truthType==6) lep_isPrompt=1; // isolated prompts
            } else if (std::abs(lep_pdgid)==11){
              if (lep_truthType==2) lep_isPrompt=1; // isolated prompts
            }
            
            m_Ibranches.at(prefix + "_isPrompt").set(*event, lep_isPrompt, sys);
          }
          
        }

        //MET Significance 
        float METSig = m_METSig.get(*met, sys);
        m_Fbranches.at("METSig").set(*event, METSig, sys);

        // dilepton kinematics
        TLorentzVector ll;
        TLorentzVector Leading_lep;
        TLorentzVector Subleading_lep;     

        if (leptons.size()>0) Leading_lep = leptons[0].first->p4();
        if (leptons.size()>1){
          Subleading_lep = leptons[1].first->p4();
          ll = Leading_lep + Subleading_lep;
        }

        m_Fbranches.at("ll_m").set(*event, ll.M(), sys);
        m_Fbranches.at("ll_pt").set(*event, ll.Pt(), sys);
        m_Fbranches.at("ll_eta").set(*event, ll.Eta(), sys);
        m_Fbranches.at("ll_phi").set(*event, ll.Phi(), sys);

        m_Fbranches.at("dRll").set(*event, Leading_lep.DeltaR(Subleading_lep), sys);
        m_Fbranches.at("dPhill").set(*event, Leading_lep.DeltaPhi(Subleading_lep), sys);
        m_Fbranches.at("dEtall").set(*event, Leading_lep.Eta() - Subleading_lep.Eta(), sys);

        m_Fbranches.at("dPhillMET").set(*event, ll.DeltaPhi(metVec), sys);
        m_Fbranches.at("dPhil1MET").set(*event, Leading_lep.DeltaPhi(metVec), sys);
        m_Fbranches.at("dPhil2MET").set(*event, Subleading_lep.DeltaPhi(metVec), sys);

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

        TLorentzVector HDijet;
        TLorentzVector HJet1;
        TLorentzVector HJet2;
        if (HJets->size() >=2){
          HDijet = HJets->at(0)->p4()+ HJets->at(1)->p4();
          HJet1 = HJets->at(0)->p4();
          HJet2 = HJets->at(1)->p4();
          m_Fbranches.at("Hdijet_m").set(*event, HDijet.M(), sys);
          m_Fbranches.at("Hdijet_pt").set(*event, HDijet.Pt(), sys);
          m_Fbranches.at("Hdijet_eta").set(*event, HDijet.Eta(), sys);
          m_Fbranches.at("Hdijet_phi").set(*event, HDijet.Phi(), sys);
          m_Fbranches.at("dRHjj").set(*event, HJet1.DeltaR(HJet2), sys);
          m_Fbranches.at("dPhiHjj").set(*event, HJet1.DeltaPhi(HJet2), sys);
          m_Fbranches.at("dEtaHjj").set(*event, HJet1.Eta() - HJet2.Eta(), sys);
        }
        
        // leading b-jet + leadinglepton sector
        if (HJets->size()>=2 && nLeptons>=2){
          TLorentzVector HJ1l1 = HJet1 + Leading_lep;
          m_Fbranches.at("Hj1l1_m").set(*event, HJ1l1.M(), sys);
          m_Fbranches.at("Hj1l1_pt").set(*event, HJ1l1.Pt(), sys);
          m_Fbranches.at("Hj1l1_eta").set(*event, HJ1l1.Eta(), sys);
          m_Fbranches.at("Hj1l1_phi").set(*event, HJ1l1.Phi(), sys);
          m_Fbranches.at("dRHj1l1").set(*event, HJet1.DeltaR(Leading_lep), sys);
          m_Fbranches.at("dPhiHj1l1").set(*event, HJet1.DeltaPhi(Leading_lep), sys);
          m_Fbranches.at("dEtaHj1l1").set(*event, HJet1.Eta() - Leading_lep.Eta(), sys);
        
          TLorentzVector HJ2l2 = HJet2 + Subleading_lep;
          m_Fbranches.at("Hj2l2_m").set(*event, HJ2l2.M(), sys);
          m_Fbranches.at("Hj2l2_pt").set(*event, HJ2l2.Pt(), sys);
          m_Fbranches.at("Hj2l2_eta").set(*event, HJ2l2.Eta(), sys);
          m_Fbranches.at("Hj2l2_phi").set(*event, HJ2l2.Phi(), sys);
          m_Fbranches.at("dRHj2l2").set(*event, HJet2.DeltaR(Subleading_lep), sys);
          m_Fbranches.at("dPhiHj2l2").set(*event, HJet2.DeltaPhi(Subleading_lep), sys);
          m_Fbranches.at("dEtaHj2l2").set(*event, HJet2.Eta() - Subleading_lep.Eta(), sys);

          TLorentzVector Hjjll = ll + HDijet;
          TLorentzVector Hjjllmet = ll + HDijet + metVec;
          m_Fbranches.at("Hdijetll_m").set(*event, Hjjll.M(), sys);
          m_Fbranches.at("Hdijetllmet_m").set(*event, Hjjllmet.M(), sys);

          double ht2 = (metVec + ll).Perp() + HDijet.Perp();
          double ht2r = ht2 / (met->met() + Leading_lep.Pt() + Subleading_lep.Pt() + HJet1.Pt() + HJet2.Pt());

          m_Fbranches.at("HT2").set(*event, ht2, sys);
          m_Fbranches.at("HT2r").set(*event, ht2r, sys);

        }

        //min Delta R (bjet, lepton)
        std::vector<double> deltaRs;
        for(unsigned int i=0; i<std::min(size_t(2),leptons.size()); i++){
          TLorentzVector tlv = leptons[i].first->p4();
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
        if(!m_UseVBFRNN){
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
            m_Fbranches.at("dRVBSjj").set(*event, (vbsJet1->p4()).DeltaR(vbsJet2->p4()), sys);
            m_Fbranches.at("dEtaVBSjj").set(*event, (vbsJet1->eta())-vbsJet2->eta(), sys);
            m_Fbranches.at("dPhiVBSjj").set(*event, (vbsJet1->p4()).DeltaPhi(vbsJet2->p4()), sys);
            
          }
        }

        //kinematics of RNN jets
        else {
          std::vector<const xAOD::JetContainer*> RNNJets = {RNNJets_boosted_20gev,RNNJets_boosted_30gev,RNNJets_resolved_20gev,RNNJets_resolved_30gev};
          std::vector<std::string> RNNJets_names = {"RNNJets_boosted_20gev","RNNJets_boosted_30gev","RNNJets_resolved_20gev","RNNJets_resolved_30gev"};
          for(unsigned int i=0; i<RNNJets.size(); i++) {
            const xAOD::JetContainer *RNNJets_container = RNNJets[i];
            std::string RNNJets_container_name = RNNJets_names[i];

            for(unsigned int j=0; j<std::min(size_t(2),RNNJets_container->size()); j++) {
              std::string prefix = "Jet"+std::to_string(j+1);
              const xAOD::Jet* RNNJet = RNNJets_container->at(0);

              m_Fbranches.at(RNNJets_container_name+"_"+prefix+"_m").set(*event, RNNJet->m(), sys);
              m_Fbranches.at(RNNJets_container_name+"_"+prefix+"_pt").set(*event, RNNJet->pt(), sys);
              m_Fbranches.at(RNNJets_container_name+"_"+prefix+"_eta").set(*event, RNNJet->eta(), sys);
              m_Fbranches.at(RNNJets_container_name+"_"+prefix+"_phi").set(*event, RNNJet->phi(), sys);
            }
          }
        }
      }

      return StatusCode::SUCCESS;
    }

}
