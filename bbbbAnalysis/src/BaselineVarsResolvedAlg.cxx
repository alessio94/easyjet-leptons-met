/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Frederic Renner

#include "BaselineVarsResolvedAlg.h"
#include "AthContainers/AuxElement.h"
#include "FourMomUtils/xAODP4Helpers.h"

namespace HH4B
{
  BaselineVarsResolvedAlg ::BaselineVarsResolvedAlg(const std::string &name,
                                                    ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode BaselineVarsResolvedAlg ::initialize()
  {
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    // make decorators
    for (const std::string& var : m_vars)
    {
      std::string deco_var = var + m_bTagWP;
      CP::SysWriteDecorHandle<float> deco{deco_var + "_%SYS%", this};
      m_decos.emplace(deco_var, deco);
      ATH_CHECK (m_decos.at(deco_var).initialize(m_systematicsList, m_eventHandle));
    }

    // Initialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsResolvedAlg ::execute()
  {
    for (const auto& sys : m_systematicsList.systematicsVector()) {
      const xAOD::EventInfo *eventInfo = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (eventInfo, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      // set defaults
      for (const std::string& var : m_vars)
      {
        std::string deco_var = var + m_bTagWP;
        m_decos.at(deco_var).set(*eventInfo, -1, sys);
      }

      // check if we have 4 small R jets
      if (jets->size() >= 4)
      {
        // construct Higgs Candidates
        xAOD::JetFourMom_t h1 = jets->at(0)->jetP4() + jets->at(1)->jetP4();
        xAOD::JetFourMom_t h2 = jets->at(2)->jetP4() + jets->at(3)->jetP4();

        // decorate eventinfo
        float deltaR12 = xAOD::P4Helpers::deltaR(jets->at(0),jets->at(1));
        float deltaR13 = xAOD::P4Helpers::deltaR(jets->at(0),jets->at(2));
        float deltaR14 = xAOD::P4Helpers::deltaR(jets->at(0),jets->at(3));
        float deltaR23 = xAOD::P4Helpers::deltaR(jets->at(1),jets->at(2));
        float deltaR24 = xAOD::P4Helpers::deltaR(jets->at(1),jets->at(3));
        float deltaR34 = xAOD::P4Helpers::deltaR(jets->at(2),jets->at(3));
        m_decos.at("resolved_DeltaR12_" + m_bTagWP).set(*eventInfo, deltaR12, sys);
        m_decos.at("resolved_DeltaR13_" + m_bTagWP).set(*eventInfo, deltaR13, sys);
        m_decos.at("resolved_DeltaR14_" + m_bTagWP).set(*eventInfo, deltaR14, sys);
        m_decos.at("resolved_DeltaR23_" + m_bTagWP).set(*eventInfo, deltaR23, sys);
        m_decos.at("resolved_DeltaR24_" + m_bTagWP).set(*eventInfo, deltaR24, sys);
        m_decos.at("resolved_DeltaR34_" + m_bTagWP).set(*eventInfo, deltaR34, sys);
        m_decos.at("resolved_h1_m_" + m_bTagWP).set(*eventInfo, h1.M(), sys);
        m_decos.at("resolved_h1_pt_" + m_bTagWP).set(*eventInfo, h1.Pt(), sys);
        m_decos.at("resolved_h1_eta_" + m_bTagWP).set(*eventInfo, h1.Eta(), sys);
        m_decos.at("resolved_h1_phi_" + m_bTagWP).set(*eventInfo, h1.Phi(), sys);
        m_decos.at("resolved_h2_m_" + m_bTagWP).set(*eventInfo, h2.M(), sys);
        m_decos.at("resolved_h2_pt_" + m_bTagWP).set(*eventInfo, h2.Pt(), sys);
        m_decos.at("resolved_h2_eta_" + m_bTagWP).set(*eventInfo, h2.Eta(), sys);
        m_decos.at("resolved_h2_phi_" + m_bTagWP).set(*eventInfo, h2.Phi(), sys);
        m_decos.at("resolved_hh_m_" + m_bTagWP).set(*eventInfo, (h1 + h2).M(), sys);
        m_decos.at("resolved_hh_pt_" + m_bTagWP).set(*eventInfo, (h1 + h2).Pt(), sys);
      m_decos.at("resolved_DeltaEtaHH_" + m_bTagWP).set(*eventInfo, std::abs(h1.Eta() - h2.Eta()), sys);
      }
    }
    return StatusCode::SUCCESS;
  }
}
