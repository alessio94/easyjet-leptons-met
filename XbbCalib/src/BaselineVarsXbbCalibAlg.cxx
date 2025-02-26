/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author 

#include "BaselineVarsXbbCalibAlg.h"

namespace XBBCALIB
{
  BaselineVarsXbbCalibAlg::BaselineVarsXbbCalibAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  { }

  StatusCode BaselineVarsXbbCalibAlg::initialize()
  {
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_lrjetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

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

  StatusCode BaselineVarsXbbCalibAlg::execute()
  {
    //Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      const xAOD::JetContainer *lrjets = nullptr;
      ANA_CHECK (m_lrjetHandle.retrieve (lrjets, sys));

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


      // Calculate vars

      // selected Probe Jet ;
      if (lrjets->size() >= 1)
      {
        m_Fbranches.at("probe_jet_pt").set(*event, lrjets->at(0)->pt(), sys);
        m_Fbranches.at("probe_jet_eta").set(*event, lrjets->at(0)->eta(), sys);
        m_Fbranches.at("probe_jet_phi").set(*event, lrjets->at(0)->phi(), sys);
      }

      if (jets->size() >= 1)
      {
        m_Fbranches.at("tag_jet_pt").set(*event, jets->at(0)->pt(), sys);
        m_Fbranches.at("tag_jet_eta").set(*event, jets->at(0)->eta(), sys);
        m_Fbranches.at("tag_jet_phi").set(*event, jets->at(0)->phi(), sys);
      }

    }
    return StatusCode::SUCCESS;
  }

}
