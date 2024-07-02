/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HHBBLLANALYSIS_MMCDECORATORALG
#define HHBBLLANALYSIS_MMCDECORATORALG

#include "AnaAlgorithm/AnaAlgorithm.h"

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMissingET/MissingETContainer.h>

#include <DiTauMassTools/MissingMassToolV2.h>

namespace HHBBLL
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
      m_jetHandle{ this, "jets", "bbllAnalysisJets_%SYS%", "Jet container to read" };
    
    CP::SysReadHandle<xAOD::ElectronContainer>
      m_electronHandle{ this, "electrons", "bbllAnalysisElectrons_%SYS%", "Electron container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
      m_muonHandle{ this, "muons", "bbllAnalysisMuons_%SYS%", "Muon container to read" };

    CP::SysReadHandle<xAOD::MissingETContainer>
    m_metHandle{ this, "met", "AnalysisMET_%SYS%", "MET container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    /// \brief Setup sys-aware output decorations
    CP::SysWriteDecorHandle<int> m_mmc_status {"mmc_status_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_mmc_pt {"mmc_pt_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_mmc_eta {"mmc_eta_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_mmc_phi {"mmc_phi_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_mmc_m {"mmc_m_%SYS%", this};

    /// \brief Steerable properties
    Gaudi::Property<std::string> m_method_str { this, "Method", "MLNU3P", 
						 "MMC method to use"};
    Gaudi::Property<std::string> m_calib_set { this, "CalibSet", "2019", 
						 "MMC calibration to use"};
    Gaudi::Property<bool> m_float_stop { this, "FloatStoppingCrit", false, 
					   "Activate MMC floating stopping criterion"};

    Gaudi::Property<int> m_verbose { this, "UseVerbose", 0, 
					"Activate MMC verbose output"};

    /// \brief Internal variables
    std::unique_ptr<DiTauMassTools::MissingMassToolV2> m_mmcTool;
    DiTauMassTools::MMCFitMethodV2::e m_method;
  };
}

#endif
