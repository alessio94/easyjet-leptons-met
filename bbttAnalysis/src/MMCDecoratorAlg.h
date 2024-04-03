/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef BBTTANALYSIS_MMCDECORATORALG
#define BBTTANALYSIS_MMCDECORATORALG

#include <memory>

#include "AnaAlgorithm/AnaAlgorithm.h"
#include <FourMomUtils/xAODP4Helpers.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/ISysHandleBase.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODTau/TauJetContainer.h>
#include <xAODMissingET/MissingETContainer.h>

#include <DiTauMassTools/MissingMassToolV2.h>

#include "HHbbttChannels.h"

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

    CP::SysReadDecorHandle<bool> m_pass_SLT {this, "passSLT", "", "events pass (baseline) SLT"};
    CP::SysReadDecorHandle<bool> m_pass_LTT {this, "passLTT", "", "events pass (baseline) LTT"};
    CP::SysReadDecorHandle<bool> m_pass_SLT_1B {this, "passSLT_1B", "", "events pass baseline SLT"};
    CP::SysReadDecorHandle<bool> m_pass_LTT_1B {this, "passLTT_1B", "", "events pass baseline LTT"};
    CP::SysReadDecorHandle<bool> m_pass_STT {this, "passSTT", "", "events pass (baseline) STT"};
    CP::SysReadDecorHandle<bool> m_pass_DTT {this, "passDTT", "", "events pass (baseline) DTT"};
    CP::SysReadDecorHandle<bool> m_pass_STT_1B {this, "passSTT_1B", "", "events pass baseline STT"};
    CP::SysReadDecorHandle<bool> m_pass_DTT_1B {this, "passDTT_1B", "", "events pass baseline DTT"};

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
    // Keep 2016MC15C as default, as 2019 has higher rate of non-converging fir for bbtt
    Gaudi::Property<std::string> m_calib_set { this, "CalibSet", "2016MC15C", 
						 "MMC calibration to use"};
    Gaudi::Property<bool> m_float_stop { this, "FloatStoppingCrit", false, 
					   "Activate MMC floating stopping criterion"};

    Gaudi::Property<int> m_verbose { this, "UseVerbose", 0, 
					"Activate MMC verbose output"};

    Gaudi::Property<std::vector<std::string>> m_channel_names
      { this, "channel", {}, "Which channel to run" };

    std::vector<HHBBTT::Channel> m_channels;

    /// \brief Internal variables
    std::unique_ptr<DiTauMassTools::MissingMassToolV2> m_mmcTool;
    DiTauMassTools::MMCFitMethodV2::e m_method;
  };
}

#endif
