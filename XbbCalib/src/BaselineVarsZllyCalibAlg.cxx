/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author 

#include "BaselineVarsZllyCalibAlg.h"

namespace XBBCALIB
{
  BaselineVarsZllyCalibAlg::BaselineVarsZllyCalibAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  { }

  StatusCode BaselineVarsZllyCalibAlg::initialize()
  {
    ATH_CHECK (m_lrjetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK (m_photonHandle.initialize(m_systematicsList));

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

  StatusCode BaselineVarsZllyCalibAlg::execute()
  {
    //Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {

      // Initialize variables
      int n_jets = 0;
      int n_electrons = 0;
      int n_muons = 0;
      int n_photons = 0;

      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *lrjets = nullptr;
      ANA_CHECK (m_lrjetHandle.retrieve (lrjets, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));
      
      const xAOD::PhotonContainer *photons = nullptr;
      ANA_CHECK (m_photonHandle.retrieve(photons, sys));

      for (const std::string &string_var: m_floatVariables) {
        m_Fbranches.at(string_var).set(*event, -99., sys);
      }

      for (const auto& var: m_intVariables) {
        m_Ibranches.at(var).set(*event, -99, sys);
      }

      // Calculate vars

      // Count electrons
      n_electrons = electrons->size();

      // Count muons
      n_muons = muons->size();

      // Count jets
      n_jets = lrjets->size();
      
      // Count photons
      n_photons = photons->size();

      m_Ibranches.at("nJets").set(*event, n_jets, sys);
      m_Ibranches.at("nElectrons").set(*event, n_electrons, sys);
      m_Ibranches.at("nMuons").set(*event, n_muons, sys);
      m_Ibranches.at("nPhotons").set(*event, n_photons, sys);
      

    }
    return StatusCode::SUCCESS;
  }

}
