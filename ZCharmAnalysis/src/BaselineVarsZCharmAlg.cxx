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

      // c-jet sector
      std::string ftag2D_WP = "ftag_quantile_GN2v01_Continuous2D";
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
      }

      //jet sector
      for (int i=0; i<std::min(n_jets,2); i++){
        std::string prefix = "Jet"+std::to_string(i+1);
        m_Fbranches.at(prefix+"_pt").set(*event, jets->at(i)->pt(), sys);
        m_Fbranches.at(prefix+"_eta").set(*event, jets->at(i)->eta(), sys);
        m_Fbranches.at(prefix+"_phi").set(*event, jets->at(i)->phi(), sys);
        m_Fbranches.at(prefix+"_E").set(*event, jets->at(i)->e(), sys); 
      }

      //c-jet sector
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

    }
    return StatusCode::SUCCESS;
  }

}


