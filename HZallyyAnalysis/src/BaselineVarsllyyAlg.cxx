/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "BaselineVarsllyyAlg.h"

#include "AthContainers/AuxElement.h"

#include <AthContainers/ConstDataVector.h>

#include "TLorentzVector.h"

#include "EasyjetHub/MT2_ROOT.h"

namespace HZALLYY {
  BaselineVarsllyyAlg::BaselineVarsllyyAlg(const std::string & name,
    ISvcLocator * pSvcLocator): AthHistogramAlgorithm(name, pSvcLocator) {
    
  }
  
  StatusCode BaselineVarsllyyAlg::initialize() {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("       BaselineVarsllyyAlg       \n");
    ATH_MSG_INFO("*********************************\n");
    
    // Read syst-aware input handles
    ATH_CHECK(m_llyyphotonHandle.initialize(m_systematicsList));
    ATH_CHECK(m_llyyelectronHandle.initialize(m_systematicsList));
    ATH_CHECK(m_llyymuonHandle.initialize(m_systematicsList));
    ATH_CHECK(m_eventHandle.initialize(m_systematicsList));
    
    if (m_isMC) {
      
      ATH_CHECK(m_ele_truthOrigin.initialize(m_systematicsList, m_llyyelectronHandle));
      ATH_CHECK(m_ele_truthType.initialize(m_systematicsList, m_llyyelectronHandle));
      ATH_CHECK(m_mu_truthOrigin.initialize(m_systematicsList, m_llyymuonHandle));
      ATH_CHECK(m_mu_truthType.initialize(m_systematicsList, m_llyymuonHandle));
           
      if (!m_saveDummy_ele_SF)
	{
	  ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
	  m_ele_SF = CP::SysReadDecorHandle < float > ("effSF_" + m_eleWPName + "_%SYS%", this);
	  ATH_CHECK(m_ele_SF.initialize(m_systematicsList, m_electronHandle));
	}
      
      ATH_CHECK (m_photonHandle.initialize(m_systematicsList));
      m_ph_SF = CP::SysReadDecorHandle < float > ("effSF_" + m_phWPName + "_%SYS%", this);
      ATH_CHECK(m_ph_SF.initialize(m_systematicsList, m_photonHandle));
      
      ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
      m_mu_SF = CP::SysReadDecorHandle < float > ("effSF_" + m_muWPName + "_%SYS%", this);
      ATH_CHECK(m_mu_SF.initialize(m_systematicsList, m_muonHandle));
      
    }
    
    // Intialise syst-aware output decorators
    for (const std::string &var: m_floatVariables) {
      CP::SysWriteDecorHandle < float > whandle {var +"_%SYS%", this};
      m_Fbranches.emplace(var, whandle);
      ATH_CHECK(m_Fbranches.at(var).initialize(m_systematicsList, m_eventHandle));
    }
    
    for (const std::string &var: m_intVariables) {
      ATH_MSG_DEBUG("initializing integer variable: " <<var);
      CP::SysWriteDecorHandle < int > whandle {var +"_%SYS%", this};
      m_Ibranches.emplace(var, whandle);
      ATH_CHECK(m_Ibranches.at(var).initialize(m_systematicsList, m_eventHandle));
    };
    
    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK(m_systematicsList.initialize());
    
    return StatusCode::SUCCESS;
  }
  
