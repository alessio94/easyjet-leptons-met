/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Frederic Renner

#include "FinalVarsAlg.h"
#include "AthContainers/AuxElement.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include <AthContainers/ConstDataVector.h>
#include <xAODJet/JetContainer.h>

namespace HH4B
{
  FinalVarsAlg ::FinalVarsAlg(const std::string &name,
                              ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("doDiHiggsResolved", m_doDiHiggsResolved);
    declareProperty("doDiHiggsBoosted", m_doDiHiggsBoosted);
    declareProperty("bTagWP", m_bTagWP);
  }

  StatusCode FinalVarsAlg ::initialize()
  {
    if (m_doDiHiggsResolved)
    {
      ATH_CHECK(m_smallRContainerInKey.initialize());
    }
    if (m_doDiHiggsBoosted)
    {
      ATH_CHECK(m_largeRContainerInKey.initialize());
      ATH_CHECK(m_leadingLargeR_GA_VRJets.initialize());
      ATH_CHECK(m_subLeadingLargeR_GA_VRJets.initialize());
    }
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

  StatusCode FinalVarsAlg ::execute()
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

    if (m_doDiHiggsResolved)
    {
      SG::ReadHandle<ConstDataVector<xAOD::JetContainer>> smallRjets(
          m_smallRContainerInKey);
      ATH_CHECK(smallRjets.isValid());

      ConstDataVector<xAOD::JetContainer> jets = *smallRjets;
      // check if we have 4 btagged small R jets
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
    }

    if (m_doDiHiggsBoosted)
    {
      SG::ReadHandle<ConstDataVector<xAOD::JetContainer>> largeRjets(
          m_largeRContainerInKey);
      ATH_CHECK(largeRjets.isValid());
      SG::ReadHandle<ConstDataVector<xAOD::JetContainer>> leadingVRjets(
          m_leadingLargeR_GA_VRJets);
      ATH_CHECK(leadingVRjets.isValid());
      SG::ReadHandle<ConstDataVector<xAOD::JetContainer>> subleadingVRjets(
          m_subLeadingLargeR_GA_VRJets);
      ATH_CHECK(subleadingVRjets.isValid());
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
    }

    return StatusCode::SUCCESS;
  }
}
