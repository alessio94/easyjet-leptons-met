/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "BaselineVarsssWWAlg.h"

#include "AthContainers/AuxElement.h"
#include <AthContainers/ConstDataVector.h>

#include "TLorentzVector.h"
#include "EasyjetHub/MT2_ROOT.h"

namespace ssWWVBS
{
  BaselineVarsssWWAlg::BaselineVarsssWWAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {

  }

  StatusCode BaselineVarsssWWAlg::initialize()
  {
    // Read syst-aware input handles
    ATH_CHECK (m_ssWWJetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_ssWWElectronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_ssWWMuonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    if(m_isMC){
      ATH_CHECK (m_ele_truthOrigin.initialize(m_systematicsList, m_ssWWElectronHandle));
      ATH_CHECK (m_ele_truthType.initialize(m_systematicsList, m_ssWWElectronHandle));

      ATH_CHECK (m_mu_truthOrigin.initialize(m_systematicsList, m_ssWWMuonHandle));
      ATH_CHECK (m_mu_truthType.initialize(m_systematicsList, m_ssWWMuonHandle));

      // SF access
      if(!m_saveDummy_ele_SF){
        ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
        m_ele_SF = CP::SysReadDecorHandle<float>("effSF_"+m_eleWPName+"_%SYS%", this);
        ATH_CHECK (m_ele_SF.initialize(m_systematicsList, m_electronHandle));
      }

      ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
      m_mu_SF = CP::SysReadDecorHandle<float>("effSF_"+m_muWPName+"_%SYS%", this);
      ATH_CHECK (m_mu_SF.initialize(m_systematicsList, m_muonHandle, SG::AllowEmpty));
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

    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_ssWWJetHandle));
    }

    ATH_CHECK (m_METSig.initialize(m_systematicsList, m_metHandle));

    if (m_isMC) {
      ATH_CHECK (m_truthFlav.initialize(m_systematicsList, m_ssWWJetHandle));
    }

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsssWWAlg::execute()
  {

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {

      // Retrieve inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_ssWWJetHandle.retrieve (jets, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_ssWWMuonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_ssWWElectronHandle.retrieve (electrons, sys));

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

      // Count leptons
      int n_electrons = electrons->size();
      int n_muons = muons->size();

      // Count jets
      int n_jets = jets->size();
      int nCentralJets = 0;
      int nForwardJets = 0;

      // b-jet sector
      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      auto nonbjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);;
      for(const xAOD::Jet* jet : *jets) {
        if (std::abs(jet->eta())<2.5) nCentralJets++; 
        else nForwardJets++;

        if (WPgiven) {
          if (m_isBtag.get(*jet, sys) && std::abs(jet->eta())<2.5) bjets->push_back(jet);
          else nonbjets->push_back(jet);
        }
      }
      int n_bjets = bjets->size();

      m_Ibranches.at("nElectrons").set(*event, n_electrons, sys);
      m_Ibranches.at("nMuons").set(*event, n_muons, sys);
      m_Ibranches.at("nJets").set(*event, n_jets, sys);
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
          else if(!m_saveDummy_ele_SF)
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

      // MET
      TVector3 metVec3;
      metVec3.SetPtEtaPhi(met->met(), 0, met->phi());
      TLorentzVector met_tlv;
      // Assumes m=0, wrong for final states with several neutrinos
      met_tlv.SetVectM(metVec3, 0);

      //MET Significance 
      float METSig = m_METSig.get(*met, sys);
      m_Fbranches.at("METSig").set(*event, METSig, sys);

      // dilepton kinematics
      TLorentzVector ll;
      TLorentzVector Leading_lep;
      TLorentzVector Subleading_lep;
      if (nLeptons >=1) Leading_lep = leptons[0].first->p4();
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

        m_Fbranches.at("dPhillMET").set(*event, ll.Vect().DeltaPhi(metVec3), sys);
        m_Fbranches.at("dPhil1MET").set(*event, Leading_lep.Vect().DeltaPhi(metVec3), sys);
        m_Fbranches.at("dPhil2MET").set(*event, Subleading_lep.Vect().DeltaPhi(metVec3), sys);
        // MET Variables;
        float mT = sqrt((abs(ll.Pt()) + met->met()) * (abs(ll.Pt()) + met->met()) - (ll.Pt() * cos(ll.Phi()) + met->met() * cos(met->phi())) * (ll.Pt() * cos(ll.Phi()) + met->met() * cos(met->phi())) - (ll.Pt() * sin(ll.Phi()) + met->met() * sin(met->phi())) * (ll.Pt() * sin(ll.Phi()) + met->met() * sin(met->phi())));
        m_Fbranches.at("mT").set(*event, mT, sys);
        m_Fbranches.at("MET_met").set(*event, met->met(), sys);
        m_Fbranches.at("MET_phi").set(*event, met->phi(), sys);
      }

