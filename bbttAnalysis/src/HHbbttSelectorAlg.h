/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef BBTTANALYSIS_HHBBTTSELECTORALG
#define BBTTANALYSIS_HHBBTTSELECTORALG

#include <memory>

#include "AnaAlgorithm/AnaAlgorithm.h"
#include <FourMomUtils/xAODP4Helpers.h>
#include <AsgDataHandles/ReadDecorHandleKey.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/ISysHandleBase.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODTau/TauJetContainer.h>
#include <xAODMissingET/MissingETContainer.h>

#include <SystematicsHandles/SysFilterReporterParams.h>

#include "HHbbttChannels.h"

namespace HHBBTT
{

  /// \brief An algorithm for counting containers
  class HHbbttSelectorAlg final : public EL::AnaAlgorithm
  {
    /// \brief The standard constructor
public:
    HHbbttSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// \brief Finalisation method, for cleanup, final print out etc
    StatusCode finalize() override;

private:

    /// \brief Steerable properties
    Gaudi::Property<std::vector<std::string>> m_channel_names
      { this, "channel", {}, "Which channel to run" };

    std::vector<HHBBTT::Channel> m_channels;

    Gaudi::Property<bool> m_bypass
      { this, "bypass", false, "Run selector algorithm in pass-through mode" };

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

    Gaudi::Property<std::string> m_IDTauDecorName
      { this, "idTauDecorKey", "isIDTau", "Decoration for ID taus" };
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_IDTauDecorKey;

    CP::SysReadDecorHandle<char> 
    m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};

    Gaudi::Property<std::string> m_eleIdDecorName
      { this, "eleIdDecorKey", "DFCommonElectronsLHTight","Decoration for electron ID working point" };
    SG::ReadDecorHandleKey<xAOD::ElectronContainer> m_eleIdDecorKey;
		      
    Gaudi::Property<std::string> m_muonIdDecorName
      { this, "muonIdDecorKey", "DFCommonMuonPassIDCuts","Decoration for muon ID cuts" };
    SG::ReadDecorHandleKey<xAOD::MuonContainer> m_muonIdDecorKey;

    Gaudi::Property<std::string> m_muonPreselDecorName
      { this, "muonPreselDecorKey", "DFCommonMuonPassPreselection","Decoration for muon preselection" };
    SG::ReadDecorHandleKey<xAOD::MuonContainer> m_muonPreselDecorKey;
   
    CP::SysWriteDecorHandle<bool> m_pass_SLT {"pass_SLT_%SYS%", this};
    CP::SysWriteDecorHandle<bool> m_pass_LTT {"pass_LTT_%SYS%", this};
    CP::SysWriteDecorHandle<bool> m_pass_STT {"pass_STT_%SYS%", this};
    CP::SysWriteDecorHandle<bool> m_pass_DTT {"pass_DTT_%SYS%", this};

    CP::SysWriteDecorHandle<bool> m_selected_el {"selected_el_%SYS%", this};
    CP::SysWriteDecorHandle<bool> m_selected_mu {"selected_mu_%SYS%", this};
    CP::SysWriteDecorHandle<bool> m_selected_tau {"selected_tau_%SYS%", this};

    /// \brief Setup sys-aware output decorations
    CP::SysFilterReporterParams m_filterParams {this, "HHbbtautau selection"};
    
    /// \brief Internal variables

    bool TWO_JETS;
    bool TWO_BJETS;
    bool LEADJET_PT;
    bool MBB_MASS;
    bool N_LEPTONS_CUT_LEPHAD;
    bool ONE_TAU;
    bool OS_CHARGE_LEPHAD;
    bool pass_SLT;
    bool pass_LTT;
    bool N_LEPTONS_CUT_HADHAD;
    bool TWO_TAU;
    bool OS_CHARGE_HADHAD;
    bool pass_STT;
    bool pass_DTT;

  };
}

#endif
