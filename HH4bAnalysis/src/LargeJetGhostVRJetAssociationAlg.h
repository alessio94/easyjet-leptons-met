/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HH4BANALYSIS_LARGEJETGHOSTVRJETASSOCIATIONALG
#define HH4BANALYSIS_LARGEJETGHOSTVRJETASSOCIATIONALG

#include <AthenaBaseComps/AthAlgorithm.h>

#include "AthContainers/ConstDataVector.h"
#include <AthContainers/AuxElement.h>
#include <limits> // NAN
#include <xAODCore/ShallowCopy.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>

// Class definition

namespace HH4B
{

  /// \brief An algorithm for dumping variables
  class LargeJetGhostVRJetAssociationAlg final : public AthAlgorithm
  {
    /// \brief The standard constructor
public:
    LargeJetGhostVRJetAssociationAlg(const std::string &name,
                                     ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    using JC = xAOD::JetContainer;
    using ELPC = ElementLink<xAOD::IParticleContainer>;

    StatusCode recordVRTrackJetGhostAssociation(const JC &) const;

    SG::ReadHandleKey<JC> m_largeJetInKey{
        this, "LargeJetInKey", "", "the large-R jet collection to run on"};

    // ghost associated VR track jets are only on the untrimmed 1.0 jets
    SG::AuxElement::ConstAccessor<ElementLink<JC>> m_largeRUntrimmedAccessor{
        "Parent"};

    SG::AuxElement::ConstAccessor<std::vector<ELPC>>
        m_ghostVRTrackJetsAccessor{"GhostAntiKtVR30Rmax4Rmin02PV0TrackJets"};

    // recommended by ftag : Remove the event if any of your signal jets have
    // relativeDeltaRToVRJet = radius(jet_i)/min(dR(jet_i,jet_j)) < 1.0.
    // checks if any of the vr jets overlap
    SG::AuxElement::ConstAccessor<float> relativeDeltaRToVRJet{
        "relativeDeltaRToVRJet"};

    SG::AuxElement::Decorator<int> m_goodVRTrackJetCountDecorator{
        "goodVRTrackJets"};

    SG::AuxElement::Decorator<std::vector<float>>
        m_leadingVRTrackJetPtDecorator{"leadingVRTrackJetsPt"};

    SG::AuxElement::Decorator<std::vector<float>>
        m_leadingVRTrackJetEtaDecorator{"leadingVRTrackJetsEta"};

    SG::AuxElement::Decorator<std::vector<float>>
        m_leadingVRTrackJetPhiDecorator{"leadingVRTrackJetsPhi"};

    SG::AuxElement::Decorator<std::vector<float>>
        m_leadingVRTrackJetMDecorator{"leadingVRTrackJetsM"};

    std::vector<SG::AuxElement::Decorator<std::vector<char>>>
        m_leadingVRTrackJetBtagDecorators;

    std::vector<SG::AuxElement::ConstAccessor<char>> m_isBtagAccessors;

    std::vector<std::string> m_workingPoints;

    bool m_isMC;
  };
}

#endif
