/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef BBTTANALYSIS_MMCDECORATORALG
#define BBTTANALYSIS_MMCDECORATORALG

#include "AnaAlgorithm/AnaAlgorithm.h"

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysListHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODTau/TauJetContainer.h>
#include <xAODMissingET/MissingETContainer.h>

#include <DiTauMassTools/MissingMassToolV2.h>

#include "HHbbttEnums.h"

namespace HHBBTT
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
      m_jetHandle{ this, "jets", "bbttAnalysisJets_%SYS%", "Jet container to read" };
    
    CP::SysReadHandle<xAOD::ElectronContainer>
      m_electronHandle{ this, "electrons", "bbttAnalysisElectrons_%SYS%", "Electron container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
      m_muonHandle{ this, "muons", "bbttAnalysisMuons_%SYS%", "Muon container to read" };

    CP::SysReadHandle<xAOD::TauJetContainer>
    m_tauHandle{ this, "taus", "bbttAnalysisTaus_%SYS%", "Tau container to read" };

    CP::SysReadHandle<xAOD::MissingETContainer>
    m_metHandle{ this, "met", "AnalysisMET_%SYS%", "MET container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    CP::SysReadDecorHandle<bool> m_pass_LepHad {this, "passLepHad", "pass_baseline_LepHad_%SYS%", "events pass lep-had selections"};
    CP::SysReadDecorHandle<bool> m_pass_HadHad {this, "passHadHad", "pass_baseline_HadHad_%SYS%", "events pass had-had selections"};

    CP::SysReadDecorHandle<bool> m_selected_el {"selected_el_%SYS%", this};
    CP::SysReadDecorHandle<bool> m_selected_mu {"selected_mu_%SYS%", this};
    CP::SysReadDecorHandle<bool> m_selected_tau {"selected_tau_%SYS%", this};

    /// \brief Setup sys-aware output decorations
    CP::SysWriteDecorHandle<int> m_mmc_status {"mmc_status_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_mmc_pt {"mmc_pt_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_mmc_eta {"mmc_eta_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_mmc_phi {"mmc_phi_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_mmc_m {"mmc_m_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_mmc_nu1_pt {"mmc_nu1_pt_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_mmc_nu1_eta {"mmc_nu1_eta_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_mmc_nu1_phi {"mmc_nu1_phi_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_mmc_nu1_m {"mmc_nu1_m_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_mmc_nu2_pt {"mmc_nu2_pt_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_mmc_nu2_eta {"mmc_nu2_eta_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_mmc_nu2_phi {"mmc_nu2_phi_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_mmc_nu2_m {"mmc_nu2_m_%SYS%", this};

    /// \brief Steerable properties
    Gaudi::Property<std::string> m_method_str { this, "Method", "MLNU3P", 
						 "MMC method to use"};
    DiTauMassTools::MMCFitMethodV2::e m_method;
    ToolHandle<DiTauMassTools::MissingMassToolV2> m_mmcTool
      { this, "mmcTool", "DiTauMassTools::MissingMassToolV2", "the Missing Mass Calculator tool"};

    Gaudi::Property<std::vector<std::string>> m_channel_names
      { this, "channel", {}, "Which channel to run" };

    std::vector<HHBBTT::Channel> m_channels;

  };
}

#endif
