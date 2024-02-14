/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Abraham Tishelman-Charny

#include "OrthogonalityAlg.h"

namespace Easyjet
{
  OrthogonalityAlg::OrthogonalityAlg(const std::string &name,
                                       ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator) { }

  StatusCode OrthogonalityAlg::initialize()
  {
    
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("       OrthogonalityAlg       \n");
    ATH_MSG_INFO("*********************************\n");
    
    ATH_CHECK (m_photonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    
    for (const std::string &string_var: m_Bvarnames) {
      CP::SysWriteDecorHandle<bool> var {string_var+"_%SYS%", this};
      m_Bbranches.emplace(string_var, var);
      ATH_CHECK (m_Bbranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    } 
    
    ATH_CHECK (m_systematicsList.initialize()); // Initialise syst list (must come after all syst-aware inputs and outputs)
    
    return StatusCode::SUCCESS;
  }

  StatusCode OrthogonalityAlg::execute()
  {
    int N_HHPhotons = -1, N_HHBjets = -1;
      
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));
      
      // Initialize variable values to 0
      for (const std::string &string_var: m_Bvarnames) {
        m_Bbranches.at(string_var).set(*event, false, sys);
      }

      // Retrive inputs
      const xAOD::PhotonContainer *photons = nullptr;
      ANA_CHECK (m_photonHandle.retrieve (photons, sys));
      
      const xAOD::JetContainer *bjets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (bjets, sys));
      
      N_HHPhotons = photons->size();
      N_HHBjets = bjets->size();

      // check if event falls into each analysis' loose selections, save variables to event info
      if(N_HHBjets == 2 and N_HHPhotons >= 2) m_Bbranches.at("orth_pass_bbyy").set(*event,1,sys);
      else if(N_HHBjets >= 3) m_Bbranches.at("orth_pass_bbbb").set(*event,1,sys);
      else if(N_HHBjets == 2 and N_HHPhotons < 2) m_Bbranches.at("orth_pass_bbtt").set(*event,1,sys);
      
    }
    
    return StatusCode::SUCCESS;
  }

}