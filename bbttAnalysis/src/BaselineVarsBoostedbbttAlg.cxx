/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Giacomo Magni and Marko Mihovilovic

#include "BaselineVarsBoostedbbttAlg.h"

namespace HHBBTT
{
  BaselineVarsBoostedbbttAlg ::BaselineVarsBoostedbbttAlg(const std::string &name,
                                                  ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode BaselineVarsBoostedbbttAlg ::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("      BaselineVarsBoostedbbttAlg     \n");
    ATH_MSG_INFO("*********************************\n");

    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK (m_lRjetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_Pass_GN2X.initialize(m_systematicsList, m_lRjetHandle));

    // make decorators
    for (const std::string &string_var: m_floatVariables) {
      ATH_MSG_DEBUG("initializing float variable: " << string_var);
      CP::SysWriteDecorHandle<float> var {string_var+"_%SYS%", this};
      m_Fbranches.emplace(string_var, var);
      ATH_CHECK (m_Fbranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }

    // Initialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsBoostedbbttAlg ::execute()
  {
    for (const auto& sys : m_systematicsList.systematicsVector()) {
      // container we read in
      const xAOD::EventInfo *eventInfo = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (eventInfo, sys));

      const xAOD::JetContainer *largeRjets = nullptr;
      ANA_CHECK (m_lRjetHandle.retrieve (largeRjets, sys));
      std::size_t n_largeRjets = largeRjets->size();

      // set defaults
      for (const std::string& var : m_floatVariables) {
        m_Fbranches.at(var).set(*eventInfo, -99, sys);
      }

      // large jet sector
      if (n_largeRjets >= 2)
      {
        for (int i=0; i<2; i++) {
          TLorentzVector h_v4 = largeRjets->at(i)->p4();
          std::string prefix = "boosted_h"+std::to_string(i+1);
          m_Fbranches.at(prefix+"_m").set(*eventInfo, h_v4.M(), sys);
          m_Fbranches.at(prefix+"_pt").set(*eventInfo, h_v4.Pt(), sys);
          m_Fbranches.at(prefix+"_eta").set(*eventInfo, h_v4.Eta(), sys);
          m_Fbranches.at(prefix+"_phi").set(*eventInfo, h_v4.Phi(), sys);
        }

        const xAOD::Jet* b_jet = nullptr;
        const xAOD::Jet* tau_jet = nullptr;
        
        // TODO: __BOOSTED__ use tau tagging
        for (const xAOD::Jet *lRjet : *largeRjets) {
          if (m_Pass_GN2X.get(*lRjet, sys) && !b_jet) {
            b_jet = lRjet;
          } else if (!tau_jet) {
            tau_jet = lRjet;
          }
        }
        if (!tau_jet || !b_jet) continue;

        TLorentzVector h1_v4 = b_jet->p4();
        TLorentzVector h2_v4 = tau_jet->p4();
        TLorentzVector hh_v4 = h1_v4 + h2_v4;
        m_Fbranches.at("boosted_hh_m").set(*eventInfo, hh_v4.M(), sys);
        m_Fbranches.at("boosted_hh_pt").set(*eventInfo, hh_v4.Pt(), sys);
        m_Fbranches.at("boosted_hh_delta_eta").set(*eventInfo, h1_v4.Eta() - h2_v4.Eta(), sys);
        m_Fbranches.at("boosted_hh_delta_phi").set(*eventInfo, h1_v4.DeltaPhi(h2_v4), sys);
      }
    }
    return StatusCode::SUCCESS;
  }
}
