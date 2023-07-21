/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Frederic Renner

#include "AthContainers/AuxElement.h"
#include "BaselineVarsBoostedAlg.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include <AthContainers/ConstDataVector.h>
#include <xAODJet/JetContainer.h>

namespace HH4B
{
  BaselineVarsBoostedAlg ::BaselineVarsBoostedAlg(const std::string &name,
                                                  ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("bTagWP", m_bTagWP);
  }

  StatusCode BaselineVarsBoostedAlg ::initialize()
  {
    ATH_CHECK(m_largeRContainerInKey.initialize());
    ATH_CHECK(m_leadingLargeR_GA_VRJets.initialize());
    ATH_CHECK(m_subLeadingLargeR_GA_VRJets.initialize());
    ATH_CHECK(m_EventInfoKey.initialize());

    // make decorators
    for (const std::string& var : m_vars)
    {
      std::string deco_var = var + m_bTagWP;
      SG::AuxElement::Decorator<float> deco(deco_var);
      m_decos.emplace(deco_var, deco);
    };
    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsBoostedAlg ::execute()
  {
    // container we read in
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    ATH_CHECK(eventInfo.isValid());
    SG::ReadHandle<ConstDataVector<xAOD::JetContainer>> largeRjets(
        m_largeRContainerInKey);
    ATH_CHECK(largeRjets.isValid());
    SG::ReadHandle<ConstDataVector<xAOD::JetContainer>> leadingVRjets(
        m_leadingLargeR_GA_VRJets);
    ATH_CHECK(leadingVRjets.isValid());
    SG::ReadHandle<ConstDataVector<xAOD::JetContainer>> subleadingVRjets(
        m_subLeadingLargeR_GA_VRJets);
    ATH_CHECK(subleadingVRjets.isValid());

    // set defaults
    for (const std::string& var : m_vars)
    {
      std::string deco_var = var + m_bTagWP;
      m_decos.at(deco_var)(*eventInfo) = -1.;
    };

    // check if we have 2 large R's and two btagged VR jets in each
    if (largeRjets->size() >= 2 && leadingVRjets->size() >= 2 &&
        subleadingVRjets->size() >= 2)
    {
      // construct Higgs Candidates
      xAOD::JetFourMom_t h1 = (*largeRjets)[0]->jetP4();
      xAOD::JetFourMom_t h2 = (*largeRjets)[1]->jetP4();
      // decorate eventinfo
      // clang-format off
      m_decos.at("boosted_h1_m_" + m_bTagWP)(*eventInfo) = h1.M();
      m_decos.at("boosted_h1_jet1_pt_" + m_bTagWP)(*eventInfo) = (*leadingVRjets)[0]->pt();
      m_decos.at("boosted_h1_jet2_pt_" + m_bTagWP)(*eventInfo) = (*leadingVRjets)[1]->pt();
      m_decos.at("boosted_h1_dR_jets_" + m_bTagWP)(*eventInfo) = xAOD::P4Helpers::deltaR((*leadingVRjets)[0],(*leadingVRjets)[1]);
      m_decos.at("boosted_h2_m_" + m_bTagWP)(*eventInfo) = h2.M();
      m_decos.at("boosted_h2_jet1_pt_" + m_bTagWP)(*eventInfo) = (*subleadingVRjets)[0]->pt();
      m_decos.at("boosted_h2_jet2_pt_" + m_bTagWP)(*eventInfo) = (*subleadingVRjets)[1]->pt();
      m_decos.at("boosted_h2_dR_jets_" + m_bTagWP)(*eventInfo) = xAOD::P4Helpers::deltaR((*subleadingVRjets)[0],(*subleadingVRjets)[1]);
      m_decos.at("boosted_hh_m_" + m_bTagWP)(*eventInfo) = (h1 + h2).M();
      // clang-format on
    }

    return StatusCode::SUCCESS;
  }
}
