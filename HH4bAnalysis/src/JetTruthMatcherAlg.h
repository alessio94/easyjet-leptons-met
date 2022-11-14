/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HH4BANALYSIS_JETTRUTHMATCHERALG
#define HH4BANALYSIS_JETTRUTHMATCHERALG

#include "xAODTruth/TruthParticleContainer.h"
#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>

namespace HH4B
{

  /// \brief An algorithm for counting containers
  class JetTruthMatcherAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    JetTruthMatcherAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};
    SG::ReadHandleKey<xAOD::JetContainer> m_smallRContainerInKey{
        this, "smallRContainerInKey", "", "containerName to read"};
    SG::ReadHandleKey<xAOD::JetContainer> m_leadingLargeR_GA_VRJets{
        this, "leadingLargeR_GA_VRJets", "", "containerName to read"};
    SG::ReadHandleKey<xAOD::JetContainer> m_subLeadingLargeR_GA_VRJets{
        this, "subLeadingLargeR_GA_VRJets", "", "containerName to read"};
    SG::WriteHandleKey<ConstDataVector<xAOD::JetContainer>> m_containerOutKey{
        this, "containerOutKey", "", "containerName to write"};
    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{
        this, "EventInfoKey", "EventInfo", "EventInfo container to dump"};

    std::string m_bTagWP;
    std::string m_regime;
    bool m_resolved = false;
    bool m_boosted = false;
    // decorators
    std::unordered_map<std::string, SG::AuxElement::Decorator<float>> m_decos;
    std::unordered_map<std::string,
                       SG::AuxElement::Decorator<std::vector<float>>>
        m_fourVecDecos;
    // clang-format off
    std::vector<std::string> m_vars{
      "_h1_closestTruthBsHaveSameInitialParticle_",
      "_h2_closestTruthBsHaveSameInitialParticle_",
      "_h1_dR_leadingJet_closestTruthB_",
      "_h1_dR_subleadingJet_closestTruthB_",
      "_h2_dR_leadingJet_closestTruthB_",
      "_h2_dR_subleadingJet_closestTruthB_",
      "_h1_parentPdgId_leadingJet_closestTruthB_",
      "_h1_parentPdgId_subleadingJet_closestTruthB_",
      "_h2_parentPdgId_leadingJet_closestTruthB_",
      "_h2_parentPdgId_subleadingJet_closestTruthB_",
    };
    // clang-format on
  };
}

#endif
