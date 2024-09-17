/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Frederic Renner

#include "GhostAssocVRJetGetterAlg.h"
#include "AthContainers/AuxElement.h"

namespace Easyjet
{
  GhostAssocVRJetGetterAlg ::GhostAssocVRJetGetterAlg(const std::string &name,
                                                      ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator) { }

  StatusCode GhostAssocVRJetGetterAlg ::initialize()
  {
    ATH_CHECK(m_inJetHandle.initialize(m_systematicsList));
    ATH_CHECK(m_outJetHandle.initialize(m_systematicsList));

    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode GhostAssocVRJetGetterAlg ::execute()
  {
    // Loop over all systs
    for (const auto &sys : m_systematicsList.systematicsVector())
    {
      const xAOD::JetContainer *inContainer = nullptr;
      ATH_CHECK(m_inJetHandle.retrieve(inContainer, sys));

      // the accessors
      SG::AuxElement::ConstAccessor<ElementLink<xAOD::JetContainer>>
          m_acc_largeR_untrimmed("Parent");
      SG::AuxElement::ConstAccessor<
          std::vector<ElementLink<xAOD::IParticleContainer>>>
          m_acc_VRTrackJets("GhostAntiKtVR30Rmax4Rmin02PV0TrackJets");
      // get ghost associated VR track jets from untrimmed large R jet
      xAOD::JetContainer largeRJets = *inContainer;
      //  This will make a view container.
      auto workContainer = std::make_unique<ConstDataVector<xAOD::JetContainer>>(
          inContainer->begin(), inContainer->end(), SG::VIEW_ELEMENTS);
      // make sure we are not accessing a non existing index
      unsigned int LRsize = largeRJets.size();
      if (LRsize != 0 && LRsize > m_whichJet)
      {
        const xAOD::Jet *untrimmedLargeR =
            *m_acc_largeR_untrimmed(*largeRJets[m_whichJet]);
        std::vector<ElementLink<xAOD::IParticleContainer>> VRTrackjets =
            m_acc_VRTrackJets(*untrimmedLargeR);
        // I know this seems stupid, but we need it in the xAOD::JetContainer
        // format
        for (const ElementLink<xAOD::IParticleContainer>& vrJet : VRTrackjets)
        {
          const xAOD::Jet *jet = static_cast<const xAOD::Jet *>(*vrJet.cptr());
          workContainer->push_back(jet);
        }
      }
      // write to eventstore
      ATH_CHECK(m_outJetHandle.record(std::move(workContainer), sys));
    }
    return StatusCode::SUCCESS;
  }
}