  StatusCode BaselineVarsllyyAlg::execute() {
    
    // Loop over all systs
    for (const auto & sys: m_systematicsList.systematicsVector()) {
      
      // Retrieve inputs
      const xAOD::EventInfo * event = nullptr;
      ANA_CHECK(m_eventHandle.retrieve(event, sys));
      
      const xAOD::PhotonContainer * photons = nullptr;
      ANA_CHECK(m_llyyphotonHandle.retrieve(photons, sys));
      
      const xAOD::MuonContainer * muons = nullptr;
      ANA_CHECK(m_llyymuonHandle.retrieve(muons, sys));

      const xAOD::ElectronContainer * electrons = nullptr;
      ANA_CHECK(m_llyyelectronHandle.retrieve(electrons, sys));
      
      for (const std::string & string_var: m_floatVariables) {
        m_Fbranches.at(string_var).set( * event, -99., sys);
      }
      
      for (const auto &var: m_intVariables) {
        m_Ibranches.at(var).set( * event, -99, sys);
      }

      static const SG::AuxElement::ConstAccessor < int > HadronConeExclTruthLabelID("HadronConeExclTruthLabelID");

      // Count photons
      int n_photons = 0;
      n_photons = photons -> size();
      
      // Photon sector
      // initialize
      const xAOD::Photon * ph0 = nullptr;
      const xAOD::Photon * ph1 = nullptr;
      
      for (unsigned int i = 0; i < std::min(size_t(2), photons -> size()); i++) {
      	const xAOD::Photon * ph = photons -> at(i);
        if (i == 0) ph0 = ph;
        else if (i == 1) ph1 = ph;
      }// end photon
      
            
      for (unsigned int i = 0; i < std::min(size_t(2), photons -> size()); i++) {
      	const xAOD::Photon * ph = photons -> at(i);
        if (!ph) continue;
        std::string prefix = "Photon" + std::to_string(i + 1);
        TLorentzVector tlv = photons -> at(i) -> p4();
	
        m_Fbranches.at(prefix + "_pt").set( * event, tlv.Pt(), sys);
        m_Fbranches.at(prefix + "_eta").set( * event, tlv.Eta(), sys);
        m_Fbranches.at(prefix + "_phi").set( * event, tlv.Phi(), sys);
        m_Fbranches.at(prefix + "_E").set( * event, tlv.E(), sys);
	
	
	if(m_isMC){
          m_Fbranches.at(prefix+"_effSF").set(*event, m_ph_SF.get(*ph, sys), sys);
        }


	
      }
      
      TLorentzVector yy(0., 0., 0., 0.);
      if (ph0 && ph1) {
        yy = ph0 -> p4() + ph1 -> p4();
        m_Fbranches.at("myy").set( * event, yy.M(), sys);
        m_Fbranches.at("pTyy").set( * event, yy.Pt(), sys);
        m_Fbranches.at("Etayy").set( * event, yy.Eta(), sys);
        m_Fbranches.at("Phiyy").set( * event, yy.Phi(), sys);
        m_Fbranches.at("dRyy").set( * event, ph0 -> p4().DeltaR(ph1 -> p4()), sys);
	m_Fbranches.at("dPhiyy").set( * event, ph0 -> p4().DeltaPhi(ph1 -> p4()), sys);
	m_Fbranches.at("dEtayy").set( * event,  ph0 -> p4().Eta()- ph1 -> p4().Eta() , sys);
	m_Fbranches.at("Xyy").set( * event, (ph0 -> p4().DeltaR(ph1 -> p4()) * yy.Pt())/(2*yy.M()) , sys);
      }
      
      int n_electrons = 0;
      int n_muons = 0;

      // Count electrons
      n_electrons = electrons -> size();

      // Count muons
      n_muons = muons -> size();

      m_Ibranches.at("nElectrons").set( * event, n_electrons, sys);
      m_Ibranches.at("nMuons").set( * event, n_muons, sys);
      m_Ibranches.at("nPhotons").set( * event, n_photons, sys);
      
      // Electron sector
      const xAOD::Electron * ele0 = nullptr;
      const xAOD::Electron * ele1 = nullptr;
      
      for (unsigned int i = 0; i < std::min(size_t(2), electrons -> size()); i++) {
        const xAOD::Electron * ele = electrons -> at(i);
        if (i == 0) ele0 = ele;
        else if (i == 1) ele1 = ele;
      }
      
      // Muon sector
      const xAOD::Muon * mu0 = nullptr;
      const xAOD::Muon * mu1 = nullptr;
      
      for (unsigned int i = 0; i < std::min(size_t(2), muons -> size()); i++) {
        const xAOD::Muon * mu = muons -> at(i);
        if (i == 0) mu0 = mu;
        else if (i == 1) mu1 = mu;
      } // end muon
      
      std::vector<std::pair<const xAOD::IParticle * , int >> leptons;
      if (ele0) leptons.emplace_back(ele0, -11 * ele0 -> charge());
      if (mu0) leptons.emplace_back(mu0, -13 * mu0 -> charge());
      if (ele1) leptons.emplace_back(ele1, -11 * ele1 -> charge());
      if (mu1) leptons.emplace_back(mu1, -13 * mu1 -> charge());
      
      std::sort(leptons.begin(), leptons.end(),
		[](const std::pair <const xAOD::IParticle * , int > & a,
		   const std::pair <const xAOD::IParticle * , int > & b) {
		  return a.first -> pt() > b.first -> pt();
		});

      TLorentzVector Leading_lep;
      TLorentzVector Subleading_lep;
      TLorentzVector ll;
      
      for (unsigned int i = 0; i < std::min(size_t(2), leptons.size()); i++) {
	
        std::string prefix = "Lepton" + std::to_string(i + 1);
        TLorentzVector tlv = leptons[i].first -> p4();
        int lep_pdgid = leptons[i].second;
	
        if (i == 0) Leading_lep = tlv;
        else if (i == 1) Subleading_lep = tlv;
	
        m_Fbranches.at(prefix + "_pt").set( * event, tlv.Pt(), sys);
        m_Fbranches.at(prefix + "_eta").set( * event, tlv.Eta(), sys);
        m_Fbranches.at(prefix + "_phi").set( * event, tlv.Phi(), sys);
        m_Fbranches.at(prefix + "_E").set( * event, tlv.E(), sys);
	
        if (m_isMC) {
	  float SF = 1.;
          if(std::abs(leptons[i].second)==13)
            SF = m_mu_SF.get(*leptons[i].first,sys);
          else if(!m_saveDummy_ele_SF)
            SF = m_ele_SF.get(*leptons[i].first,sys);
          m_Fbranches.at(prefix+"_effSF").set(*event, SF, sys);
	}
	
        int charge = leptons[i].second > 0 ? -1 : 1;
	
        m_Ibranches.at(prefix + "_charge").set( * event, charge, sys);
        m_Ibranches.at(prefix + "_pdgid").set( * event, leptons[i].second, sys);
	
        // leptons truth information
        if (m_isMC) {
	  
          int lep_truthOrigin = std::abs(lep_pdgid) == 11 ?
            m_ele_truthOrigin.get( * leptons[i].first, sys) :
            m_mu_truthOrigin.get( * leptons[i].first, sys);
          m_Ibranches.at(prefix + "_truthOrigin").set( * event, lep_truthOrigin, sys);
	  
          int lep_truthType = std::abs(lep_pdgid) == 11 ?
            m_ele_truthType.get( * leptons[i].first, sys) :
            m_mu_truthType.get( * leptons[i].first, sys);
          m_Ibranches.at(prefix + "_truthType").set( * event, lep_truthType, sys);
	  
          int lep_isPrompt = 0;
          if (std::abs(lep_pdgid) == 13) { // simplistic
            if (lep_truthType == 6) lep_isPrompt = 1; // isolated prompts
          } else if (std::abs(lep_pdgid) == 11) {
            if (lep_truthType == 2) lep_isPrompt = 1; // isolated prompts
          }
          m_Ibranches.at(prefix + "_isPrompt").set( * event, lep_isPrompt, sys);
        }
      }
      
      if (leptons.size() >= 2) {
        ll = Leading_lep + Subleading_lep;
        m_Fbranches.at("mll").set( * event, ll.M(), sys);
        m_Fbranches.at("pTll").set( * event, ll.Pt(), sys);
        m_Fbranches.at("Etall").set( * event, ll.Eta(), sys);
        m_Fbranches.at("Phill").set( * event, ll.Phi(), sys);
        m_Fbranches.at("dRll").set( * event, Leading_lep.DeltaR(Subleading_lep), sys);
	m_Fbranches.at("dPhill").set( * event, Leading_lep.DeltaPhi(Subleading_lep), sys);
	m_Fbranches.at("dEtall").set( * event, Leading_lep.Eta() - Subleading_lep.Eta() , sys);
	
      }
      
      // H->Za->yyll system building
      if (ph0 && ph1 && leptons.size() >= 2) {
	
        TLorentzVector Z_ll  = Leading_lep + Subleading_lep;
        TLorentzVector a_yy  = ph0 -> p4() + ph1 -> p4();
        TLorentzVector H_Za  = a_yy + Z_ll;
        
        // Set variables for the H->aa system
	m_Fbranches.at("mH_Za").set(*event, H_Za.M(), sys);
        m_Fbranches.at("pTH_Za").set(*event, H_Za.Pt(), sys);
        m_Fbranches.at("EtaH_Za").set(*event, H_Za.Eta(), sys);
        m_Fbranches.at("PhiH_Za").set(*event, H_Za.Phi(), sys);
        m_Fbranches.at("dRH_Za").set(*event, Z_ll.DeltaR(a_yy), sys);
	m_Fbranches.at("dPhiH_Za").set(*event, Z_ll.DeltaPhi(a_yy), sys);
	m_Fbranches.at("dEtaH_Za").set(*event, Z_ll.Eta() - a_yy.Eta(), sys);
	
      }
    } // 
    
    return StatusCode::SUCCESS;
  }
  
}
