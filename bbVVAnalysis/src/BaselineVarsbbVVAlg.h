/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef BBVVANALYSIS_FINALVARSBBVVALG
#define BBVVANALYSIS_FINALVARSBBVVALG

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMissingET/MissingETContainer.h>

#include "HHbbVVEnums.h"

namespace HHBBVV
{

  /// \brief An algorithm for counting containers
  class BaselineVarsbbVVAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    BaselineVarsbbVVAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any


private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};
    Gaudi::Property<std::vector<std::string>> m_channel_names
    { this, "channel", {}, "Which channel to run" };
    
    std::vector<HHBBVV::Channel> m_channels; // Which bbVV channels to run

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::JetContainer>
      m_jetHandle{ this, "jets", "bbVVAnalysisJets_%SYS%", "Jet container to read" };

    CP::SysReadHandle<xAOD::JetContainer>
      m_lrjetHandle{ this, "lrjets", "bbVVAnalysisLRJets_%SYS%", "Large-R jet container to read" };
    
    CP::SysReadHandle<xAOD::ElectronContainer>
    m_electronHandle{ this, "electrons", "bbVVAnalysisElectrons_%SYS%", "Electron container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
    m_muonHandle{ this, "muons", "bbVVAnalysisMuons_%SYS%", "Muon container to read" };

    CP::SysReadHandle<xAOD::MissingETContainer>
    m_metHandle{ this, "met", "AnalysisMET_%SYS%", "MET container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    Gaudi::Property<std::string> m_eleWPName
      { this, "eleWP", "","Electron ID + Iso working point" };
    CP::SysReadDecorHandle<float> m_ele_SF{"", this};

    Gaudi::Property<std::string> m_muWPName
      { this, "muonWP", "","Muon ID + Iso working point" };
    CP::SysReadDecorHandle<float> m_mu_SF{"", this};

    CP::SysReadDecorHandle<bool> 
    m_selected_el { this, "selected_el", "selected_el_%SYS%", "Name of input dectorator for selected el"};
    CP::SysReadDecorHandle<bool> 
    m_selected_mu { this, "selected_mu", "selected_mu_%SYS%", "Name of input dectorator for selected mu"};

    CP::SysReadDecorHandle<bool>
    m_Whad { this, "Whad", "Whad_%SYS%", "Name of input dectorator for Whad jet"};
    CP::SysReadDecorHandle<bool>
    m_Whad2 { this, "Whad2", "Whad2_%SYS%", "Name of input dectorator for Whad jet"};
    CP::SysReadDecorHandle<bool>
    m_Hbb { this, "Hbb", "Hbb_%SYS%", "Name of input dectorator for Hbb jet"};

    CP::SysReadDecorHandle<float> 
    m_ANN_70_Score {this, "ANN_70_Score", "ANN70Tagger_Score", "ANN70Tagger score"};

    CP::SysReadDecorHandle<bool> 
    m_Pass_GN2X_FlatMassQCDEff_0p58 {this, "GN2X_PassFlatMassQCDEff_0p58", "GN2X_select_FlatMassQCDEff_0p58", "GN2X_select_FlatMassQCDEff_0p58 selection"};
    CP::SysReadDecorHandle<bool> 
    m_Pass_ANN_70 {this, "ANN_Pass70", "ANN70Tagger_Tagged", "ANN70Tagger selection"};

    Gaudi::Property<std::vector<std::string>> m_floatVariables
          {this, "floatVariableList", {}, "Name list of floating variables"};

    Gaudi::Property<std::vector<std::string>> m_intVariables
          {this, "intVariableList", {}, "Name list of integer variables"};

    
    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_Fbranches;

    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;

    // Local variables and functions

    bool m_run_lep = false;
    
  };
}

#endif
