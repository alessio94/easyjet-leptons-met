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
  }

  StatusCode JetTruthMatcherAlg ::initialize()
  {
    ATH_CHECK(m_containerInKey.initialize());
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

  StatusCode JetTruthMatcherAlg ::execute()
  {
    // container we read in
    SG::ReadHandle<xAOD::JetContainer> inContainer(m_containerInKey);
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    SG::ReadHandle<xAOD::TruthParticleContainer> truthBosons(
        "TruthBosonsWithDecayParticles");
    SG::ReadHandle<xAOD::TruthParticleContainer> truthBSM(
        "TruthBSMWithDecayParticles");
    ATH_CHECK(inContainer.isValid());
    ATH_CHECK(eventInfo.isValid());
    ATH_CHECK(truthBosons.isValid());
    ATH_CHECK(truthBSM.isValid());

    // set default final final decorations
    for (std::string var : m_vars)
    {
      std::string deco_var = var + m_bTagWP;
      m_decos.at(deco_var)(*eventInfo) = -1.;
    };

    // check if we have 4 paired jets
    if ((*inContainer).size() < 4)
    {
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
    for (const xAOD::Jet *jet : *inContainer)
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
      m_decos.at("resolved_h1_closestTruthBsHaveSameInitialParticle_" + m_bTagWP)(*eventInfo) = 1.;
    }
    else
    {
      m_decos.at("resolved_h1_closestTruthBsHaveSameInitialParticle_" + m_bTagWP)(*eventInfo) = 0.;
    }

    if (closestTruthB[2]->parent()->barcode() ==
        closestTruthB[3]->parent()->barcode())
    {
      m_decos.at("resolved_h2_closestTruthBsHaveSameInitialParticle_" + m_bTagWP)(*eventInfo) = 1.;
    }
    else
    {
      m_decos.at("resolved_h2_closestTruthBsHaveSameInitialParticle_" + m_bTagWP)(*eventInfo) = 0.;
    }

    m_decos.at("resolved_h1_dR_leadingJet_closestTruthB_" + m_bTagWP)(*eventInfo) = closestTruthBdeltaR[0];
    m_decos.at("resolved_h1_dR_subleadingJet_closestTruthB_" + m_bTagWP)(*eventInfo) = closestTruthBdeltaR[1];
    m_decos.at("resolved_h2_dR_leadingJet_closestTruthB_" + m_bTagWP)(*eventInfo) = closestTruthBdeltaR[2];
    m_decos.at("resolved_h2_dR_subleadingJet_closestTruthB_" + m_bTagWP)(*eventInfo) = closestTruthBdeltaR[3];
    // clang-format on

    return StatusCode::SUCCESS;
  }
}
