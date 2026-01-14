/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HH4BANALYSIS_FINALVARSVBFRNNALG
#define HH4BANALYSIS_FINALVARSVBFRNNALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>

namespace HH4B
{

  /// \brief An algorithm for counting containers
  class VBFRNNVarsAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    VBFRNNVarsAlg(const std::string &name, ISvcLocator *pSvcLocator);

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
      m_RNNjetBoostedHandle{ this, "RNNJets_boosted", "RNNJets_boosted_%SYS%", "input RNN jets container for boosted 20 GeV" };
      
    CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    Gaudi::Property<std::vector<std::string>> m_Fvars
      {this, "floatVariableList", {}, "Name list of floating variables"};

    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_Fdecos;

    Gaudi::Property<bool> m_UseVBFRNN { this, "UseVBFRNN", false, "Add VBF-RNN jets in ntuples or not" };
  };
}

#endif
