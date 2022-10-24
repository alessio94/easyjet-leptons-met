/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HH4BANALYSIS_JETTRUTHMATCHERALG
#define HH4BANALYSIS_JETTRUTHMATCHERALG

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

    SG::ReadHandleKey<xAOD::JetContainer> m_containerInKey{
        this, "containerInKey", "", "containerName to read"};
    SG::WriteHandleKey<ConstDataVector<xAOD::JetContainer>> m_containerOutKey{
        this, "containerOutKey", "", "containerName to write"};
    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{
        this, "EventInfoKey", "EventInfo", "EventInfo container to dump"};

    std::string m_bTagWP;
    std::unordered_map<std::string, SG::AuxElement::Decorator<float>> m_decos;

    // clang-format off
    std::vector<std::string> m_vars{
      "resolved_h1_closestTruthBsHaveSameInitialParticle_",
      "resolved_h2_closestTruthBsHaveSameInitialParticle_",
      "resolved_h1_dR_leadingJet_closestTruthB_",
      "resolved_h1_dR_subleadingJet_closestTruthB_",
      "resolved_h2_dR_leadingJet_closestTruthB_",
      "resolved_h2_dR_subleadingJet_closestTruthB_",
    };
    // clang-format on
  };
}

#endif
