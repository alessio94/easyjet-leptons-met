/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LeptonVarsbbllAlg.h"

namespace HHBBLL
{
  LeptonVarsbbllAlg::LeptonVarsbbllAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {}

  StatusCode LeptonVarsbbllAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("       LeptonVarsbbllAlg       \n");
    ATH_MSG_INFO("*********************************\n");

    ATH_CHECK (m_bbllElectronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_bbllMuonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    if(m_isMC){
      ATH_CHECK (m_ele_truthOrigin.initialize(m_systematicsList, m_bbllElectronHandle));
      ATH_CHECK (m_ele_truthType.initialize(m_systematicsList, m_bbllElectronHandle));
      ATH_CHECK (m_mu_truthOrigin.initialize(m_systematicsList, m_bbllMuonHandle));
      ATH_CHECK (m_mu_truthType.initialize(m_systematicsList, m_bbllMuonHandle));
      if(!m_saveDummy_ele_SF){
        ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
        m_ele_SF = CP::SysReadDecorHandle<float>("effSF_"+m_eleWPName+"_%SYS%", this);
        ATH_CHECK (m_ele_SF.initialize(m_systematicsList, m_electronHandle));
      }
      ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
      m_mu_SF = CP::SysReadDecorHandle<float>("effSF_"+m_muWPName+"_%SYS%", this);
      ATH_CHECK (m_mu_SF.initialize(m_systematicsList, m_muonHandle));
    }

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

  StatusCode LeptonVarsbbllAlg::execute()
  {
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      if (!m_doSystematics && sys.name()!="") continue;

      // container we read in
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_bbllElectronHandle.retrieve (electrons, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_bbllMuonHandle.retrieve (muons, sys));

      const xAOD::Electron* ele0 = nullptr;
      const xAOD::Electron* ele1 = nullptr;

      for(unsigned int i=0; i<std::min(size_t(2),electrons->size()); i++){
        const xAOD::Electron* ele = electrons->at(i);
        if(i==0) ele0 = ele;
        else if(i==1) ele1 = ele;
      }

      // Muon sector
      const xAOD::Muon* mu0 = nullptr;
      const xAOD::Muon* mu1 = nullptr;

      for(unsigned int i=0; i<std::min(size_t(2),muons->size()); i++){
        const xAOD::Muon* mu = muons->at(i);
        if(i==0) mu0 = mu;
        else if(i==1) mu1 = mu;
      }
      //
      for (const std::string &string_var: m_floatVariables) {
        m_Fbranches.at(string_var).set(*event, -99., sys);
      }

      for (const auto& var: m_intVariables) {
        m_Ibranches.at(var).set(*event, -99, sys);
      }

      TLorentzVector Leading_lep;
      TLorentzVector Subleading_lep;
      //
      int n_electrons = electrons->size();;
      int n_muons= muons->size();
      //
      if(m_save_extra_vars) {
	m_Ibranches.at("nElectrons").set(*event, n_electrons, sys);
	m_Ibranches.at("nMuons").set(*event, n_muons, sys);
      }
      m_Ibranches.at("nLeptons").set(*event, electrons->size() + muons->size(), sys);
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
        int lep_pdgid = leptons[i].second;
        if(i==0) Leading_lep = tlv;
        else if(i==1) Subleading_lep = tlv;
        m_Fbranches.at(prefix+"_pt").set(*event, tlv.Pt(), sys);
        m_Fbranches.at(prefix+"_eta").set(*event, tlv.Eta(), sys);
        m_Fbranches.at(prefix+"_phi").set(*event, tlv.Phi(), sys);
        m_Fbranches.at(prefix+"_E").set(*event, tlv.E(), sys);
        if(m_isMC){
	  float SF = 1.;
          if(std::abs(leptons[i].second)==13)
            SF = m_mu_SF.get(*leptons[i].first,sys);
          else if(!m_saveDummy_ele_SF)
            SF = m_ele_SF.get(*leptons[i].first,sys);
	  m_Fbranches.at(prefix+"_effSF").set(*event, SF, sys);
        }
	int charge = leptons[i].second>0 ? -1 : 1;
        m_Ibranches.at(prefix+"_charge").set(*event, charge, sys);
        m_Ibranches.at(prefix+"_pdgid").set(*event, leptons[i].second, sys);
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
      TLorentzVector ll(0.,0.,0.,0.);
      if(leptons.size() >=2) {
	m_Fbranches.at("mll").set(*event, ll.M(), sys);
	m_Fbranches.at("pTll").set(*event, ll.Pt(), sys);
	m_Fbranches.at("Phill").set(*event, ll.Phi(), sys);
	m_Fbranches.at("dRll").set(*event, Leading_lep.DeltaR(Subleading_lep), sys);
      }
    }
    return StatusCode::SUCCESS;
  }
}




