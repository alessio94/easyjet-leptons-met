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

  enum TriggerChannel
  {
    SLT = 0,
    LTT = 1,
    ETT = 2,
    ETT_4J12 = 3,
    MTT_2016 = 4,
    MTT_high = 5,
    MTT_low = 6,
    STT = 7,
    DTT = 8,
    DTT_2016 = 9,
    DTT_4J12 = 10,
    DTT_L1Topo = 11,
  };

  enum Var
  {
    ele = 0,
    mu = 1,
    leadingtau = 2,
    leadingtaumax = 3,
    subleadingtau = 4,
    leadingjet = 5,
    subleadingjet = 6,
  };


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

    CP::SysReadDecorHandle<unsigned int>
    m_rdmRunNumber {this, "randomRunNumber", "RandomRunNumber", "Random run number for MC"};

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
      "pass_trigger_SLT", "pass_trigger_LTT", "pass_trigger_STT",
      "pass_trigger_DTT_2016", "pass_trigger_DTT_4J12",
      "pass_trigger_DTT_L1Topo", "pass_trigger_DTT",
      "pass_baseline_SLT", "pass_baseline_LTT", "pass_baseline_STT",
      "pass_baseline_DTT_2016", "pass_baseline_DTT_4J12",
      "pass_baseline_DTT_L1Topo", "pass_baseline_DTT",
      "pass_SLT", "pass_LTT", "pass_STT",
      "pass_DTT_2016", "pass_DTT_4J12", "pass_DTT_L1Topo",  "pass_DTT"
    };

    CP::SysWriteDecorHandle<bool> m_selected_el {"selected_el_%SYS%", this};
    CP::SysWriteDecorHandle<bool> m_selected_mu {"selected_mu_%SYS%", this};
    CP::SysWriteDecorHandle<bool> m_selected_tau {"selected_tau_%SYS%", this};

    /// \brief Setup sys-aware output decorations
    CP::SysFilterReporterParams m_filterParams {this, "HHbbtautau selection"};
    
    /// \brief Internal variables

    std::unordered_map<HHBBTT::TriggerChannel, std::unordered_map<HHBBTT::Var, float>> m_pt_threshold;

    bool m_trigPassed_SLT;
    bool m_trigPassed_LTT;
    bool m_trigPassed_STT;
    bool m_trigPassed_DTT;
    bool m_trigPassed_DTT_2016;
    bool m_trigPassed_DTT_4J12;
    bool m_trigPassed_DTT_L1Topo;

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
    bool pass_baseline_DTT_2016;
    bool pass_baseline_DTT_4J12;
    bool pass_baseline_DTT_L1Topo;
    bool pass_baseline_DTT;
    bool pass_STT;
    bool pass_DTT_2016;
    bool pass_DTT_4J12;
    bool pass_DTT_L1Topo;
    bool pass_DTT;

    bool m_is15;
    bool m_is16;
    bool m_is17;
    bool m_is18;
    bool m_is22;
    bool m_is23;

    bool m_is16PeriodA;
    bool m_is16PeriodB_D3;
    bool m_is16PeriodD4_end;
    bool m_is17PeriodB1_B4;
    bool m_is17PeriodB5_B7;
    bool m_is17PeriodB8_end;
    bool m_is18PeriodB_end;
    bool m_is18PeriodK_end;
    bool m_l1topo_disabled;

    void applyTriggerSelection(const xAOD::EventInfo* event,
			       const xAOD::ElectronContainer* electrons,
			       const xAOD::MuonContainer* muons,
			       const xAOD::TauJetContainer* taus,
			       const xAOD::JetContainer* jets,
			       const CP::SystematicSet& sys);
    void applySingleLepTriggerSelection(const xAOD::EventInfo* event,
					const xAOD::ElectronContainer* electrons,
					const xAOD::MuonContainer* muons,
					const CP::SystematicSet& sys);
    void applyLepHadTriggerSelection(const xAOD::EventInfo* event,
				     const xAOD::ElectronContainer* electrons,
				     const xAOD::MuonContainer* muons,
				     const xAOD::TauJetContainer* taus,
				     const xAOD::JetContainer* jets,
				     const CP::SystematicSet& sys);
    void applySingleTauTriggerSelection(const xAOD::EventInfo* event,
					const xAOD::TauJetContainer* taus,
					const CP::SystematicSet& sys);
    void applyDiTauTriggerSelection(const xAOD::EventInfo* event,
				    const xAOD::TauJetContainer* taus,
				    const xAOD::JetContainer* jets,
				    const CP::SystematicSet& sys);

    void setRunNumberQuantities(unsigned int rdmNumber);

  };
}

#endif
