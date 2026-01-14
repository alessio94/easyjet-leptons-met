/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Abraham Tishelman-Charny

#include "OrthogonalityAlg.h"

namespace Easyjet
{

  // 
  enum ORTH_CHANNEL : uint8_t {BBYY, BBBB, BBTT, N_CHANNELS};
  std::vector<std::string> Bvarnames = {"orth_pass_bbyy", "orth_pass_bbbb", "orth_pass_bbtt"};

  OrthogonalityAlg::OrthogonalityAlg(const std::string &name,
                                       ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator),
      m_Bbranches{Bvarnames,this}
      { }

  StatusCode OrthogonalityAlg::initialize()
  {
    
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("       OrthogonalityAlg       \n");
    ATH_MSG_INFO("*********************************\n");

    ATH_CHECK( Bvarnames.size() == N_CHANNELS );

    ATH_CHECK (m_photonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    
    ATH_CHECK (m_Bbranches.initialize(m_systematicsList, m_eventHandle)); 
    
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
      for (uint8_t ivar=0; ivar<N_CHANNELS; ++ivar) {
        m_Bbranches.at(ivar).set(*event, false, sys);
      }

      // Retrive inputs
      const xAOD::PhotonContainer *photons = nullptr;
      ANA_CHECK (m_photonHandle.retrieve (photons, sys));
      
      const xAOD::JetContainer *bjets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (bjets, sys));
      
      N_HHPhotons = photons->size();
      N_HHBjets = bjets->size();

      // check if event falls into each analysis' loose selections, save variables to event info
      if(N_HHBjets == 2 and N_HHPhotons >= 2) m_Bbranches.at(BBYY).set(*event,1,sys);
      else if(N_HHBjets >= 3) m_Bbranches.at(BBBB).set(*event,1,sys);
      else if(N_HHBjets == 2 and N_HHPhotons < 2) m_Bbranches.at(BBTT).set(*event,1,sys);
      
    }
    
    return StatusCode::SUCCESS;
  }

}