/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_largejetHandle.initialize(m_systematicsList));
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
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    ATH_CHECK (m_METSig.initialize(m_systematicsList, m_metHandle));

    if (m_isMC) {
      ATH_CHECK (m_truthFlav.initialize(m_systematicsList, m_jetHandle));
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
    
    ATH_CHECK (m_GN2Xv01_phbb.initialize(m_systematicsList, m_largejetHandle));
    ATH_CHECK (m_GN2Xv01_phcc.initialize(m_systematicsList, m_largejetHandle));
    ATH_CHECK (m_GN2Xv01_pqcd.initialize(m_systematicsList, m_largejetHandle));
    ATH_CHECK (m_GN2Xv01_ptop.initialize(m_systematicsList, m_largejetHandle));

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
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      const xAOD::JetContainer *largeJets  = nullptr;
      ANA_CHECK (m_largejetHandle.retrieve (largeJets , sys));

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

      static const SG::AuxElement::ConstAccessor<int>  HadronConeExclTruthLabelID("HadronConeExclTruthLabelID");
      
      TLorentzVector Leading_jet;
      TLorentzVector Subleading_jet;
      TLorentzVector met_vector;

      // Count leptons
      int n_electrons = electrons->size();
      int n_muons = muons->size();

      // Count jets
      int n_jets = jets->size();

      // b-jet sector
      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      for(const xAOD::Jet* jet : *jets) {
        if (WPgiven) {
          if (m_isBtag.get(*jet, sys) && std::abs(jet->eta())<2.5) bjets->push_back(jet);
        }
      }
      int n_bjets = bjets->size();

      m_Ibranches.at("nElectrons").set(*event, n_electrons, sys);
      m_Ibranches.at("nMuons").set(*event, n_muons, sys);
      m_Ibranches.at("nJets").set(*event, n_jets, sys);
      m_Ibranches.at("nBJets").set(*event, n_bjets, sys);

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
          float SF = std::abs(lep_pdgid)==11 ?
          m_ele_SF.get(*leptons[i].first,sys) :
          m_mu_SF.get(*leptons[i].first,sys);
          m_Fbranches.at(prefix+"_effSF").set(*event, SF, sys);
        }
        int charge = lep_pdgid > 0 ? -1 : 1;
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

      //MET Significance 
      float METSig = m_METSig.get(*met, sys);
      m_Fbranches.at("METSig").set(*event, METSig, sys);

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
      TLorentzVector jj;
      for (int i=0; i<std::min(n_jets,2); i++){
        m_Fbranches.at("Jet"+std::to_string(i+1)+"_pt").set(*event, jets->at(i)->pt(), sys);
        m_Fbranches.at("Jet"+std::to_string(i+1)+"_eta").set(*event, jets->at(i)->eta(), sys);
        m_Fbranches.at("Jet"+std::to_string(i+1)+"_phi").set(*event, jets->at(i)->phi(), sys);
        m_Fbranches.at("Jet"+std::to_string(i+1)+"_E").set(*event, jets->at(i)->e(), sys); 
      }
      if (n_jets >=2){
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
      for (int i=0; i<std::min(n_bjets, 2); i++){
        m_Fbranches.at("Jet_b"+std::to_string(i+1)+"_pt").set(*event, bjets->at(i)->pt(), sys);
        m_Fbranches.at("Jet_b"+std::to_string(i+1)+"_eta").set(*event, bjets->at(i)->eta(), sys);
        m_Fbranches.at("Jet_b"+std::to_string(i+1)+"_phi").set(*event, bjets->at(i)->phi(), sys);
        m_Fbranches.at("Jet_b"+std::to_string(i+1)+"_E").set(*event, bjets->at(i)->e(), sys);
        if (m_isMC) {
          m_Ibranches.at("Jet_b"+std::to_string(i+1)+"_truthLabel").set(*event, m_truthFlav.get(*bjets->at(i), sys), sys);
        }
      }
      if (n_bjets >=2){
        TLorentzVector bb = bjets->at(0)->p4()+bjets->at(1)->p4();
        m_Fbranches.at("mbb").set(*event, bb.M(), sys);
        m_Fbranches.at("pTbb").set(*event, bb.Pt(), sys);
        m_Fbranches.at("Etabb").set(*event, bb.Eta(), sys);
        m_Fbranches.at("Phibb").set(*event, bb.Phi(), sys);
        m_Fbranches.at("dRbb").set(*event, (bjets->at(0)->p4()).DeltaR(bjets->at(1)->p4()), sys);
        m_Fbranches.at("dPhibb").set(*event, (bjets->at(0)->p4()).DeltaPhi(bjets->at(1)->p4()), sys);
        m_Fbranches.at("dEtabb").set(*event, (bjets->at(0)->eta()) - bjets->at(1)->eta(), sys);
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

  template<typename ParticleType>
    std::pair<int, int> BaselineVarsZCharmAlg::truthOrigin(const ParticleType* particle) {
    static const SG::AuxElement::ConstAccessor<int> lepttruthOrigin("truthOrigin");
    static const SG::AuxElement::ConstAccessor<int> lepttruthType("truthType");
  
    return {lepttruthOrigin(*particle), lepttruthType(*particle)};
  }

}


