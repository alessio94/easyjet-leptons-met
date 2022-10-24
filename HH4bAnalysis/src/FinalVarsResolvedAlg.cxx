/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Frederic Renner

#include "FinalVarsResolvedAlg.h"
#include "AthContainers/AuxElement.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include <AthContainers/ConstDataVector.h>
#include <xAODJet/JetContainer.h>

namespace HH4B
{
  FinalVarsResolvedAlg ::FinalVarsResolvedAlg(const std::string &name,
                                              ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("bTagWP", m_bTagWP);
  }

  StatusCode FinalVarsResolvedAlg ::initialize()
  {
    ATH_CHECK(m_smallRContainerInKey.initialize());
    ATH_CHECK(m_EventInfoKey.initialize());

    // make decorators
    for (std::string var : m_vars)
    {
      std::string deco_var = var + m_bTagWP;
      SG::AuxElement::Decorator<float> deco(deco_var);
      m_decos.emplace(deco_var, deco);
    };
    return StatusCode::SUCCESS;
  }

  StatusCode FinalVarsResolvedAlg ::execute()
  {
    // container we read in
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    ATH_CHECK(eventInfo.isValid());

    // set defaults
    for (std::string var : m_vars)
    {
      std::string deco_var = var + m_bTagWP;
      m_decos.at(deco_var)(*eventInfo) = -1.;
    };

    SG::ReadHandle<ConstDataVector<xAOD::JetContainer>> smallRjets(
        m_smallRContainerInKey);
    ATH_CHECK(smallRjets.isValid());

    ConstDataVector<xAOD::JetContainer> jets = *smallRjets;
    // check if we have 4 small R jets
    if (jets.size() >= 4)
    {
      // construct Higgs Candidates
      xAOD::JetFourMom_t h1 = jets[0]->jetP4() + jets[1]->jetP4();
      xAOD::JetFourMom_t h2 = jets[2]->jetP4() + jets[3]->jetP4();

      // decorate eventinfo
      // clang-format off
      m_decos.at("resolved_DeltaR12_" + m_bTagWP)(*eventInfo) = xAOD::P4Helpers::deltaR(jets[0],jets[1]);
      m_decos.at("resolved_DeltaR13_" + m_bTagWP)(*eventInfo) = xAOD::P4Helpers::deltaR(jets[0],jets[2]);
      m_decos.at("resolved_DeltaR14_" + m_bTagWP)(*eventInfo) = xAOD::P4Helpers::deltaR(jets[0],jets[3]);
      m_decos.at("resolved_DeltaR23_" + m_bTagWP)(*eventInfo) = xAOD::P4Helpers::deltaR(jets[1],jets[2]);
      m_decos.at("resolved_DeltaR24_" + m_bTagWP)(*eventInfo) = xAOD::P4Helpers::deltaR(jets[1],jets[3]);
      m_decos.at("resolved_DeltaR34_" + m_bTagWP)(*eventInfo) = xAOD::P4Helpers::deltaR(jets[2],jets[3]);
      m_decos.at("resolved_h1_m_" + m_bTagWP)(*eventInfo) = h1.M();
      m_decos.at("resolved_h2_m_" + m_bTagWP)(*eventInfo) = h2.M();
      m_decos.at("resolved_hh_m_" + m_bTagWP)(*eventInfo) = (h1 + h2).M();
      // clang-format on
    }

    return StatusCode::SUCCESS;
  }
}
