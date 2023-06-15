/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EASYJET_LARGEJETGHOSTVRJETASSOCIATIONALG
#define EASYJET_LARGEJETGHOSTVRJETASSOCIATIONALG

#include <AthenaBaseComps/AthAlgorithm.h>

#include "Math/GenVector/VectorUtil.h" // for computing deltaR
#include <AthContainers/AuxElement.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>

// Hbb tag
#include "FlavorTagDiscriminants/HbbTagConfig.h"
#include "FlavorTagDiscriminants/HbbTag.h"

// Class definition

// Forward declare the Xbb tagger 
namespace FlavorTagDiscriminants {
  class HbbTag;
}

namespace Easyjet
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
    using ELPC = ElementLink<xAOD::IParticleContainer>;
    using ELJC = ElementLink<xAOD::JetContainer>;

    StatusCode recordVRTrackJetGhostAssociation(const xAOD::JetContainer &,
                                                const xAOD::EventInfo &) const;

    // Xbb tagger 
    std::unique_ptr<FlavorTagDiscriminants::HbbTag> m_hbbTagXbb2020v3;

    Gaudi::Property<bool> m_isMC {
      this, "isMC", false, "Is this simulation?"
    };

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{
        this, "EventInfoKey", "EventInfo",
        "the eventInfo container to decorate"};

    SG::ReadHandleKey<xAOD::JetContainer> m_largeJetInKey{
        this, "LargeJetInKey", "", "the large-R jet collection to run on"};

    Gaudi::Property<std::string> m_eventInfoDecorSuffix{
        this, "EventInfoDecorSuffix", "",
        "suffix indicating the type of jet used for EventInfo decorations"};

    Gaudi::Property<std::vector<std::string> > m_workingPoints {
        this, "workingPoints", {}, "the working points to select VR track jets with"};
    
    // ghost associated VR track jets are only on the untrimmed 1.0 jets
    SG::AuxElement::ConstAccessor<ELJC> m_largeRUntrimmedAccessor{"Parent"};

    SG::AuxElement::ConstAccessor<std::vector<ELPC>>
        m_ghostVRTrackJetsAccessor{"GhostAntiKtVR30Rmax4Rmin02PV0TrackJets"};

    SG::AuxElement::ConstAccessor<int>
        m_TruthLabelAccessor{"HadronConeExclTruthLabelID"};

    std::vector<SG::AuxElement::ConstAccessor<char>> m_isBtagAccessors;

    // recommended by ftag : Remove the event if any of your signal jets have
    // relativeDeltaRToVRJet = radius(jet_i)/min(dR(jet_i,jet_j)) < 1.0.
    // checks if any of the vr jets overlap
    SG::AuxElement::ConstAccessor<float> relativeDeltaRToVRJet{
        "relativeDeltaRToVRJet"};

    // Name will be updated with suffix in initialise
    // Retained here for reference
    SG::AuxElement::Decorator<char> m_passRelativeDeltaRToVRJetCutDecorator{
        "passRelativeDeltaRToVRJetCut"};

    SG::AuxElement::Decorator<int> m_goodVRTrackJetCountDecorator{
        "goodVRTrackJets"};

    SG::AuxElement::Decorator<float> m_minRelativeDeltaRToVRJetDecorator{
        "minRelativeDeltaRToVRJet"};

    SG::AuxElement::Decorator<std::vector<int>> m_HadronConeExclTruthLabelIDDecorator{
        "VRTrackJetsTruthLabel"};

    SG::AuxElement::Decorator<std::vector<float>>
        m_leadingVRTrackJetPtDecorator{"leadingVRTrackJetsPt"};

    SG::AuxElement::Decorator<std::vector<float>>
        m_leadingVRTrackJetEtaDecorator{"leadingVRTrackJetsEta"};

    SG::AuxElement::Decorator<std::vector<float>>
        m_leadingVRTrackJetPhiDecorator{"leadingVRTrackJetsPhi"};

    SG::AuxElement::Decorator<std::vector<float>>
        m_leadingVRTrackJetMDecorator{"leadingVRTrackJetsM"};

    SG::AuxElement::Decorator<float> m_leadingVRTrackJetDeltaR12Decorator{
        "leadingVRTrackJetsDeltaR12"};

    SG::AuxElement::Decorator<float> m_leadingVRTrackJetDeltaR13Decorator{
        "leadingVRTrackJetsDeltaR13"};

    SG::AuxElement::Decorator<float> m_leadingVRTrackJetDeltaR32Decorator{
        "leadingVRTrackJetsDeltaR32"};

    std::vector<SG::AuxElement::Decorator<std::vector<char>>>
        m_leadingVRTrackJetBtagDecorators;
    
  };
}

#endif