      //jet sector
      TLorentzVector Leading_jet;
      TLorentzVector Subleading_jet;

      for (int i=0; i<std::min(n_jets,2); i++){
        if(i==0) Leading_jet = jets->at(i)->p4();
        else if(i==1) Subleading_jet = jets->at(i)->p4();
        m_Fbranches.at("Jet"+std::to_string(i+1)+"_pt").set(*event, jets->at(i)->pt(), sys);
        m_Fbranches.at("Jet"+std::to_string(i+1)+"_eta").set(*event, jets->at(i)->eta(), sys);
        m_Fbranches.at("Jet"+std::to_string(i+1)+"_phi").set(*event, jets->at(i)->phi(), sys);
        m_Fbranches.at("Jet"+std::to_string(i+1)+"_E").set(*event, jets->at(i)->e(), sys); 
      }

      TLorentzVector jj;
      if (n_jets >=2){
        jj = Leading_jet + Subleading_jet;
        m_Fbranches.at("mjj").set(*event, jj.M(), sys);
        m_Fbranches.at("pTjj").set(*event, jj.Pt(), sys);
        m_Fbranches.at("Etajj").set(*event, jj.Eta(), sys);
        m_Fbranches.at("Phijj").set(*event, jj.Phi(), sys);
        m_Fbranches.at("dRjj").set(*event, (jets->at(0)->p4()).DeltaR(jets->at(1)->p4()), sys);
        m_Fbranches.at("dEtajj").set(*event, (jets->at(0)->eta())-(jets->at(1)->eta()), sys);
        m_Fbranches.at("dPhijj").set(*event, (jets->at(0)->p4()).DeltaPhi(jets->at(1)->p4()), sys);
      }
      // Zeppenfeld variable
      TLorentzVector Subsubleading_jet;
      if (n_jets >=3)
      {
        Subsubleading_jet = jets->at(2)->p4();
        float epsilon_j3 = abs((Subsubleading_jet.Eta() - 0.5 * (Leading_jet.Eta() + Subleading_jet.Eta())) / (Subleading_lep.Eta() - Subleading_jet.Eta()));
        m_Fbranches.at("epsilon_j3").set(*event, epsilon_j3, sys);
      }
      // gap jets
      int n_gap_jets = 0;
      if (n_jets >=3)
      {
        for (int i = 2; i < n_jets; i++)
        {
          if (jets->at(i)->eta() > std::min(Leading_jet.Eta(), Subleading_jet.Eta()) && jets->at(i)->eta() < std::max(Leading_jet.Eta(), Subleading_jet.Eta()))
          {
            n_gap_jets++;
          }
        }
      }
      m_Ibranches.at("nGapJets").set(*event, n_gap_jets, sys);
      // kinematics of vbs jets
      float max_mjj = 0.;
      const xAOD::Jet* vbsJet1 = nullptr;
      const xAOD::Jet* vbsJet2 = nullptr;

