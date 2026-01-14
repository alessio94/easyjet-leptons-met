/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HHBBTTANALYSIS_FINALVARSBOOSTEDALG
#define HHBBTTANALYSIS_FINALVARSBOOSTEDALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>

namespace HHBBTT
{

  /// \brief An algorithm for counting containers
  class BaselineVarsBoostedbbttAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    BaselineVarsBoostedbbttAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    /// ToolHandle<whatever> handle {this, "pythonName", "defaultValue", "someInfo"};

    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::JetContainer>
      m_lRjetHandle{ this, "largeRjets", "bbttAnalysisLargeRJets_%SYS%", "Large-R jet container to read" };
      
    CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    Gaudi::Property<std::vector<std::string>> m_floatVariables
      {this, "floatVariableList", {}, "Name list of floating variables"};
    
    CP::SysReadDecorHandle<int> 
    m_Pass_GN2X{this, "GN2X_WP", "", "GN2X b-tagging working point"}; 

    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_Fbranches;

  };
}

#endif
