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

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    Gaudi::Property<std::vector<int>> m_years
      { this, "Years", false, "which years are running" };
    
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

    Gaudi::Property<std::string> m_tauWPName
      { this, "tauWP", "", "Tau ID working point" };
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_tauWPDecorKey;

    CP::SysReadDecorHandle<char> 
    m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};

    CP::SysReadDecorHandle<unsigned int>
    m_runNumber {this, "runNumber", "runNumber", "Runnumber"};

    std::unordered_map<std::string, CP::SysReadDecorHandle<bool> > m_triggerdecos;

    Gaudi::Property<std::string> m_eleWPName
      { this, "eleWP", "","Electron ID + Iso working point" };
    SG::ReadDecorHandleKey<xAOD::ElectronContainer> m_eleWPDecorKey;
		      
    Gaudi::Property<std::string> m_muonWPName
      { this, "muonWP", "","Muon ID + Iso cuts" };
    SG::ReadDecorHandleKey<xAOD::MuonContainer> m_muonWPDecorKey;

    Gaudi::Property<std::vector<std::string>> m_triggers 
          {this, "triggerLists", {}, "Name list of trigger"};

    std::unordered_map<std::string, CP::SysWriteDecorHandle<bool> > m_Bbranches;
    std::vector<std::string> m_Bvarnames{      
      "pass_trigger_SLT", "pass_trigger_LTT", "pass_trigger_STT", "pass_trigger_DTT",
      "pass_baseline_SLT", "pass_baseline_LTT", "pass_baseline_STT", "pass_baseline_DTT",
      "pass_SLT", "pass_LTT", "pass_STT", "pass_DTT",
    };

    CP::SysWriteDecorHandle<bool> m_selected_el {"selected_el_%SYS%", this};
    CP::SysWriteDecorHandle<bool> m_selected_mu {"selected_mu_%SYS%", this};
    CP::SysWriteDecorHandle<bool> m_selected_tau {"selected_tau_%SYS%", this};

    /// \brief Setup sys-aware output decorations
    CP::SysFilterReporterParams m_filterParams {this, "HHbbtautau selection"};
    
    /// \brief Internal variables

    bool trigPassed_SLT;
    bool trigPassed_LTT;
    bool trigPassed_STT;
    bool trigPassed_DTT;
    bool TWO_JETS;
    bool TWO_BJETS;
    bool MBB_MASS;
    bool N_LEPTONS_CUT_LEPHAD;
    bool ONE_TAU;
    bool OS_CHARGE_LEPHAD;
    bool pass_baseline_SLT;
    bool pass_baseline_LTT;
    bool pass_SLT;
    bool pass_LTT;
    bool N_LEPTONS_CUT_HADHAD;
    bool TWO_TAU;
    bool OS_CHARGE_HADHAD;
    bool pass_baseline_STT;
    bool pass_baseline_DTT;
    bool pass_STT;
    bool pass_DTT;
    std::unordered_map<std::string, std::unordered_map<std::string, float>> m_pt_threshold;
    bool DTT_DeltaR_cut;


    bool is15;
    bool is16;
    bool is17;
    bool is18;
    bool is22;
    bool is23;


    bool is16PeriodA;
    bool is16PeriodB_D3;
    bool is16PeriodD4_end;
    bool is17PeriodB1_B4;
    bool is17PeriodB5_B7;
    bool is17PeriodB8_end;
    bool is18PeriodB_end;
    bool is18PeriodK_end;

    bool l1topo_disabled;

    

    void applyTriggerSelection(const xAOD::TauJetContainer* taus, const xAOD::EventInfo* event, const CP::SystematicSet& sys);
    void applyLepHadTriggerSelection(const xAOD::TauJetContainer* taus, const xAOD::EventInfo* event, const CP::SystematicSet& sys);
    void applySingleTauTriggerSelection(const xAOD::EventInfo* event, const CP::SystematicSet& sys);
    void applyDiTauTriggerSelection(const xAOD::EventInfo* event, const CP::SystematicSet& sys);

  };
}

#endif
