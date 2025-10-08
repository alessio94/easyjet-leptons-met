/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "BaselineVarsAlg.h"
#include "AthContainers/AuxElement.h"
#include "TLorentzVector.h"
#include <AthenaKernel/Units.h>
namespace HZALLYY {
  BaselineVarsAlg::BaselineVarsAlg(const std::string & name,
    ISvcLocator * pSvcLocator): AthHistogramAlgorithm(name, pSvcLocator) {
    
  }
  
  StatusCode BaselineVarsAlg::initialize() {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("       BaselineVarsAlg       \n");
    ATH_MSG_INFO("*********************************\n");
    
    // Read syst-aware input handles
    ATH_CHECK(m_eventHandle.initialize(m_systematicsList));
        
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

    for (const auto& particle : {"Lepton", "Photon"}) {
      for (int i = 1; i <= 2; ++i) {
        std::string base = std::string(particle) + std::to_string(i);
        for (const auto& var : {"pt", "eta", "phi", "E"}) {
	  m_fvars.emplace_back(base + "_" + var);
        }
      }
    }
    
    for (const std::string &string_var: m_fvars) {
      CP::SysReadDecorHandle<float> var {string_var+"_%SYS%", this};
      m_FDecors.emplace(string_var, var);
      ATH_CHECK (m_FDecors.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }

    for (const std::string &string_var: m_ivars) {
      CP::SysReadDecorHandle<int> var {string_var+"_%SYS%", this};
      m_IDecors.emplace(string_var, var);
      ATH_CHECK (m_IDecors.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }
    
    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK(m_systematicsList.initialize());
    
    return StatusCode::SUCCESS;
  }
  
  StatusCode BaselineVarsAlg::execute() {
    
    // Loop over all systs
    for (const auto & sys: m_systematicsList.systematicsVector()) {
      
      // Retrieve inputs
      const xAOD::EventInfo * event = nullptr;
      ANA_CHECK(m_eventHandle.retrieve(event, sys));
      
      for (const std::string & string_var: m_floatVariables) {
        m_Fbranches.at(string_var).set( * event, -99., sys);
      }
      
      for (const auto &var: m_intVariables) {
        m_Ibranches.at(var).set( * event, -99, sys);
      }
      
      TLorentzVector Leading_lep(0.,0.,0.,0.);
      TLorentzVector Subleading_lep(0.,0.,0.,0.);
      TLorentzVector Leading_photon(0.,0.,0.,0.);
      TLorentzVector Subleading_photon(0.,0.,0.,0.);
      TLorentzVector yy(0.,0.,0.,0.);
      TLorentzVector ll(0.,0.,0.,0.);
      TLorentzVector llyy(0.,0.,0.,0.);
      
      int n_leptons = m_IDecors.at("nLeptons").get(*event, sys);
      int n_photons = m_IDecors.at("nPhotons").get(*event, sys);
      
      Leading_lep.SetPtEtaPhiE(m_FDecors.at("Lepton1_pt").get(*event, sys), 
			       m_FDecors.at("Lepton1_eta").get(*event, sys), 
			       m_FDecors.at("Lepton1_phi").get(*event, sys), 
			       m_FDecors.at("Lepton1_E").get(*event, sys));
      
      Subleading_lep.SetPtEtaPhiE(m_FDecors.at("Lepton2_pt").get(*event, sys), 
				  m_FDecors.at("Lepton2_eta").get(*event, sys), 
				  m_FDecors.at("Lepton2_phi").get(*event, sys), 
				  m_FDecors.at("Lepton2_E").get(*event, sys));
      
      Leading_photon.SetPtEtaPhiE(m_FDecors.at("Photon1_pt").get(*event, sys), 
				  m_FDecors.at("Photon1_eta").get(*event, sys), 
				  m_FDecors.at("Photon1_phi").get(*event, sys), 
				  m_FDecors.at("Photon1_E").get(*event, sys));

   
      Subleading_photon.SetPtEtaPhiE(m_FDecors.at("Photon2_pt").get(*event, sys), 
				     m_FDecors.at("Photon2_eta").get(*event, sys), 
				     m_FDecors.at("Photon2_phi").get(*event, sys), 
				     m_FDecors.at("Photon2_E").get(*event, sys));
    
    if(n_leptons>=2) ll = Leading_lep + Subleading_lep;
      

      bool Is_Resolved = false;
      bool Is_Merged = false;

      if ( n_photons >=2&&
       	   Leading_photon.Pt() > 10.0 * Athena::Units::GeV &&
       	   Subleading_photon.Pt() > 10.0 * Athena::Units::GeV)
	//&& Leading_photon.DeltaR(Subleading_photon) < 1.5)
      
	    {
	      Is_Resolved = true;
	  
	      yy = Leading_photon + Subleading_photon;
	
	      m_Fbranches.at("res_myy").set(*event, yy.M(), sys);
	      m_Fbranches.at("res_pTyy").set(*event, yy.Pt(), sys);
	      m_Fbranches.at("res_Etayy").set(*event, yy.Eta(), sys);
	      m_Fbranches.at("res_Phiyy").set(*event, yy.Phi(), sys);
	      m_Fbranches.at("res_dRyy").set(*event, Leading_photon.DeltaR(Subleading_photon), sys);
	      m_Fbranches.at("res_dPhiyy").set(*event, abs(Leading_photon.DeltaPhi(Subleading_photon)), sys);
	      m_Fbranches.at("res_dEtayy").set(*event,  abs(Leading_photon.Eta()- Subleading_photon.Eta()) , sys);
	      m_Fbranches.at("res_Xyy").set(*event, (Leading_photon.DeltaR(Subleading_photon) * yy.Pt())/(2*yy.M()) , sys);
	      m_Fbranches.at("res_Ph1ptOvermyy").set(*event, Leading_photon.Pt()/yy.M(), sys);
	      m_Fbranches.at("res_Ph2ptOvermyy").set(*event, Subleading_photon.Pt()/yy.M(), sys);
	  
	      // H->Za->yyll system building
	      if (n_leptons >= 2) {
	    
		llyy  = yy + ll;
	    
		// Set variables for the H->Za system
		m_Fbranches.at("res_mH_Za").set(*event, llyy.M(), sys);
		m_Fbranches.at("res_pTH_Za").set(*event, llyy.Pt(), sys);
		m_Fbranches.at("res_EtaH_Za").set(*event, llyy.Eta(), sys);
		m_Fbranches.at("res_PhiH_Za").set(*event, llyy.Phi(), sys);
		m_Fbranches.at("res_dRH_Za").set(*event, ll.DeltaR(yy), sys);
		m_Fbranches.at("res_dPhiH_Za").set(*event, abs(ll.DeltaPhi(yy)), sys);
		m_Fbranches.at("res_dEtaH_Za").set(*event, abs(ll.Eta() - yy.Eta()), sys);
	      }
	    }
	
      if ( !Is_Resolved && n_photons >=1 &&
	   Leading_photon.Pt() > 20 * Athena::Units::GeV)
	{
	  Is_Merged = true;
	
	  // H->Za->yyll system building
	  if (n_leptons >= 2) {
	    
	    llyy  = Leading_photon + ll;
	    
	    // Set variables for the H->Za system
	    m_Fbranches.at("mer_mH_Za").set(*event, llyy.M(), sys);
	    m_Fbranches.at("mer_pTH_Za").set(*event, llyy.Pt(), sys);
	    m_Fbranches.at("mer_EtaH_Za").set(*event, llyy.Eta(), sys);
	    m_Fbranches.at("mer_PhiH_Za").set(*event, llyy.Phi(), sys);
	    m_Fbranches.at("mer_dRH_Za").set(*event, ll.DeltaR(Leading_photon), sys);
	    m_Fbranches.at("mer_dPhiH_Za").set(*event, abs(ll.DeltaPhi(Leading_photon)), sys);
	    m_Fbranches.at("mer_dEtaH_Za").set(*event, abs(ll.Eta() - Leading_photon.Eta()), sys);
	  }
	}
      
      m_Ibranches.at("isResolved_Event").set( * event, Is_Resolved, sys);
      m_Ibranches.at("isMerged_Event").set( * event, Is_Merged, sys);
    } // 
      
          return StatusCode::SUCCESS;
  }
  
}
