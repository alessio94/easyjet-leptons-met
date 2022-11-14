/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Frederic Renner

#include "JetTruthMatcherAlg.h"
#include "AthContainers/AuxElement.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "xAODTruth/TruthParticleContainer.h"
#include <AthContainers/ConstDataVector.h>
#include <xAODJet/JetContainer.h>

namespace HH4B
{
  JetTruthMatcherAlg ::JetTruthMatcherAlg(const std::string &name,
                                          ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("bTagWP", m_bTagWP);
    declareProperty("regime", m_regime);
    declareProperty("containerOutKey", m_containerOutKey);
  }

  StatusCode JetTruthMatcherAlg ::initialize()
  {
    if (m_regime == "resolved")
    {
      m_resolved = true;
    }
    if (m_regime == "boosted")
    {
      m_boosted = true;
    }
    if (m_resolved)
    {
      ATH_CHECK(m_smallRContainerInKey.initialize());
    }
    else if (m_boosted)
    {
      ATH_CHECK(m_leadingLargeR_GA_VRJets.initialize());
      ATH_CHECK(m_subLeadingLargeR_GA_VRJets.initialize());
    }

    ATH_CHECK(m_EventInfoKey.initialize());
    ATH_CHECK(m_containerOutKey.initialize());

    // make decorators
    for (std::string var : m_vars)
    {
      std::string deco_var = m_regime + var + m_bTagWP;
      SG::AuxElement::Decorator<float> deco(deco_var);
      m_decos.emplace(deco_var, deco);
    };
    return StatusCode::SUCCESS;
  }

