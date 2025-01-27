/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HHBBYYANALYSIS_LEPTONVARSYYBBALG
#define HHBBYYANALYSIS_LEPTONVARSYYBBALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>

namespace HHBBYY
{
  /// \brief An algorithm for counting containers
  class LeptonVarsbbyyAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
  public:
    LeptonVarsbbyyAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;

  private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::ElectronContainer>
    m_electronHandle{ this, "electrons", "bbyyAnalysisElectrons_%SYS%", "Electron container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
    m_muonHandle{ this, "muons", "bbyyAnalysisMuons_%SYS%", "Muon container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    CP::SysReadDecorHandle<unsigned int> m_year
      {this, "year", "dataTakingYear", ""};
 
    Gaudi::Property<bool> m_doSystematics
      { this, "doSystematics", false, "Run on all systematics" };

    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches_lep;
  };
}
#endif