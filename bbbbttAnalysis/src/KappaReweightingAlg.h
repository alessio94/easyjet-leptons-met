/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef BBBBTTANALYSIS_KAPPAREWEIGHTING
#define BBBBTTANALYSIS_KAPPAREWEIGHTING

#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>
#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <xAODEventInfo/EventInfo.h>

#include <AsgTools/AnaToolHandle.h>
#include <PMGTools/PMGTruthWeightTool.h>

namespace HHHBBBBTT
{

  /// \brief An algorithm for counting containers
  class KappaReweightingAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    KappaReweightingAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:

    // The reweighting is independent of the systematics.
    SG::ReadHandleKey<xAOD::EventInfo>
     m_EventHandleKey{ this, "EventHandleKey", "EventInfo", "EventInfo to read" };
    
    /// \brief Setup output decorations
    std::unordered_map<std::string, SG::WriteDecorHandleKey<xAOD::EventInfo>>
      m_Fbranches;

    /// Reweight tool
    ToolHandle<PMGTools::IPMGTruthWeightTool> m_wgtHandle{this,
      "PMGTruthWeightTool",
      "PMGTools::PMGTruthWeightTool/PMGTruthWeightTool",
      "PMGTool for k3-k4 reweighting"};

    /// variable names
    Gaudi::Property<std::vector<std::string>> m_reweightVars
        { this, "reweightVars", {}, "Kappa-variations weights that should be read"};
    
  };
}
#endif