/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Frederic Renner

#include "BaselineVarsBoostedAlg.h"
#include "AthContainers/AuxElement.h"
#include "FourMomUtils/xAODP4Helpers.h"

namespace HH4B
{
  BaselineVarsBoostedAlg ::BaselineVarsBoostedAlg(const std::string &name,
                                                  ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode BaselineVarsBoostedAlg ::initialize()
  {
    ATH_CHECK (m_LargeJetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_leadingLargeR_GA_VRJetsHandle.initialize(m_systematicsList));
    ATH_CHECK (m_subleadingLargeR_GA_VRJetsHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    // make decorators
    for (const std::string& var : m_vars)
    {
      std::string deco_var = var + m_bTagWP;
      CP::SysWriteDecorHandle<float> deco{deco_var + "_%SYS%", this};
      m_decos.emplace(deco_var, deco);
      ATH_CHECK (m_decos.at(deco_var).initialize(m_systematicsList, m_eventHandle));
    };
    // Initialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsBoostedAlg ::execute()
  {
    for (const auto& sys : m_systematicsList.systematicsVector()) {
      // container we read in
      const xAOD::EventInfo *eventInfo = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (eventInfo, sys));

      const xAOD::JetContainer *largeRjets = nullptr;
      ANA_CHECK (m_LargeJetHandle.retrieve (largeRjets, sys));

      const xAOD::JetContainer *leadingVRjets = nullptr;
      ANA_CHECK (m_leadingLargeR_GA_VRJetsHandle.retrieve (leadingVRjets, sys));

      const xAOD::JetContainer *subleadingVRjets = nullptr;
      ANA_CHECK (m_subleadingLargeR_GA_VRJetsHandle.retrieve (subleadingVRjets, sys));

      // set defaults
      for (const std::string& var : m_vars)
      {
        std::string deco_var = var + m_bTagWP;
        m_decos.at(deco_var).set(*eventInfo, -1, sys);
      };

      // check if we have 2 large R's and two btagged VR jets in each
      if (largeRjets->size() >= 2 && leadingVRjets->size() >= 2 &&
          subleadingVRjets->size() >= 2)
      {
        // construct Higgs Candidates
        xAOD::JetFourMom_t h1 = (*largeRjets)[0]->jetP4();
        xAOD::JetFourMom_t h2 = (*largeRjets)[1]->jetP4();
        float h1_dR_jets = xAOD::P4Helpers::deltaR((*leadingVRjets)[0],(*leadingVRjets)[1]);
        float h2_dR_jets = xAOD::P4Helpers::deltaR((*subleadingVRjets)[0],(*subleadingVRjets)[1]);
        // decorate eventinfo
        // clang-format off
        m_decos.at("boosted_h1_m_" + m_bTagWP).set(*eventInfo, h1.M(), sys);
        m_decos.at("boosted_h1_jet1_pt_" + m_bTagWP).set(*eventInfo, (*leadingVRjets)[0]->pt(), sys);
        m_decos.at("boosted_h1_jet2_pt_" + m_bTagWP).set(*eventInfo, (*leadingVRjets)[1]->pt(), sys);
        m_decos.at("boosted_h1_dR_jets_" + m_bTagWP).set(*eventInfo, h1_dR_jets, sys);
        m_decos.at("boosted_h2_m_" + m_bTagWP).set(*eventInfo, h2.M(), sys);
        m_decos.at("boosted_h2_jet1_pt_" + m_bTagWP).set(*eventInfo, (*subleadingVRjets)[0]->pt(), sys);
        m_decos.at("boosted_h2_jet2_pt_" + m_bTagWP).set(*eventInfo, (*subleadingVRjets)[1]->pt(), sys);
        m_decos.at("boosted_h2_dR_jets_" + m_bTagWP).set(*eventInfo, h2_dR_jets, sys);
        m_decos.at("boosted_hh_m_" + m_bTagWP).set(*eventInfo, (h1 + h2).M(), sys);
        // clang-format on
      }
    }
    return StatusCode::SUCCESS;
  }
}
