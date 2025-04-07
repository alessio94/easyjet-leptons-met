/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "PhotonVarsAlg.h"

namespace HZALLYY {
  PhotonVarsAlg::PhotonVarsAlg(const std::string & name,
    ISvcLocator * pSvcLocator): AthHistogramAlgorithm(name, pSvcLocator) {
    
  }
  
  StatusCode PhotonVarsAlg::initialize() {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("       PhotonVarsAlg       \n");
    ATH_MSG_INFO("*********************************\n");
    
    // Read syst-aware input handles
    ATH_CHECK(m_llyyphotonHandle.initialize(m_systematicsList));
    ATH_CHECK(m_eventHandle.initialize(m_systematicsList));
    
    if (m_isMC) {
      ATH_CHECK (m_photonHandle.initialize(m_systematicsList));
      m_ph_SF = CP::SysReadDecorHandle < float > ("effSF_" + m_phWPName + "_%SYS%", this);
      ATH_CHECK(m_ph_SF.initialize(m_systematicsList, m_photonHandle));
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
  
  StatusCode PhotonVarsAlg::execute() {
    
    // Loop over all systs
    for (const auto & sys: m_systematicsList.systematicsVector()) {
      
      // Retrieve inputs
      const xAOD::EventInfo * event = nullptr;
      ANA_CHECK(m_eventHandle.retrieve(event, sys));
      
      const xAOD::PhotonContainer * photons = nullptr;
      ANA_CHECK(m_llyyphotonHandle.retrieve(photons, sys));

      for (const std::string & string_var: m_floatVariables) {
        m_Fbranches.at(string_var).set( * event, -99., sys);
      }
      
      for (const auto &var: m_intVariables) {
        m_Ibranches.at(var).set( * event, -99, sys);
      }
      
      for (unsigned int i = 0; i < std::min(std::size_t(2), photons -> size()); i++) {
      	const xAOD::Photon * ph = photons -> at(i);
        if (!ph) continue;
        std::string prefix = "Photon" + std::to_string(i + 1);
        TLorentzVector tlv = ph -> p4();
	m_Fbranches.at(prefix + "_pt").set( * event, tlv.Pt(), sys);
        m_Fbranches.at(prefix + "_eta").set( * event, tlv.Eta(), sys);
        m_Fbranches.at(prefix + "_phi").set( * event, tlv.Phi(), sys);
        m_Fbranches.at(prefix + "_E").set( * event, tlv.E(), sys);
	if(m_isMC){
          m_Fbranches.at(prefix+"_effSF").set(*event, m_ph_SF.get(*ph, sys), sys);
        }
      }            
      
      m_Ibranches.at("nPhotons").set( * event, photons -> size(), sys);
    } // 
    
    return StatusCode::SUCCESS;
  }
  
}
