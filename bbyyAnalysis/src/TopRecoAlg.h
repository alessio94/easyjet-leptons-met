/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef YYANALYSIS_TOPRECOALG
#define YYANALYSIS_TOPRECOALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMissingET/MissingETContainer.h>
#include <xAODJet/JetContainer.h>

#include "MVAUtils/BDT.h"

namespace HHBBYY
{
  /// \brief An algorithm for counting containers
  class TopRecoAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
  public:
    TopRecoAlg(const std::string &name, ISvcLocator *pSvcLocator);

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

    CP::SysReadHandle<xAOD::JetContainer>
    m_jetHandle{ this, "jets", "bbyyAnalysisJets_%SYS%", "Jet container to read" };

    CP::SysReadHandle<xAOD::ElectronContainer>
    m_electronHandle{ this, "electrons", "bbyyAnalysisElectrons_%SYS%", "Electron container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
    m_muonHandle{ this, "muons", "bbyyAnalysisMuons_%SYS%", "Muon container to read" };

    CP::SysReadHandle<xAOD::MissingETContainer>
    m_metHandle{ this, "met", "AnalysisMET_%SYS%", "MET container to read" };
    

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    // B-tagging decorator
    CP::SysReadDecorHandle<char> m_isBtag {this, "bTagWPDecorName", "", "Name of input decorator for b-tagging"};

    // PCBT decorator
    CP::SysReadDecorHandle<int> m_PCBT {this, "PCBTDecorName", "", "Name of input decorator for PCBT"};

    Gaudi::Property<std::string> m_topBDT_path { this, "BDT_path", {}, "Path to BDT model"};

    // Declare the Top Reco BDT
    std::unique_ptr<MVAUtils::BDT> m_topBDT {nullptr} ;

    Gaudi::Property<bool> m_doSystematics
      { this, "doSystematics", false, "Run on all systematics" };
    
    Gaudi::Property<std::vector<std::string>> m_intVariables
      {this, "intVariableList", {}, "Name list of integer variables"};

    Gaudi::Property<std::vector<std::string>> m_floatVariables
    {this, "floatVariableList", {}, "Name list of integer variables"};

    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_Fbranches; 

  };
}
#endif