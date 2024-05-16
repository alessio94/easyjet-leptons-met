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
      ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
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
        ATH_CHECK (m_truthFlav.initialize(m_systematicsList, m_jetHandle));
      }

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

        const xAOD::JetContainer *jets = nullptr;
        ANA_CHECK (m_jetHandle.retrieve (jets, sys));

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
        
        int n_jets = jets->size();
        int nCentralJets = 0;

        int n_electrons = electrons->size();
        int n_muons = muons->size();

        const xAOD::Electron* ele0 = nullptr;
        const xAOD::Electron* ele1 = nullptr;
        const xAOD::Muon* mu0 = nullptr;
        const xAOD::Muon* mu1 = nullptr;
        // b-jet sector
        bool WPgiven = !m_isBtag.empty();
        auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
        
        for(const xAOD::Jet* jet : *jets) {
          // count central jets
          if (std::abs(jet->eta())<2.5) {
            nCentralJets++;
            if (WPgiven) {
              if (m_isBtag.get(*jet, sys)) bjets->push_back(jet);
            }
          }
        }
        int n_bjets = bjets->size();

        m_Ibranches.at("nJets").set(*event, n_jets, sys);
        m_Ibranches.at("nElectrons").set(*event, n_electrons, sys);
        m_Ibranches.at("nMuons").set(*event, n_muons, sys);
        m_Ibranches.at("nBJets").set(*event, n_bjets, sys);
        m_Ibranches.at("nCentralJets").set(*event, nCentralJets, sys);
        // Electron sector
        for (std::size_t i=0; i<std::min(electrons->size(),(std::size_t)2); i++){
          const xAOD::Electron* ele = electrons->at(i);
          //leading electron
          if (i==0) ele0 = ele;
          //subleading electron
          else if (i==1) ele1 = ele;

          if(m_isMC){
            float ele_SF = m_ele_SF.get(*ele, sys);
            m_Fbranches.at("Electron"+std::to_string(i+1)+"_effSF").set(*event, ele_SF, sys);
          }

          m_Fbranches.at("Electron"+std::to_string(i+1)+"_pt").set(*event, ele->pt(), sys);
          m_Fbranches.at("Electron"+std::to_string(i+1)+"_eta").set(*event, ele->eta(), sys);
          m_Fbranches.at("Electron"+std::to_string(i+1)+"_phi").set(*event, ele->phi(), sys);
          m_Fbranches.at("Electron"+std::to_string(i+1)+"_E").set(*event, ele->e(), sys);


        }
        // Muon sector
        for (std::size_t i=0; i< std::min(muons->size(),(std::size_t)2); i++){
          const xAOD::Muon* mu = muons->at(i);
          //leading muon
          if (i==0) mu0 = mu;
          //subleading muon
          else if (i==1) mu1 = mu;
          
          if(m_isMC){
            float mu_SF = m_mu_SF.get(*mu, sys);
            m_Fbranches.at("Muon"+std::to_string(i+1)+"_effSF").set(*event, mu_SF, sys);
          }

          m_Fbranches.at("Muon"+std::to_string(i+1)+"_pt").set(*event, mu->pt(), sys);
          m_Fbranches.at("Muon"+std::to_string(i+1)+"_eta").set(*event, mu->eta(), sys);
          m_Fbranches.at("Muon"+std::to_string(i+1)+"_phi").set(*event, mu->phi(), sys);
          m_Fbranches.at("Muon"+std::to_string(i+1)+"_E").set(*event, mu->e(), sys);
        
        }
        TLorentzVector ll;
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
        
        bool is_ee = electrons->size() == 2 && muons->size() == 0 ;
        bool is_mumu = electrons->size() == 0 && muons->size() == 2;
        bool is_emu = electrons->size() == 1 && muons->size() == 1 ;

        TLorentzVector Leading_lep;
        TLorentzVector Subleading_lep;

        float lep1_SF = -99;
        int lep1_charge = -99;
        int lep1_pdgid = -99;
        float lep2_SF = -99;
        int lep2_charge = -99;
        int lep2_pdgid = -99;

        if (is_ee) {
          Leading_lep = ele0->p4();
          Subleading_lep = ele1->p4();

          lep1_charge = ele0->charge();
          lep2_charge = ele1->charge();

          lep1_pdgid = ele0->charge() > 0 ? -11 : 11;
          lep2_pdgid = ele1->charge() > 0 ? -11 : 11;

          if(m_isMC){
            lep1_SF = m_ele_SF.get(*ele0, sys);
            lep2_SF = m_ele_SF.get(*ele1, sys);
          }
        }
        else if(is_mumu) {
          Leading_lep = mu0->p4();
          Subleading_lep = mu1->p4();

          lep1_charge = mu0->charge();
          lep2_charge = mu1->charge();

          lep1_pdgid = mu0->charge() > 0 ? -13 : 13;
          lep2_pdgid = mu1->charge() > 0 ? -13 : 13;
          
          if(m_isMC){
            lep1_SF = m_mu_SF.get(*mu0, sys);
            lep2_SF = m_mu_SF.get(*mu1, sys);
          }
          
        }
        else if(is_emu) {
          if (ele0->pt() > mu0->pt()){
            Leading_lep = ele0->p4();
            Subleading_lep = mu0->p4();

            lep1_charge = ele0->charge();
            lep2_charge = mu0->charge();
            lep1_pdgid = ele0->charge() > 0 ? -11 : 11;
            lep2_pdgid = mu0->charge() > 0 ? -13 : 13;
            if(m_isMC){
              lep1_SF = m_ele_SF.get(*ele0, sys);
              lep2_SF = m_mu_SF.get(*mu0, sys);
            }
          }
          else{
            Leading_lep = mu0->p4();
            Subleading_lep = ele0->p4();
            
            lep1_charge = mu0->charge();
            lep2_charge = ele0->charge();
            lep1_pdgid = mu0->charge() > 0 ? -13 : 13;
            lep2_pdgid = ele0->charge() > 0 ? -11 : 11;
            if(m_isMC){
              lep1_SF = m_mu_SF.get(*mu0, sys);
              lep2_SF = m_ele_SF.get(*ele0, sys);
            }

          }
          
        }
        if(is_ee || is_mumu || is_mumu){
          m_Fbranches.at("Lepton1_pt").set(*event, Leading_lep.Pt(), sys);
          m_Fbranches.at("Lepton1_eta").set(*event, Leading_lep.Eta(), sys);
          m_Fbranches.at("Lepton1_phi").set(*event, Leading_lep.Phi(), sys);
          m_Fbranches.at("Lepton1_E").set(*event, Leading_lep.E(), sys);
          if(m_isMC) m_Fbranches.at("Lepton1_effSF").set(*event, lep1_SF, sys);
          m_Ibranches.at("Lepton1_charge").set(*event, lep1_charge, sys);
          m_Ibranches.at("Lepton1_pdgid").set(*event, lep1_pdgid, sys);

          m_Fbranches.at("Lepton2_pt").set(*event, Subleading_lep.Pt(), sys);
          m_Fbranches.at("Lepton2_eta").set(*event, Subleading_lep.Eta(), sys);
          m_Fbranches.at("Lepton2_phi").set(*event, Subleading_lep.Phi(), sys);
          m_Fbranches.at("Lepton2_E").set(*event, Subleading_lep.E(), sys);
          if(m_isMC) m_Fbranches.at("Lepton2_effSF").set(*event, lep2_SF, sys);
          m_Ibranches.at("Lepton2_charge").set(*event, lep2_charge, sys);
          m_Ibranches.at("Lepton2_pdgid").set(*event, lep2_pdgid, sys);

          m_Fbranches.at("dRll").set(*event, Leading_lep.DeltaR(Subleading_lep), sys);
        }

        //jet sector
        for (std::size_t i=0; i<std::min(jets->size(),(std::size_t)2); i++){
          m_Fbranches.at("Jet"+std::to_string(i+1)+"_pt").set(*event, jets->at(i)->pt(), sys);
          m_Fbranches.at("Jet"+std::to_string(i+1)+"_eta").set(*event, jets->at(i)->eta(), sys);
          m_Fbranches.at("Jet"+std::to_string(i+1)+"_phi").set(*event, jets->at(i)->phi(), sys);
          m_Fbranches.at("Jet"+std::to_string(i+1)+"_E").set(*event, jets->at(i)->e(), sys); 
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
        }
        // b-jet + lepton sector
        if (n_bjets>=1 && (n_electrons>=1 || n_muons>=1)) {
          TLorentzVector bl = bjets->at(0)->p4()+Leading_lep;
          m_Fbranches.at("mbl").set(*event, bl.M(), sys);
          m_Fbranches.at("pTbl").set(*event, bl.Pt(), sys);
          m_Fbranches.at("Etabl").set(*event, bl.Eta(), sys);
          m_Fbranches.at("Phibl").set(*event, bl.Phi(), sys);
          m_Fbranches.at("dRbl").set(*event, (bjets->at(0)->p4()).DeltaR(Leading_lep), sys);
        }
      }
      return StatusCode::SUCCESS;
    } 
}