  StatusCode JetTruthMatcherAlg ::execute()
  {
    // this will hold the jets we want to truthmatch
    ConstDataVector<xAOD::JetContainer> jets(SG::VIEW_ELEMENTS);
    // get the jets for the analysis regime
    if (m_resolved)
    {
      SG::ReadHandle<xAOD::JetContainer> pairedJets(m_smallRContainerInKey);
      ATH_CHECK(pairedJets.isValid());
      for (const xAOD::Jet *jet : *pairedJets)
      {
        jets.push_back(jet);
      }
    }
    else if (m_boosted)
    {
      SG::ReadHandle<xAOD::JetContainer> h1_VRjets(m_leadingLargeR_GA_VRJets);
      SG::ReadHandle<xAOD::JetContainer> h2_VRjets(
          m_subLeadingLargeR_GA_VRJets);
      ATH_CHECK(h1_VRjets.isValid());
      ATH_CHECK(h2_VRjets.isValid());
      if ((*h1_VRjets).size() >= 2 && (*h2_VRjets).size() >= 2)
      {
        jets.push_back((*h1_VRjets)[0]);
        jets.push_back((*h1_VRjets)[1]);
        jets.push_back((*h2_VRjets)[0]);
        jets.push_back((*h2_VRjets)[1]);
      }
    }
    // some other containers we need
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    SG::ReadHandle<xAOD::TruthParticleContainer> truthBosons(
        "TruthBosonsWithDecayParticles");
    SG::ReadHandle<xAOD::TruthParticleContainer> truthBSM(
        "TruthBSMWithDecayParticles");
    ATH_CHECK(eventInfo.isValid());
    ATH_CHECK(truthBosons.isValid());
    ATH_CHECK(truthBSM.isValid());

    // we want to find the closest truth b's and its deltaR to the jets
    // this will be used to decorate four vectors with the JetSelectorAlg
    auto closestTruthBout =
        std::make_unique<ConstDataVector<xAOD::JetContainer>>(
            SG::VIEW_ELEMENTS);
    SG::WriteHandle<ConstDataVector<xAOD::JetContainer>> Writer(
        m_containerOutKey);
    // set default decorations
    for (std::string var : m_vars)
    {
      std::string deco_var = m_regime + var + m_bTagWP;
      m_decos.at(deco_var)(*eventInfo) = -1.;
    };

    // check if we have 4 jets
    if (jets.size() < 4)
    {
      ATH_CHECK(Writer.record(std::move(closestTruthBout)));
      return StatusCode::SUCCESS;
    }

    // collect initial truths: Higgs and Scalar
    ConstDataVector<xAOD::TruthParticleContainer> truthInitialParticles(
        SG::VIEW_ELEMENTS);
    for (const xAOD::TruthParticle *boson : *truthBosons)
    {
      // get Higgs
      if (boson->pdgId() == 25)
      {
        truthInitialParticles.push_back(boson);
      }
    }
    for (const xAOD::TruthParticle *bsm : *truthBSM)
    {
      // get Scalar
      if (bsm->pdgId() == 35)
      {
        truthInitialParticles.push_back(bsm);
      }
    }

    // get all truth B's from inital particles
    std::vector<const xAOD::TruthParticle *> truthBs;
    for (const xAOD::TruthParticle *tp : truthInitialParticles)
    {
      // get the b quark children
      for (size_t i = 0; i < tp->nChildren(); i++)
      {
        const xAOD::TruthParticle *thisChild = tp->child(i);
        // only collect the ones that come from the initial particles
        if (thisChild->absPdgId() == 5 &&
            thisChild->parent()->barcode() == tp->barcode())
        {
          truthBs.push_back(thisChild);
        }
      }
    }

    // find the closest truth b and its deltaR to the jet
    std::vector<const xAOD::TruthParticle *> closestTruthB;
    std::vector<float> closestTruthBdeltaR;
    for (const xAOD::Jet *jet : jets)
    {
      // calculate dR to truth B's from initial particles
      std::vector<float> dRtoTruthBs;
      for (const xAOD::TruthParticle *truthB : truthBs)
      {
        dRtoTruthBs.push_back(xAOD::P4Helpers::deltaR(truthB, jet));
      }
      // find the closest one to this jet
      std::vector<float>::iterator it =
          std::min_element(std::begin(dRtoTruthBs), std::end(dRtoTruthBs));
      int closestBindex = std::distance(std::begin(dRtoTruthBs), it);
      closestTruthB.push_back(truthBs[closestBindex]);
      closestTruthBdeltaR.push_back(dRtoTruthBs[closestBindex]);
    }

    // check if closest truth b's to the higgs candidate jets come from same
    // initial particle. The matching will only become useful with the further
    // down dR criterion for the closeness of the truths. This assumes as input
    // the jet order from the JetPairingAlg.

    // clang-format off
    if (closestTruthB[0]->parent()->barcode() ==
        closestTruthB[1]->parent()->barcode())
    {
      m_decos.at(m_regime + "_h1_closestTruthBsHaveSameInitialParticle_" + m_bTagWP)(*eventInfo) = 1.;
    }
    else
    {
      m_decos.at(m_regime + "_h1_closestTruthBsHaveSameInitialParticle_" + m_bTagWP)(*eventInfo) = 0.;
    }

    if (closestTruthB[2]->parent()->barcode() ==
        closestTruthB[3]->parent()->barcode())
    {
      m_decos.at(m_regime + "_h2_closestTruthBsHaveSameInitialParticle_" + m_bTagWP)(*eventInfo) = 1.;
    }
    else
    {
      m_decos.at(m_regime + "_h2_closestTruthBsHaveSameInitialParticle_" + m_bTagWP)(*eventInfo) = 0.;
    }

    m_decos.at(m_regime + "_h1_dR_leadingJet_closestTruthB_" + m_bTagWP)(*eventInfo) = closestTruthBdeltaR[0];
    m_decos.at(m_regime + "_h1_dR_subleadingJet_closestTruthB_" + m_bTagWP)(*eventInfo) = closestTruthBdeltaR[1];
    m_decos.at(m_regime + "_h2_dR_leadingJet_closestTruthB_" + m_bTagWP)(*eventInfo) = closestTruthBdeltaR[2];
    m_decos.at(m_regime + "_h2_dR_subleadingJet_closestTruthB_" + m_bTagWP)(*eventInfo) = closestTruthBdeltaR[3];
    m_decos.at(m_regime + "_h1_parentPdgId_leadingJet_closestTruthB_" + m_bTagWP)(*eventInfo) = closestTruthB[0]->parent()->pdgId();
    m_decos.at(m_regime + "_h1_parentPdgId_subleadingJet_closestTruthB_" + m_bTagWP)(*eventInfo) = closestTruthB[1]->parent()->pdgId();
    m_decos.at(m_regime + "_h2_parentPdgId_leadingJet_closestTruthB_" + m_bTagWP)(*eventInfo) = closestTruthB[2]->parent()->pdgId();
    m_decos.at(m_regime + "_h2_parentPdgId_subleadingJet_closestTruthB_" + m_bTagWP)(*eventInfo) = closestTruthB[3]->parent()->pdgId();
    // clang-format on

    // I know this is not so nice, but it does the job:
    // write matched truths as jets to eventStore to write with
    // jetSelectorAlg four vector info
    for (const xAOD::TruthParticle *b : closestTruthB)
    {
      const xAOD::IParticle *ptcl = static_cast<const xAOD::IParticle *>(b);
      const xAOD::Jet *jet = static_cast<const xAOD::Jet *>(ptcl);
      closestTruthBout->push_back(jet);
    }

    ATH_CHECK(Writer.record(std::move(closestTruthBout)));

    return StatusCode::SUCCESS;
  }
}
