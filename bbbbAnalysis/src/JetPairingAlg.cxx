/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Frederic Renner

#include "JetPairingAlg.h"
#include "AthContainers/AuxElement.h"
#include <AthContainers/ConstDataVector.h>
#include "FourMomUtils/xAODP4Helpers.h"

namespace HH4B
{
  JetPairingAlg ::JetPairingAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode JetPairingAlg ::initialize()
  {
    ATH_CHECK(m_inJetHandle.initialize(m_systematicsList));
    ATH_CHECK(m_outJetHandle.initialize(m_systematicsList));

    ATH_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode JetPairingAlg ::execute()
  {
    // Loop over all systs
    for (const auto &sys : m_systematicsList.systematicsVector())
    {
      // Retrieve inputs
      const xAOD::JetContainer *inContainer = nullptr;
      ATH_CHECK(m_inJetHandle.retrieve(inContainer, sys));

      // fill workContainer with "views" of the inContainer
      // see TJ's tutorial for this
      auto workContainer = std::make_unique<ConstDataVector<xAOD::JetContainer>>(
          inContainer->begin(), inContainer->end(), SG::VIEW_ELEMENTS);

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
      }
      ATH_CHECK(m_outJetHandle.record(std::move(workContainer), sys));
    }
    return StatusCode::SUCCESS;
  }
}
