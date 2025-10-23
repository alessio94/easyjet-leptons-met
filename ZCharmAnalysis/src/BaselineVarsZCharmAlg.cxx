/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "BaselineVarsZCharmAlg.h"

#include "AthContainers/AuxElement.h"
#include <AthContainers/ConstDataVector.h>

#include "TLorentzVector.h"

namespace ZCC
{
  BaselineVarsZCharmAlg::BaselineVarsZCharmAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {

  }

  StatusCode BaselineVarsZCharmAlg::initialize()
  {
    // Read syst-aware input handles
    ATH_CHECK (m_ZCharmJetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_ZCharmLRJetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_ZCharmElectronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_ZCharmMuonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    if(m_isMC){
      ATH_CHECK (m_ele_truthOrigin.initialize(m_systematicsList, m_ZCharmElectronHandle));
      ATH_CHECK (m_ele_truthType.initialize(m_systematicsList, m_ZCharmElectronHandle));
      ATH_CHECK (m_mu_truthOrigin.initialize(m_systematicsList, m_ZCharmMuonHandle));
      ATH_CHECK (m_mu_truthType.initialize(m_systematicsList, m_ZCharmMuonHandle));

      ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
      m_ele_SF = CP::SysReadDecorHandle<float>("effSF_"+m_eleWPName+"_%SYS%", this);
      ATH_CHECK (m_ele_SF.initialize(m_systematicsList, m_electronHandle));

      ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
      m_mu_SF = CP::SysReadDecorHandle<float>("effSF_"+m_muWPName+"_%SYS%", this);
      ATH_CHECK (m_mu_SF.initialize(m_systematicsList, m_muonHandle));
    }

    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_ZCharmJetHandle));
    }

    for (const std::string &var : m_PCBTnames){
      ATH_MSG_DEBUG("initializing PCBT: " << var);
      CP::SysReadDecorHandle<int> rhandle{var, this};
      m_PCBTs.emplace(var, rhandle);
      ATH_CHECK(m_PCBTs.at(var).initialize(m_systematicsList, m_ZCharmJetHandle));
    };


    if (m_isMC) {
      ATH_CHECK (m_truthFlav.initialize(m_systematicsList, m_ZCharmJetHandle));
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
    
    ATH_CHECK (m_GN2Xv01_phbb.initialize(m_systematicsList, m_ZCharmLRJetHandle));
    ATH_CHECK (m_GN2Xv01_phcc.initialize(m_systematicsList, m_ZCharmLRJetHandle));
    ATH_CHECK (m_GN2Xv01_pqcd.initialize(m_systematicsList, m_ZCharmLRJetHandle));
    ATH_CHECK (m_GN2Xv01_ptop.initialize(m_systematicsList, m_ZCharmLRJetHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsZCharmAlg::execute()
  {

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {

      // Retrieve inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_ZCharmJetHandle.retrieve (jets, sys));

      const xAOD::JetContainer *largeJets  = nullptr;
      ANA_CHECK (m_ZCharmLRJetHandle.retrieve (largeJets , sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_ZCharmMuonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_ZCharmElectronHandle.retrieve (electrons, sys));


      for (const std::string &string_var: m_floatVariables) {
        m_Fbranches.at(string_var).set(*event, -99., sys);
      }

      for (const auto& var: m_intVariables) {
        m_Ibranches.at(var).set(*event, -99, sys);
      }

      // Count leptons
      int n_electrons = electrons->size();
      int n_muons = muons->size();

      // Count jets
      int n_jets = jets->size();

      // b-jet sector
      std::string ftag2D_WP = "ftag_quantile_GN2v01_Continuous2D";
      std::vector<int> btag_values = {4,5,6};
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      for(const xAOD::Jet* jet : *jets) {
          int pcbt = m_PCBTs.at(ftag2D_WP).get(*jet, sys);
          if (std::find(btag_values.begin(), btag_values.end(), pcbt)!=btag_values.end() && std::abs(jet->eta())<2.5) {
            bjets->push_back(jet);
          }
      }
      int n_bjets = bjets->size();

      // c-jet sector
      std::vector<int> ctag_values = {1,2,3};
      auto cjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      if(std::find(m_PCBTnames.begin(), m_PCBTnames.end(), ftag2D_WP)!=m_PCBTnames.end())
      {
        for(const xAOD::Jet* jet : *jets) {
          int pcbt = m_PCBTs.at(ftag2D_WP).get(*jet, sys);
          if (std::find(ctag_values.begin(), ctag_values.end(), pcbt)!=ctag_values.end() && std::abs(jet->eta())<2.5) {
            cjets->push_back(jet);
          }
        }
      }
      int n_cjets = cjets->size();

      m_Ibranches.at("nElectrons").set(*event, n_electrons, sys);
      m_Ibranches.at("nMuons").set(*event, n_muons, sys);
      m_Ibranches.at("nJets").set(*event, n_jets, sys);
      m_Ibranches.at("nBJets").set(*event, n_bjets, sys);
      m_Ibranches.at("nCJets").set(*event, n_cjets, sys);

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

      int nLeptons = leptons.size(); // Do not to use nLeptons==2 as a requirement, the events possibly have more than 2 leptons 
      m_Ibranches.at("nLeptons").set(*event, nLeptons, sys);

      std::sort(leptons.begin(), leptons.end(),
          [](const std::pair<const xAOD::IParticle*, int>& a,
              const std::pair<const xAOD::IParticle*, int>& b) {
            return a.first->pt() > b.first->pt(); });

      for(int i=0; i<std::min(nLeptons, 2); i++){
        std::string prefix = "Lepton"+std::to_string(i+1);
        TLorentzVector tlv = leptons[i].first->p4();
        int lep_pdgid = leptons[i].second;
        m_Fbranches.at(prefix+"_pt").set(*event, tlv.Pt(), sys);
        m_Fbranches.at(prefix+"_eta").set(*event, tlv.Eta(), sys);
        m_Fbranches.at(prefix+"_phi").set(*event, tlv.Phi(), sys);
        m_Fbranches.at(prefix+"_E").set(*event, tlv.E(), sys);
        if(m_isMC){
          float SF = 1.;
          if(std::abs(lep_pdgid)==13)
            SF = m_mu_SF.get(*leptons[i].first,sys);
          else
            SF = m_ele_SF.get(*leptons[i].first,sys);
          m_Fbranches.at(prefix+"_effSF").set(*event, SF, sys);
        }
        int charge = lep_pdgid > 0 ? -1 : 1;
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


      // dilepton kinematics
      TLorentzVector ll;
      TLorentzVector Leading_lep;
      TLorentzVector Subleading_lep;

      if (nLeptons >= 1) Leading_lep = leptons[0].first->p4();
      if (nLeptons >= 2){

        Subleading_lep = leptons[1].first->p4();
        ll = Leading_lep + Subleading_lep;

        m_Fbranches.at("mll").set(*event, ll.M(), sys);
        m_Fbranches.at("pTll").set(*event, ll.Pt(), sys);
        m_Fbranches.at("Etall").set(*event, ll.Eta(), sys);
        m_Fbranches.at("Phill").set(*event, ll.Phi(), sys);

        m_Fbranches.at("dRll").set(*event, Leading_lep.DeltaR(Subleading_lep), sys);
        m_Fbranches.at("dPhill").set(*event, Leading_lep.DeltaPhi(Subleading_lep), sys);
        m_Fbranches.at("dEtall").set(*event, Leading_lep.Eta() - Subleading_lep.Eta(), sys);
      }

      //jet sector
      if(n_jets){
        m_Fbranches.at("dRZj").set(*event, ll.DeltaR(jets->at(0)->p4()), sys);
        m_Fbranches.at("dPhiZj").set(*event, ll.DeltaPhi(jets->at(0)->p4()), sys);
      }

      for (int i=0; i<std::min(n_jets,2); i++){
        std::string prefix = "Jet"+std::to_string(i+1);
        m_Fbranches.at(prefix+"_pt").set(*event, jets->at(i)->pt(), sys);
        m_Fbranches.at(prefix+"_eta").set(*event, jets->at(i)->eta(), sys);
        m_Fbranches.at(prefix+"_phi").set(*event, jets->at(i)->phi(), sys);
        m_Fbranches.at(prefix+"_E").set(*event, jets->at(i)->e(), sys); 
      }
      if (n_jets >=2){
        TLorentzVector jj;
        jj = jets->at(0)->p4()+jets->at(1)->p4();
        m_Fbranches.at("mjj").set(*event, jj.M(), sys);
        m_Fbranches.at("pTjj").set(*event, jj.Pt(), sys);
        m_Fbranches.at("Etajj").set(*event, jj.Eta(), sys);
        m_Fbranches.at("Phijj").set(*event, jj.Phi(), sys);
        m_Fbranches.at("dRjj").set(*event, (jets->at(0)->p4()).DeltaR(jets->at(1)->p4()), sys);
        m_Fbranches.at("dEtajj").set(*event, (jets->at(0)->eta())-(jets->at(1)->eta()), sys);
        m_Fbranches.at("dPhijj").set(*event, (jets->at(0)->p4()).DeltaPhi(jets->at(1)->p4()), sys);
      }

      //b-jet sector      
      if(n_bjets){
        m_Fbranches.at("dRZb").set(*event, ll.DeltaR(bjets->at(0)->p4()), sys);
        m_Fbranches.at("dPhiZb").set(*event, ll.DeltaPhi(bjets->at(0)->p4()), sys);
      }

      for (int i=0; i<std::min(n_bjets, 2); i++){
        std::string prefix = "Jet_b"+std::to_string(i+1);
        m_Fbranches.at(prefix+"_pt").set(*event, bjets->at(i)->pt(), sys);
        m_Fbranches.at(prefix+"_eta").set(*event, bjets->at(i)->eta(), sys);
        m_Fbranches.at(prefix+"_phi").set(*event, bjets->at(i)->phi(), sys);
        m_Fbranches.at(prefix+"_E").set(*event, bjets->at(i)->e(), sys);
        if (m_isMC) {
          m_Ibranches.at(prefix+"_truthLabel").set(*event, m_truthFlav.get(*bjets->at(i), sys), sys);
        }
        for (const auto& var: m_PCBTnames) {
          std::string new_var = var;
          new_var.erase(0, 14); // remove 'ftag_quantile_' from var name
          if(new_var.find("Continuous2D")!=std::string::npos){
            new_var.erase(new_var.length() - 13, new_var.length()); // remove '_Continuous2D' from var name
            m_Ibranches.at(prefix+"_pcbt2D_"+new_var).set(*event, m_PCBTs.at(var).get(*bjets->at(i), sys), sys);
          }
          else{
            new_var.erase(new_var.length() - 11, new_var.length()); // remove '_Continuous' from var name
            m_Ibranches.at(prefix+"_pcbt_"+new_var).set(*event, m_PCBTs.at(var).get(*bjets->at(i), sys), sys);
          }
        }
      }
      if (n_bjets >=2){
        TLorentzVector bb = bjets->at(0)->p4()+bjets->at(1)->p4();
        m_Fbranches.at("mbb").set(*event, bb.M(), sys);
        m_Fbranches.at("pT_over_mbb").set(*event, (bb.Pt())/(bb.M()), sys);
        m_Fbranches.at("pTbb").set(*event, bb.Pt(), sys);
        m_Fbranches.at("Etabb").set(*event, bb.Eta(), sys);
        m_Fbranches.at("Phibb").set(*event, bb.Phi(), sys);
        m_Fbranches.at("dRbb").set(*event, (bjets->at(0)->p4()).DeltaR(bjets->at(1)->p4()), sys);
        m_Fbranches.at("dPhibb").set(*event, (bjets->at(0)->p4()).DeltaPhi(bjets->at(1)->p4()), sys);
        m_Fbranches.at("dEtabb").set(*event, (bjets->at(0)->eta()) - bjets->at(1)->eta(), sys);
      }

      //c-jet sector
      if(n_cjets){
        m_Fbranches.at("dRZc").set(*event, ll.DeltaR(cjets->at(0)->p4()), sys);
        m_Fbranches.at("dPhiZc").set(*event, ll.DeltaPhi(cjets->at(0)->p4()), sys);
      }

      for (int i=0; i<std::min(n_cjets, 2); i++){
        std::string prefix = "Jet_c"+std::to_string(i+1);
        m_Fbranches.at(prefix+"_pt").set(*event, cjets->at(i)->pt(), sys);
        m_Fbranches.at(prefix+"_eta").set(*event, cjets->at(i)->eta(), sys);
        m_Fbranches.at(prefix+"_phi").set(*event, cjets->at(i)->phi(), sys);
        m_Fbranches.at(prefix+"_E").set(*event, cjets->at(i)->e(), sys);
        if (m_isMC) {
          m_Ibranches.at(prefix+"_truthLabel").set(*event, m_truthFlav.get(*cjets->at(i), sys), sys);
        }
        for (const auto& var: m_PCBTnames) {
          std::string new_var = var;
          new_var.erase(0, 14); // remove 'ftag_quantile_' from var name
          if(new_var.find("Continuous2D")!=std::string::npos){
            new_var.erase(new_var.length() - 13, new_var.length()); // remove '_Continuous2D' from var name
            m_Ibranches.at(prefix+"_pcbt2D_"+new_var).set(*event, m_PCBTs.at(var).get(*cjets->at(i), sys), sys);
          }
          else{
            new_var.erase(new_var.length() - 11, new_var.length()); // remove '_Continuous' from var name
            m_Ibranches.at(prefix+"_pcbt_"+new_var).set(*event, m_PCBTs.at(var).get(*cjets->at(i), sys), sys);
          }
        }
      }
      if (n_cjets >=2){
        int cjet0_val = m_PCBTs.at(ftag2D_WP).get(*cjets->at(0), sys);
        int cjet1_val = m_PCBTs.at(ftag2D_WP).get(*cjets->at(1), sys);
        int PCFT2D2jets = 0;
  
          for (int i = 1; i <= 3; i++) {
            for (int j = 1; j <= i; j++) {  // Only consider the case where j <= i.
                PCFT2D2jets++;
                if ((cjet0_val == i && cjet1_val == j) || (cjet0_val == j && cjet1_val == i)) {
                    m_Ibranches.at("PCFT2D2jets").set(*event, PCFT2D2jets, sys);
                }
            }
        }
  
        TLorentzVector cc = cjets->at(0)->p4()+cjets->at(1)->p4();
        m_Fbranches.at("mcc").set(*event, cc.M(), sys);
        m_Fbranches.at("pT_over_mcc").set(*event, (cc.Pt())/(cc.M()), sys);
        m_Fbranches.at("pTcc").set(*event, cc.Pt(), sys);
        m_Fbranches.at("Etacc").set(*event, cc.Eta(), sys);
        m_Fbranches.at("Phicc").set(*event, cc.Phi(), sys);
        m_Fbranches.at("dRcc").set(*event, (cjets->at(0)->p4()).DeltaR(cjets->at(1)->p4()), sys);
        m_Fbranches.at("dPhicc").set(*event, (cjets->at(0)->p4()).DeltaPhi(cjets->at(1)->p4()), sys);
        m_Fbranches.at("dEtacc").set(*event, (cjets->at(0)->eta()) - cjets->at(1)->eta(), sys);
      }

      // large jet sector
      for (std::size_t i=0; i<std::min(largeJets ->size(),(std::size_t)1); i++){

        const xAOD::Jet* largeJet = largeJets->at(i);
        std::string prefix = "LargeRJet"+std::to_string(i+1);

        // calculate Xbb/cc score
        float phbb = m_GN2Xv01_phbb.get(*largeJet, sys);
        float phcc = m_GN2Xv01_phcc.get(*largeJet, sys);
        float pqcd = m_GN2Xv01_pqcd.get(*largeJet, sys);
        float ptop = m_GN2Xv01_ptop.get(*largeJet, sys);
        float fbb = 0.25;
        float ftop = 0.25;
        float XccScore= log (phcc / (fbb*phbb + ftop*ptop + pqcd*(1-fbb-ftop)));

        m_Fbranches.at(prefix+"_pt").set(*event, largeJet->pt(), sys);
        m_Fbranches.at(prefix+"_eta").set(*event, largeJet->eta(), sys);
        m_Fbranches.at(prefix+"_phi").set(*event, largeJet->phi(), sys);
        m_Fbranches.at(prefix+"_E").set(*event, largeJet->e(), sys);
        m_Fbranches.at(prefix+"_m").set(*event, largeJet->m(), sys);
        
        m_Fbranches.at(prefix+"_GN2Xv01_phbb").set(*event, phbb, sys);
        m_Fbranches.at(prefix+"_GN2Xv01_phcc").set(*event, phcc, sys);
        m_Fbranches.at(prefix+"_GN2Xv01_pqcd").set(*event, pqcd, sys);
        m_Fbranches.at(prefix+"_GN2Xv01_ptop").set(*event, ptop, sys);
        m_Fbranches.at(prefix+"_GN2Xv01_DXcc").set(*event, XccScore, sys);

      }
      m_Ibranches.at("nLargeRJets").set(*event, largeJets ->size(), sys);

    }
    return StatusCode::SUCCESS;
  }

}


