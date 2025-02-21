/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HHBBLLANALYSIS_NEUTRINOWEIGHTINGALG
#define HHBBLLANALYSIS_NEUTRINOWEIGHTINGALG

#include "AsgTools/ToolHandleArray.h"

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMissingET/MissingETContainer.h>

#include "NeutrinoWeightingTool.h"


namespace HHBBLL
{

  /// \brief An algorithm for counting containers
  class NeutrinoWeightingAlg final : public EL::AnaAlgorithm
  {
    /// \brief The standard constructor
public:
    NeutrinoWeightingAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::JetContainer>
      m_jetHandle{ this, "jets", "bbllAnalysisJets_%SYS%", "Jet container to read" };

    CP::SysReadDecorHandle<char> 
      m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};
    
    CP::SysReadHandle<xAOD::ElectronContainer>
      m_electronHandle{ this, "electrons", "bbllAnalysisElectrons_%SYS%", "Electron container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
      m_muonHandle{ this, "muons", "bbllAnalysisMuons_%SYS%", "Muon container to read" };

    CP::SysReadHandle<xAOD::MissingETContainer>
    m_metHandle{ this, "met", "AnalysisMET_%SYS%", "MET container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    Gaudi::Property<std::vector<std::string>> m_cutList
    { this, "NW_cutList", {}, "List of cuts to apply" };

    std::unordered_map<std::string, CP::SysReadDecorHandle<bool>>
      m_cuts;

    ToolHandleArray<NeutrinoWeightingTool> m_reco_pairings { this, "NeutrinoWeightingTools", {}, "" };

    /// \brief Setup sys-aware output decorations
    CP::SysWriteDecorHandle<int> m_solutions {"NW_solutions_%SYS%", this};

    Gaudi::Property<std::vector<std::string>> m_floatVariables
    { this, "floatNWVariables", {}, "List of float variables to write out" };

    Gaudi::Property<bool> m_save_extra_vars
      { this, "save_extra_vars", false, "Compute quantities which may be useful." };

    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>>
      m_fBranches;

  };
}

#endif
