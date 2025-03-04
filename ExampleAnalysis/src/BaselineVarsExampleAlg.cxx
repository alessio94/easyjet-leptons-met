/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "BaselineVarsExampleAlg.h"

namespace EXANA
{
  BaselineVarsExampleAlg ::BaselineVarsExampleAlg(const std::string &name,
                                                  ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode BaselineVarsExampleAlg ::initialize()
  {
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK (m_JetHandle.initialize(m_systematicsList));
    //ATH_CHECK (m_LargeRJetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_photonHandle.initialize(m_systematicsList));

    // Intialise syst-aware output decorators
    for (const std::string &var : m_floatVariables) {
      CP::SysWriteDecorHandle<float> whandle{var+"_%SYS%", this};
      m_Fbranches.emplace(var, whandle);
      ATH_CHECK (m_Fbranches.at(var).initialize(m_systematicsList, m_eventHandle));
    }

    for (const std::string &var : m_intVariables){
      CP::SysWriteDecorHandle<int> whandle{var+"_%SYS%", this};
      m_Ibranches.emplace(var, whandle);
      ATH_CHECK(m_Ibranches.at(var).initialize(m_systematicsList, m_eventHandle));
    };

    // Initialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsExampleAlg::execute()
  {
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      //const xAOD::JetContainer *lrjets = nullptr;
      //ANA_CHECK (m_LargeRJetHandle.retrieve (lrjets, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_JetHandle.retrieve (jets, sys));

      const xAOD::PhotonContainer *photons = nullptr;
      ANA_CHECK (m_photonHandle.retrieve (photons, sys));

      for (const std::string &string_var: m_floatVariables) {
        m_Fbranches.at(string_var).set(*event, -99., sys);
      }

      for (const auto& var: m_intVariables) {
        m_Ibranches.at(var).set(*event, -99, sys);
      }

      float mjj = -99;
      if (jets->size() >= 2) {
        TLorentzVector jet_pair = jets->at(0)->p4() + jets->at(1)->p4();
        mjj = jet_pair.M();
      }

      m_Fbranches.at("mjj").set(*event, mjj, sys);
      m_Ibranches.at("n_photons").set(*event, photons->size(), sys);
    }

    return StatusCode::SUCCESS;
  }
}
