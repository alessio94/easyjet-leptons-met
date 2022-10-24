/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HH4BANALYSIS_FINALVARSRESOLVEDALG
#define HH4BANALYSIS_FINALVARSRESOLVEDALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>

namespace HH4B
{

  /// \brief An algorithm for counting containers
  class FinalVarsResolvedAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    FinalVarsResolvedAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    SG::ReadHandleKey<ConstDataVector<xAOD::JetContainer>>
        m_smallRContainerInKey{this, "smallRContainerInKey", "",
                               "containerName to read"};
    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{
        this, "EventInfoKey", "EventInfo", "EventInfo container to dump"};

    std::string m_bTagWP;
    std::unordered_map<std::string, SG::AuxElement::Decorator<float>> m_decos;
    // clang-format off
    std::vector<std::string> m_vars{
      "resolved_DeltaR12_",
      "resolved_DeltaR13_",
      "resolved_DeltaR14_",
      "resolved_DeltaR23_",
      "resolved_DeltaR24_",
      "resolved_DeltaR34_",
      "resolved_h1_m_",
      "resolved_h2_m_",
      "resolved_hh_m_",
    };
    // clang-format on
  };
}

#endif
