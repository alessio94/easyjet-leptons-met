/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HH4BANALYSIS_FINALVARSBBTTALG
#define HH4BANALYSIS_FINALVARSBBTTALG

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

#include <AthContainers/ConstDataVector.h>

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <FourMomUtils/xAODP4Helpers.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>

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

    /// \brief Setup sys-aware output decorations
    CP::SysWriteDecorHandle<float> m_leading_muon_pt {"Leading_Muon_pt_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_leading_muon_eta {"Leading_Muon_eta_%SYS%", this};

    CP::SysWriteDecorHandle<float> m_leading_elec_pt {"Leading_Electron_pt_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_leading_elec_eta {"Leading_Electron_eta_%SYS%", this};

    CP::SysWriteDecorHandle<float> m_leading_tau_pt {"Leading_Tau_pt_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_leading_tau_eta {"Leading_Tau_eta_%SYS%", this};

    // Steerbale vars
    std::string m_bTagWP;

    // Local variables and functions

  };
}

#endif
