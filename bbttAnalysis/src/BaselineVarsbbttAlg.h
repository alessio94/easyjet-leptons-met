/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef BBTTANALYSIS_FINALVARSBBTTALG
#define BBTTANALYSIS_FINALVARSBBTTALG

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <AthContainers/ConstDataVector.h>

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <FourMomUtils/xAODP4Helpers.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODTau/TauJetContainer.h>
#include <xAODMissingET/MissingETContainer.h>

namespace HHBBTT
{

  /// \brief An algorithm for counting containers
  class BaselineVarsbbttAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    BaselineVarsbbttAlg(const std::string &name, ISvcLocator *pSvcLocator);

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
      m_jetHandle{ this, "jets", "",   "Jet container to read" };
    
    CP::SysReadHandle<xAOD::ElectronContainer>
    m_electronHandle{ this, "electrons", "",   "Electron container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
    m_muonHandle{ this, "muons", "",   "Muon container to read" };

    CP::SysReadHandle<xAOD::TauJetContainer>
    m_tauHandle{ this, "taus", "",   "Tau container to read" };

    CP::SysReadHandle<xAOD::MissingETContainer>
    m_metHandle{ this, "met", "AnalysisMET",   "MET container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

    CP::SysReadDecorHandle<float> 
    m_mmc_pt { this, "mmc_pt", "mmc_pt_%SYS%", "MMC pt key"};
    CP::SysReadDecorHandle<float> 
    m_mmc_eta { this, "mmc_eta", "mmc_eta_%SYS%", "MMC eta key"};
    CP::SysReadDecorHandle<float> 
    m_mmc_phi { this, "mmc_phi", "mmc_phi_%SYS%", "MMC phi key"};
    CP::SysReadDecorHandle<float> 
    m_mmc_m { this, "mmc_m", "mmc_m_%SYS%", "MMC mass key"};

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    Gaudi::Property<std::string> m_eleWPName
      { this, "eleWP", "","Electron ID + Iso working point" };
    CP::SysReadDecorHandle<float> m_ele_SF{"", this};

    Gaudi::Property<std::string> m_muWPName
      { this, "muonWP", "","Muon ID + Iso working point" };
    CP::SysReadDecorHandle<float> m_mu_SF{"", this};

    Gaudi::Property<std::string> m_tauWPName
      { this, "tauWP", "","Tau ID working point" };
    CP::SysReadDecorHandle<float> m_tau_effSF{"", this};

    CP::SysReadDecorHandle<bool> 
    m_selected_el { this, "selected_el", "selected_el_%SYS%", "Name of input decorator for selected el"};
    CP::SysReadDecorHandle<bool> 
    m_selected_mu { this, "selected_mu", "selected_mu_%SYS%", "Name of input decorator for selected mu"};
    CP::SysReadDecorHandle<bool> 
    m_selected_tau { this, "selected_tau", "selected_tau_%SYS%", "Name of input decorator for selected tau"};
    
    CP::SysReadDecorHandle<char> 
    m_isBtag {this, "bTagWPDecorName", "", "Name of input decorator for b-tagging"};

    Gaudi::Property<bool> m_storeHighLevelVariables
      { this, "storeHighLevelVariables", false, "Flag to store high level variables in output" };
    SG::ReadDecorHandleKey<xAOD::MuonContainer> m_muonPreselDecorKey;

    Gaudi::Property<std::vector<std::string>> m_floatVariables
          {this, "floatVariableList", {}, "Name list of floating variables"};

    Gaudi::Property<std::vector<std::string>> m_intVariables
          {this, "intVariableList", {}, "Name list of integer variables"};

    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>>
        m_Fbranches;

    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;

  };
} // namespace HHBBTT

#endif
