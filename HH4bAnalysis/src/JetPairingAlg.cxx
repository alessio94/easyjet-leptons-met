/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Frederic Renner

#include "JetPairingAlg.h"
#include "AthContainers/AuxElement.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "xAODJet/JetContainer.h"
#include <AthContainers/ConstDataVector.h>

namespace HH4B
{
  JetPairingAlg ::JetPairingAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("pairingStrategy", m_pairingStrategy);
  }

  StatusCode JetPairingAlg ::initialize()
  {
    ATH_CHECK(m_containerInKey.initialize());
    ATH_CHECK(m_EventInfoKey.initialize());
    ATH_CHECK(m_containerOutKey.initialize());

    // make decorators for the four vectors
    std::vector<std::string> vars{
        "pt",
        "eta",
        "phi",
        "m",
    };
    for (std::string var : vars)
    {
      std::string deco_var = m_containerOutKey.key() + "_" + var;
      SG::AuxElement::Decorator<std::vector<float>> deco(deco_var);
      m_fourVecDecos.emplace(deco_var, deco);
    };

    return StatusCode::SUCCESS;
  }

  StatusCode JetPairingAlg ::execute()
  {
    // container we read in
    SG::ReadHandle<ConstDataVector<xAOD::JetContainer>> inContainer(
        m_containerInKey);
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    ATH_CHECK(inContainer.isValid());
    ATH_CHECK(eventInfo.isValid());

    // fill workContainer with "views" of the inContainer
    // see TJ's tutorial for this
    auto workContainer = std::make_unique<ConstDataVector<xAOD::JetContainer>>(
        inContainer->begin(), inContainer->end(), SG::VIEW_ELEMENTS);

    // decorate eventInfo
    std::vector<float> jet_pt;
    std::vector<float> jet_eta;
    std::vector<float> jet_phi;
    std::vector<float> jet_m;

    // set defaults
    jet_pt.push_back(-100);
    jet_eta.push_back(-100);
    jet_phi.push_back(-100);
    jet_m.push_back(-100);

    // this assumes that container is pt sorted (use the JetSelectorAlg for
    // this) and checks if we have at least 4 jets otherwise exit this alg
    if (m_pairingStrategy == "minDeltaR" && workContainer->size() >= 4)
    {
      // calculate dR to the next three leading jets and decorate jet
      const SG::AuxElement::Decorator<float> dRtoLeadingJet_dec(
          "dRtoLeadingJet");
      const SG::AuxElement::ConstAccessor<float> dRtoLeadingJet_acc(
          "dRtoLeadingJet");

      // decorate dR(jet,leading jet) to each jet
      bool firstJet = true;
      for (const xAOD::Jet *jet : *workContainer)
      {
        // more instructive than done with iterators
        if (firstJet)
        {
          dRtoLeadingJet_dec(*jet) = 0;
          firstJet = false;
          continue;
        }
        dRtoLeadingJet_dec(*jet) =
            xAOD::P4Helpers::deltaR(jet, (*workContainer)[0]);
      }

      // now sort them for their closeness
      std::partial_sort(
          workContainer->begin(),     // Iterator from which to start sorting
          workContainer->begin() + 4, // Use begin + N to sort first N
          workContainer->end(),       // Iterator marking the end of the range
          [dRtoLeadingJet_dec, dRtoLeadingJet_acc](
              const xAOD::IParticle *left, const xAOD::IParticle *right)
          { return dRtoLeadingJet_acc(*left) < dRtoLeadingJet_acc(*right); });

      // lets return the pairing of the leading (h1) and subleading (h2) Higgs
      // candidates as four jets in the order:
      // h1_leading_pt_jet
      // h1_subleading_pt_jet
      // h2_leading_pt_jet
      // h2_subleading_pt_jet
      if ((*workContainer)[2]->pt() < (*workContainer)[3]->pt())
      {
        // no swap method on ConstDataVector, so by hand
        const xAOD::Jet *temp = (*workContainer)[2];
        (*workContainer)[2] = (*workContainer)[3];
        (*workContainer)[3] = temp;
      }
      // keep only the higgs candidate ones to avoid confusion
      workContainer->erase(workContainer->begin() + 4, workContainer->end());

      // decorate
      jet_pt.clear();
      jet_eta.clear();
      jet_phi.clear();
      jet_m.clear();
      for (const xAOD::Jet *jet : *workContainer)
      {
        jet_pt.push_back(jet->pt());
        jet_eta.push_back(jet->eta());
        jet_phi.push_back(jet->phi());
        jet_m.push_back(jet->m());
      }
    }

    // clang-format off
      m_fourVecDecos.at(m_containerOutKey.key() + "_pt")(*eventInfo) = jet_pt;
      m_fourVecDecos.at(m_containerOutKey.key() + "_eta")(*eventInfo) = jet_eta;
      m_fourVecDecos.at(m_containerOutKey.key() + "_phi")(*eventInfo) = jet_phi;
      m_fourVecDecos.at(m_containerOutKey.key() + "_m")(*eventInfo) = jet_m;
    // clang-format on

    // write to EventStore
    SG::WriteHandle<ConstDataVector<xAOD::JetContainer>> Writer(
        m_containerOutKey);
    ATH_CHECK(Writer.record(std::move(workContainer)));

    return StatusCode::SUCCESS;
  }
}
