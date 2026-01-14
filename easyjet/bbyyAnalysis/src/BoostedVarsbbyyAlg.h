/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HHBBYYANALYSIS_BOOSTEDVARSYYBBALG
#define HHBBYYANALYSIS_BOOSTEDVARSYYBBALG

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>

namespace HHBBYY
{

  /// \brief An algorithm for counting containers
  class BoostedVarsbbyyAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
  public:
    BoostedVarsbbyyAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

  private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::JetContainer>
    m_jetHandle{ this, "jets", "bbyyAnalysisLargeRJets_%SYS%", "Jet container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    CP::SysReadDecorHandle<int> m_R10TruthLabel{"R10TruthLabel_R22v1", this};

    CP::SysReadDecorHandle<float> m_GN2Xv01_phbb{"GN2Xv01_phbb", this};
    CP::SysReadDecorHandle<float> m_GN2Xv01_phcc{"GN2Xv01_phcc", this};
    CP::SysReadDecorHandle<float> m_GN2Xv01_pqcd{"GN2Xv01_pqcd", this};
    CP::SysReadDecorHandle<float> m_GN2Xv01_ptop{"GN2Xv01_ptop", this};

    Gaudi::Property<std::vector<std::string>> m_Fvarnames
      {this, "floatVariableList", {}, "Name list of float variables"};

    Gaudi::Property<std::vector<std::string>> m_Ivarnames
      {this, "intVariableList", {}, "Name list of integer variables"};

    Gaudi::Property<bool> m_doSystematics
      { this, "doSystematics", false, "Run on all systematics" };
      
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float> > m_Fbranches;

    std::unordered_map<std::string, CP::SysWriteDecorHandle<int> > m_Ibranches;

  };
}
#endif