      for(unsigned int i=0;i<nonbjets->size();i++){
        for(unsigned int j=0;j<i;j++){
          const xAOD::Jet* nonbjet1 = nonbjets->at(i);
          const xAOD::Jet* nonbjet2 = nonbjets->at(j);

          if (nonbjet1->eta() * nonbjet2->eta() > 0) continue; //back-to-back
        
          // perhaps we need a minimum pt requirement on vbs jets
          //if (nonbjet1->pt() < 30. * Athena::Units::GeV || nonbjet2->pt() < 30. * Athena::Units::GeV) continue;
          double mjj = (nonbjet1->p4() + nonbjet2->p4()).M();
          if (mjj > max_mjj) {
            max_mjj = mjj;
            vbsJet1 = nonbjet1;
            vbsJet2 = nonbjet2;
          } 
        }
      }
      if (vbsJet1 && vbsJet2){
        TLorentzVector vbs_jj = vbsJet1->p4() + vbsJet2->p4();
        
        m_Fbranches.at("mjj_vbs").set(*event, vbs_jj.M(), sys);
        m_Fbranches.at("pTjj_vbs").set(*event, vbs_jj.Pt(), sys);
        m_Fbranches.at("Etajj_vbs").set(*event, vbs_jj.Eta(), sys);
        m_Fbranches.at("Phijj_vbs").set(*event, vbs_jj.Phi(), sys);
        m_Fbranches.at("dRjj_vbs").set(*event, (vbsJet1->p4()).DeltaR(vbsJet2->p4()), sys);
        m_Fbranches.at("dEtajj_vbs").set(*event, (vbsJet1->eta())-vbsJet2->eta(), sys);
        m_Fbranches.at("dPhijj_vbs").set(*event, (vbsJet1->p4()).DeltaPhi(vbsJet2->p4()), sys);
        
      }

      //b-jet sector
      for (int i=0; i<std::min(n_bjets, 2); i++){
        m_Fbranches.at("Jet_b"+std::to_string(i+1)+"_pt").set(*event, bjets->at(i)->pt(), sys);
        m_Fbranches.at("Jet_b"+std::to_string(i+1)+"_eta").set(*event, bjets->at(i)->eta(), sys);
        m_Fbranches.at("Jet_b"+std::to_string(i+1)+"_phi").set(*event, bjets->at(i)->phi(), sys);
        m_Fbranches.at("Jet_b"+std::to_string(i+1)+"_E").set(*event, bjets->at(i)->e(), sys);
        if (m_isMC) {
          m_Ibranches.at("Jet_b"+std::to_string(i+1)+"_truthLabel").set(*event, m_truthFlav.get(*bjets->at(i), sys), sys);
        }
      }

      // combine jj + ll

      TLorentzVector lljj;
      TLorentzVector lljjmet;
      if (n_jets>=2 && nLeptons>=2) {
        lljj = ll + jj;
        lljjmet = ll + jj + met_tlv;
        m_Fbranches.at("mlljj").set(*event, lljj.M(), sys);
        m_Fbranches.at("mlljjmet").set(*event, lljjmet.M(), sys);

        // Ht2r 
        double ht2 = (met_tlv + ll).Perp() + jj.Perp();
        double ht2r = ht2 / (met->met() + Leading_lep.Pt() + Subleading_lep.Pt() + Leading_jet.Pt() + Subleading_jet.Pt());

        m_Fbranches.at("HT2").set(*event, ht2, sys);
        m_Fbranches.at("HT2r").set(*event, ht2r, sys);
      }

      // Transverse mass of the pT-leading lepton wrt met
      if (nLeptons >= 1)
      {
        float mt_lept1_met = std::sqrt(2 * met->met() * Leading_lep.Pt() * (1 - std::cos(Leading_lep.DeltaPhi(met_tlv))));
        m_Fbranches.at("mT_Lepton1_Met").set(*event, mt_lept1_met, sys);

        if (nLeptons >= 2)
        {
          float mt_lept2_met = std::sqrt(2 * met->met() * Subleading_lep.Pt() * (1 - std::cos(Subleading_lep.DeltaPhi(met_tlv))));
          float mt_l_min = std::min(mt_lept1_met, mt_lept2_met);
          m_Fbranches.at("mT_Lepton2_Met").set(*event, mt_lept2_met, sys);
          m_Fbranches.at("mT_L_min").set(*event, mt_l_min, sys);
        }
      }

      // MT2_jj
      if (n_jets>=2){
        // stransverse mass of jet pair MT2_jj
        ComputeMT2 mt2_calculator = ComputeMT2(Leading_jet, Subleading_jet, met_tlv, 0, 0);
        double mT2_jj = mt2_calculator.Compute();
        m_Fbranches.at("mT2_jj").set(*event, mT2_jj, sys);
      }

    }
    return StatusCode::SUCCESS;
  }

}


