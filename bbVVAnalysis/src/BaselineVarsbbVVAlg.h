/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Kira Abeling, JaeJin Hong

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

    Gaudi::Property<std::vector<std::string>> m_GN2X_wps
      { this, "GN2X_WPs", {}, "GN2X_hbb_wps from the bbVV config" };
    std::vector<CP::SysReadDecorHandle<bool>> m_GN2X_wp_Handles;

    Gaudi::Property<std::string> m_WTag_Type
      { this, "wtag_type", "", "WTagger type from the bbVV config"};
    Gaudi::Property<std::string> m_WTag_WP
      { this, "wtag_wp", "", "WTagger wp from the bbVV config"};
    CP::SysReadDecorHandle<float> m_WTag_score{"", this};
    CP::SysReadDecorHandle<bool> m_Pass_WTag{"", this};

    std::vector<CP::SysReadDecorHandle<float>> m_tau_wta;
    std::vector<CP::SysReadDecorHandle<float>> m_ecf;

    CP::SysReadDecorHandle<float> m_GN2Xv01_phbb{"GN2Xv01_phbb", this};
    CP::SysReadDecorHandle<float> m_GN2Xv01_phcc{"GN2Xv01_phcc", this};
    CP::SysReadDecorHandle<float> m_GN2Xv01_pqcd{"GN2Xv01_pqcd", this};
    CP::SysReadDecorHandle<float> m_GN2Xv01_ptop{"GN2Xv01_ptop", this};

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
