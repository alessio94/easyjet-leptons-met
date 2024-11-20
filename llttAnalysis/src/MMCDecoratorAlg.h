/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef LLTTANALYSIS_MMCDECORATORALG
#define LLTTANALYSIS_MMCDECORATORALG

#include <memory>

#include "AnaAlgorithm/AnaAlgorithm.h"

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODTau/TauJetContainer.h>
#include <xAODMissingET/MissingETContainer.h>

#include <DiTauMassTools/MissingMassTool.h>

#include "HllttChannels.h"

namespace HLLTT
{

  /// \brief An algorithm for counting containers
  class MMCDecoratorAlg final : public EL::AnaAlgorithm
  {
    /// \brief The standard constructor
public:
    MMCDecoratorAlg(const std::string &name, ISvcLocator *pSvcLocator);

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
      m_jetHandle{ this, "jets", "llttAnalysisJets_%SYS%", "Jet container to read" };
    
    CP::SysReadHandle<xAOD::ElectronContainer>
      m_electronHandle{ this, "electrons", "llttAnalysisElectrons_%SYS%", "Electron container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
      m_muonHandle{ this, "muons", "llttAnalysisMuons_%SYS%", "Muon container to read" };

    CP::SysReadHandle<xAOD::TauJetContainer>
    m_tauHandle{ this, "taus", "llttAnalysisTaus_%SYS%", "Tau container to read" };

    CP::SysReadHandle<xAOD::MissingETContainer>
    m_metHandle{ this, "met", "AnalysisMET_%SYS%", "MET container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    CP::SysReadDecorHandle<bool> m_pass_baseline_LepLep {this, "passLepLep", "", "events pass (baseline) LepLep"};
    CP::SysReadDecorHandle<bool> m_pass_baseline_LepHad {this, "passLepHad", "", "events pass (baseline) LepHad"};
    CP::SysReadDecorHandle<bool> m_pass_baseline_HadHad {this, "passHadHad", "", "events pass (baseline) HadHad"};

    CP::SysReadDecorHandle<bool> m_selected_el {"selected_el_%SYS%", this};
    CP::SysReadDecorHandle<bool> m_selected_mu {"selected_mu_%SYS%", this};
    CP::SysReadDecorHandle<bool> m_selected_tau {"selected_tau_%SYS%", this};

    /// \brief Setup sys-aware output decorations
    CP::SysWriteDecorHandle<int> m_mmc_status {"mmc_status_%SYS%", this};
    CP::SysWriteDecorHandle<int> m_mmc_types {"mmc_types_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_mmc_pt {"mmc_pt_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_mmc_eta {"mmc_eta_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_mmc_phi {"mmc_phi_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_mmc_m {"mmc_m_%SYS%", this};

    /// \brief Steerable properties
    Gaudi::Property<std::string> m_method_str { this, "Method", "MLNU3P", 
						 "MMC method to use"};
    DiTauMassTools::MMCFitMethod::e m_method;
    ToolHandle<DiTauMassTools::MissingMassTool> m_mmcTool
      { this, "mmcTool", "DiTauMassTools::MissingMassTool", "the Missing Mass Calculator tool"};

    Gaudi::Property<std::vector<std::string>> m_channel_names
      { this, "channel", {}, "Which channel to run" };

    std::vector<HLLTT::Channel> m_channels;

  };
}

#endif
