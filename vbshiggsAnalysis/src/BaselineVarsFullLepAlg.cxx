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
      ATH_CHECK (m_signaljetHandle.initialize(m_systematicsList));
      ATH_CHECK (m_vbsjetHandle.initialize(m_systematicsList));
      ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
      ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
      ATH_CHECK (m_metHandle.initialize(m_systematicsList));
      ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

      if(m_isMC){
        m_ele_SF = CP::SysReadDecorHandle<float>("el_effSF_"+m_eleWPName+"_%SYS%", this);
      }
      ATH_CHECK (m_ele_SF.initialize(m_systematicsList, m_electronHandle, SG::AllowEmpty));

      if(m_isMC){
        m_mu_SF = CP::SysReadDecorHandle<float>("muon_effSF_"+m_muWPName+"_%SYS%", this);
      }
      ATH_CHECK (m_mu_SF.initialize(m_systematicsList, m_muonHandle, SG::AllowEmpty));

      if (!m_isBtag.empty()) {
        ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_signaljetHandle));
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
        ATH_CHECK (m_truthFlav.initialize(m_systematicsList, m_signaljetHandle));
      }

      ATH_CHECK (m_eleECIDS.initialize(m_systematicsList, m_electronHandle));
      
      ATH_CHECK (m_METSig.initialize(m_systematicsList, m_metHandle));
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

        const xAOD::JetContainer *signalJets = nullptr;
        ANA_CHECK (m_signaljetHandle.retrieve (signalJets, sys));

        const xAOD::JetContainer *vbsjets = nullptr;
        ANA_CHECK (m_vbsjetHandle.retrieve (vbsjets, sys));

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
        
        int n_jets = signalJets->size() + vbsjets->size();
        int nCentralJets = 0;
        int nForwardJets = 0;

        int n_electrons = electrons->size();
        int n_muons = muons->size();

        // b-jet sector
        bool WPgiven = !m_isBtag.empty();
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

        int nLeptons = leptons.size();
        m_Ibranches.at("nLeptons").set(*event, nLeptons, sys);

        std::sort(leptons.begin(), leptons.end(),
            [](const std::pair<const xAOD::IParticle*, int>& a,
               const std::pair<const xAOD::IParticle*, int>& b) {
              return a.first->pt() > b.first->pt(); });

        for(unsigned int i=0; i<std::min(size_t(2),leptons.size()); i++){
          std::string prefix = "Lepton"+std::to_string(i+1);
          TLorentzVector tlv = leptons[i].first->p4();
          int lep_pdgid = leptons[i].second;
          m_Fbranches.at(prefix+"_pt").set(*event, tlv.Pt(), sys);
          m_Fbranches.at(prefix+"_eta").set(*event, tlv.Eta(), sys);
          m_Fbranches.at(prefix+"_phi").set(*event, tlv.Phi(), sys);
          m_Fbranches.at(prefix+"_E").set(*event, tlv.E(), sys);
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
            auto [lep_truthOrigin, lep_truthType] = truthOrigin(leptons[i].first);
            m_Ibranches.at(prefix + "_truthOrigin").set(*event, lep_truthOrigin, sys);
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

        // dilepton kinematics
        TLorentzVector ll;
        TLorentzVector Leading_lep;
        TLorentzVector Subleading_lep;
        TVector3 metVec;
        metVec.SetPtEtaPhi(met->met(), 0, met->phi());

        //MET Significance 
        float METSig = m_METSig.get(*met, sys);
        m_Fbranches.at("METSig").set(*event, METSig, sys);

        // ee
        if (ele0 && ele1){
          ll = ele0->p4() + ele1->p4();
        } 
        //mumu
        if (mu0 && mu1){
          ll = mu0->p4() + mu1->p4();
        }
        //emu
        if (ele0 && mu0){
          ll = electrons->at(0)->p4() + muons->at(0)->p4(); 
        }
        m_Fbranches.at("mll").set(*event, ll.M(), sys);
        m_Fbranches.at("pTll").set(*event, ll.Pt(), sys);
        m_Fbranches.at("Etall").set(*event, ll.Eta(), sys);
        m_Fbranches.at("Phill").set(*event, ll.Phi(), sys);
        
        if (leptons.size() > 2){
          Leading_lep = leptons[0].first->p4();
          Subleading_lep = leptons[1].first->p4();
        }

        m_Fbranches.at("dRll").set(*event, Leading_lep.DeltaR(Subleading_lep), sys);
        m_Fbranches.at("dPhill").set(*event, Leading_lep.DeltaPhi(Subleading_lep), sys);
        m_Fbranches.at("dEtall").set(*event, Leading_lep.Eta() - Subleading_lep.Eta(), sys);

        m_Fbranches.at("dPhillMET").set(*event, ll.Vect().DeltaPhi(metVec), sys);
        m_Fbranches.at("dPhil1MET").set(*event, Leading_lep.Vect().DeltaPhi(metVec), sys);
        m_Fbranches.at("dPhil2MET").set(*event, Subleading_lep.Vect().DeltaPhi(metVec), sys);

        //jet sector
        for (std::size_t i=0; i<std::min(signalJets->size(),(std::size_t)2); i++){
          m_Fbranches.at("Jet"+std::to_string(i+1)+"_pt").set(*event, signalJets->at(i)->pt(), sys);
          m_Fbranches.at("Jet"+std::to_string(i+1)+"_eta").set(*event, signalJets->at(i)->eta(), sys);
          m_Fbranches.at("Jet"+std::to_string(i+1)+"_phi").set(*event, signalJets->at(i)->phi(), sys);
          m_Fbranches.at("Jet"+std::to_string(i+1)+"_E").set(*event, signalJets->at(i)->e(), sys); 
        }

        //b-jet sector
        for (std::size_t i=0; i<std::min(bjets->size(),(std::size_t)2); i++){
          m_Fbranches.at("Jet_b"+std::to_string(i+1)+"_pt").set(*event, bjets->at(i)->pt(), sys);
          m_Fbranches.at("Jet_b"+std::to_string(i+1)+"_eta").set(*event, bjets->at(i)->eta(), sys);
          m_Fbranches.at("Jet_b"+std::to_string(i+1)+"_phi").set(*event, bjets->at(i)->phi(), sys);
          m_Fbranches.at("Jet_b"+std::to_string(i+1)+"_E").set(*event, bjets->at(i)->e(), sys);
          if (m_isMC) {
            m_Ibranches.at("Jet_b"+std::to_string(i+1)+"_truthLabel").set(*event, m_truthFlav.get(*bjets->at(i), sys), sys);
          }
        }
        if (bjets->size() >=2){
          TLorentzVector bb = bjets->at(0)->p4()+bjets->at(1)->p4();
          m_Fbranches.at("mbb").set(*event, bb.M(), sys);
          m_Fbranches.at("pTbb").set(*event, bb.Pt(), sys);
          m_Fbranches.at("Etabb").set(*event, bb.Eta(), sys);
          m_Fbranches.at("Phibb").set(*event, bb.Phi(), sys);
          m_Fbranches.at("dRbb").set(*event, (bjets->at(0)->p4()).DeltaR(bjets->at(1)->p4()), sys);
          m_Fbranches.at("dPhibb").set(*event, (bjets->at(0)->p4()).DeltaPhi(bjets->at(1)->p4()), sys);
          m_Fbranches.at("dEtabb").set(*event, (bjets->at(0)->eta()) - bjets->at(1)->eta(), sys);
        }
        
        // leading b-jet + leadinglepton sector
        if (n_bjets>=1 && nLeptons>=1){
          TLorentzVector bl = bjets->at(0)->p4()+Leading_lep;
          m_Fbranches.at("mb1l1").set(*event, bl.M(), sys);
          m_Fbranches.at("pTb1l1").set(*event, bl.Pt(), sys);
          m_Fbranches.at("Etab1l1").set(*event, bl.Eta(), sys);
          m_Fbranches.at("Phib1l1").set(*event, bl.Phi(), sys);
          m_Fbranches.at("dRb1l1").set(*event, (bjets->at(0)->p4()).DeltaR(Leading_lep), sys);
          m_Fbranches.at("dPhib1l1").set(*event, (bjets->at(0)->p4()).DeltaPhi(Leading_lep), sys);
          m_Fbranches.at("dEtab1l1").set(*event, (bjets->at(0)->eta()) - Leading_lep.Eta(), sys);
        }

        // subleading b-jet + subleading lepton sector
        if (n_bjets>=2 && nLeptons>=2){
          TLorentzVector bl = bjets->at(1)->p4() + Subleading_lep;
          m_Fbranches.at("mb2l2").set(*event, bl.M(), sys);
          m_Fbranches.at("pTb2l2").set(*event, bl.Pt(), sys);
          m_Fbranches.at("Etab2l2").set(*event, bl.Eta(), sys);
          m_Fbranches.at("Phib2l2").set(*event, bl.Phi(), sys);
          m_Fbranches.at("dRb2l2").set(*event, (bjets->at(1)->p4()).DeltaR(Subleading_lep), sys);
          m_Fbranches.at("dPhib2l2").set(*event, (bjets->at(1)->p4()).DeltaPhi(Subleading_lep), sys);
          m_Fbranches.at("dEtab2l2").set(*event, (bjets->at(1)->eta()) - Subleading_lep.Eta(), sys);
        }

        // kinematics of vbs jets
        if (vbsjets->size() >=2){
          const xAOD::Jet* vbsJet1 = vbsjets->at(0);
          const xAOD::Jet* vbsJet2 = vbsjets->at(1);

          TLorentzVector vbs_jj = vbsJet1->p4() + vbsJet2->p4();
          
          m_Fbranches.at("mjj").set(*event, vbs_jj.M(), sys);
          m_Fbranches.at("pTjj").set(*event, vbs_jj.Pt(), sys);
          m_Fbranches.at("Etajj").set(*event, vbs_jj.Eta(), sys);
          m_Fbranches.at("Phijj").set(*event, vbs_jj.Phi(), sys);
          m_Fbranches.at("dRjj").set(*event, (vbsJet1->p4()).DeltaR(vbsJet2->p4()), sys);
          m_Fbranches.at("dEtajj").set(*event, (vbsJet1->eta())-vbsJet2->eta(), sys);
          m_Fbranches.at("dPhijj").set(*event, (vbsJet1->p4()).DeltaPhi(vbsJet2->p4()), sys);
          
        }
      }

      return StatusCode::SUCCESS;
    } 
    template<typename ParticleType>
    std::pair<int, int> BaselineVarsFullLepAlg::truthOrigin(const ParticleType* particle) {
      static const SG::AuxElement::ConstAccessor<int> lepttruthOrigin("truthOrigin");
      static const SG::AuxElement::ConstAccessor<int> lepttruthType("truthType");
    
      return {lepttruthOrigin(*particle), lepttruthType(*particle)};
  }

}