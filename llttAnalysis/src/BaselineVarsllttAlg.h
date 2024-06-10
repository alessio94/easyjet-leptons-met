/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef LLTTANALYSIS_FINALVARSLLTTALG
#define LLTTANALYSIS_FINALVARSLLTTALG

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODTau/TauJetContainer.h>
#include <xAODMissingET/MissingETContainer.h>

namespace HLLTT
{

  /// \brief An algorithm for counting containers
  class BaselineVarsllttAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    BaselineVarsllttAlg(const std::string &name, ISvcLocator *pSvcLocator);

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

    CP::SysReadHandle<xAOD::TauJetContainer>
    m_mrmtauHandle{ this, "mrmtaus", "TauJets_MuonRM",   "Tau container to read" };

    CP::SysReadHandle<xAOD::MissingETContainer>
    m_metHandle{ this, "met", "AnalysisMET",   "MET container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

    CP::SysReadDecorHandle<int>
      m_mmc_status { this, "mmc_status", "mmc_status_%SYS%", "MMC status key"};
    CP::SysReadDecorHandle<int>
      m_mmc_types { this, "mmc_types", "mmc_types_%SYS%", "MMC types key"};
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
    m_selected_el { this, "selected_el", "selected_el_%SYS%", "Name of input dectorator for selected el"};
    CP::SysReadDecorHandle<bool> 
    m_selected_mu { this, "selected_mu", "selected_mu_%SYS%", "Name of input dectorator for selected mu"};
    CP::SysReadDecorHandle<bool> 
    m_selected_tau { this, "selected_tau", "selected_tau_%SYS%", "Name of input dectorator for selected tau"};

    CP::SysReadDecorHandle<bool>
    m_selected_mrmtau { this, "selected_mrmtau", "selected_mrmtau_%SYS%", "Name of input dectorator for selected tau"};

    CP::SysReadDecorHandle<char> 
    m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};

    Gaudi::Property<std::vector<std::string>> m_floatVariables
          {this, "floatVariableList", {}, "Name list of floating variables"};

    Gaudi::Property<std::vector<std::string>> m_intVariables
          {this, "intVariableList", {}, "Name list of integer variables"};

    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>>
        m_Fbranches;

    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;

  };
} // namespace HLLTT

#endif
