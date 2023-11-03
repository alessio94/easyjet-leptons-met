/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Frederic Renner

#include "GhostAssocVRJetGetterAlg.h"
#include "AthContainers/AuxElement.h"
#include "AthContainers/DataVector.h"
#include <xAODJet/JetContainer.h>

namespace Easyjet
{
  GhostAssocVRJetGetterAlg ::GhostAssocVRJetGetterAlg(const std::string &name,
                                                      ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator) { }

  StatusCode GhostAssocVRJetGetterAlg ::initialize()
  {
    ATH_CHECK(m_containerInKey.initialize());
    ATH_CHECK(m_EventInfoKey.initialize());
    ATH_CHECK(m_containerOutKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode GhostAssocVRJetGetterAlg ::execute()
  {
    // container we read in
    SG::ReadHandle<xAOD::JetContainer> inContainer(m_containerInKey);
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    ATH_CHECK(inContainer.isValid());
    ATH_CHECK(eventInfo.isValid());

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
        SG::VIEW_ELEMENTS);
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
    SG::WriteHandle<ConstDataVector<xAOD::JetContainer>> Writer(
        m_containerOutKey);
    ATH_CHECK(Writer.record(std::move(workContainer)));

    return StatusCode::SUCCESS;
  }
}
