/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HH4BANALYSIS_FINALVARSBBTTALG
#define HH4BANALYSIS_FINALVARSBBTTALG

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

namespace HH4B
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

    CP::SysReadDecorHandle<bool> 
    m_selected_el { this, "selected_el", "selected_el_%SYS%", "Name of input dectorator for selected el"};
    CP::SysReadDecorHandle<bool> 
    m_selected_mu { this, "selected_mu", "selected_mu_%SYS%", "Name of input dectorator for selected mu"};
    CP::SysReadDecorHandle<bool> 
    m_selected_tau { this, "selected_tau", "selected_tau_%SYS%", "Name of input dectorator for selected tau"};

    /// \brief Setup sys-aware output decorations
    CP::SysWriteDecorHandle<float> m_HH_pt {"HH_pt_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_HH_eta {"HH_eta_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_HH_phi {"HH_phi_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_HH_m {"HH_m_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_HH_vis_pt {"HH_vis_pt_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_HH_vis_eta {"HH_vis_eta_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_HH_vis_phi {"HH_vis_phi_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_HH_vis_m {"HH_vis_m_%SYS%", this};

    CP::SysWriteDecorHandle<float> m_selected_lepton_pt {"Selected_Lepton_pt_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_selected_lepton_eta {"Selected_Lepton_eta_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_selected_lepton_phi {"Selected_Lepton_phi_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_selected_lepton_charge {"Selected_Lepton_charge_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_selected_lepton_pdgid {"Selected_Lepton_pdgid_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_selected_tau_pt {"Selected_Tau_pt_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_selected_tau_eta {"Selected_Tau_eta_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_selected_tau_phi {"Selected_Tau_phi_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_selected_tau_charge {"Selected_Tau_charge_%SYS%", this};

    // Local variables and functions

  };
}

#endif
