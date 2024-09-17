/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HH4BANALYSIS_FINALVARSBOOSTEDALG
#define HH4BANALYSIS_FINALVARSBOOSTEDALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>

namespace HH4B
{

  /// \brief An algorithm for counting containers
  class BaselineVarsBoostedAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    BaselineVarsBoostedAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::JetContainer>
      m_LargeJetHandle{ this, "largeRContainerInKey", "", "Large-R jet container to read" };
    CP::SysReadHandle<xAOD::JetContainer>
      m_leadingLargeR_GA_VRJetsHandle{ this, "leadingLargeR_GA_VRJets", "", "Large-R jet container to read" };
    CP::SysReadHandle<xAOD::JetContainer>
      m_subleadingLargeR_GA_VRJetsHandle{ this, "subLeadingLargeR_GA_VRJets", "", "Large-R jet container to read" };
    CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    Gaudi::Property<std::string> m_bTagWP {this, "bTagWP", "", "B-tagging working point."};
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_decos;
    // clang-format off
    std::vector<std::string> m_vars{
      "boosted_h1_m_",
      "boosted_h1_jet1_pt_",
      "boosted_h1_jet2_pt_",
      "boosted_h1_dR_jets_",
      "boosted_h2_m_",
      "boosted_h2_jet1_pt_",
      "boosted_h2_jet2_pt_",
      "boosted_h2_dR_jets_",
      "boosted_hh_m_",
    };
    // clang-format on
  };
}

#endif
