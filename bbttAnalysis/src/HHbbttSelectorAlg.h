/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef BBTTANALYSIS_HHBBTTSELECTORALG
#define BBTTANALYSIS_HHBBTTSELECTORALG

#include <memory>

#include "AnaAlgorithm/AnaAlgorithm.h"
#include <AsgDataHandles/ReadDecorHandleKey.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysFilterReporterParams.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODTau/TauJetContainer.h>
#include <xAODMissingET/MissingETContainer.h>

#include "TriggerMatchingTool/IMatchingTool.h"

#include <EasyjetHub/CutManager.h>


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

  enum Booleans
  {
    pass_trigger_SLT,
    pass_trigger_LTT,
    pass_trigger_STT,
    pass_trigger_DTT,
    pass_trigger_DTT_2016,
    pass_trigger_DTT_4J12,
    pass_trigger_DTT_L1Topo,

    TWO_JETS,
    TWO_BJETS,
    ONE_BJET,
    MBB_MASS,
    N_LEPTONS_CUT_LEPHAD,
    ONE_TAU,
    OS_CHARGE_LEPHAD,
    pass_baseline_SLT,
    pass_baseline_LTT,
    pass_SLT,
    pass_LTT,
    pass_SLT_1B,
    pass_LTT_1B,
    N_LEPTONS_CUT_HADHAD,
    TWO_TAU,
    OS_CHARGE_HADHAD,
    OS_CHARGE_LEPTONS,
    pass_baseline_STT,
    pass_baseline_DTT_2016,
    pass_baseline_DTT_4J12,
    pass_baseline_DTT_L1Topo,
    pass_baseline_DTT,
    pass_STT,
    pass_DTT_2016,
    pass_DTT_4J12,
    pass_DTT_L1Topo,
    pass_DTT,
    pass_STT_1B,
    pass_DTT_2016_1B,
    pass_DTT_4J12_1B,
    pass_DTT_L1Topo_1B,
    pass_DTT_1B,
    pass_ZCR,
    pass_TopEMuCR,

    is15,
    is16,
    is17,
    is18,
    is22,
    is23,

    is16PeriodA,
    is16PeriodB_D3,
    is16PeriodD4_end,
    is17PeriodB1_B4,
    is17PeriodB5_B7,
    is17PeriodB8_end,
    is18PeriodB_end,
    is18PeriodK_end,
    is22_75bunches,
    is23_75bunches,
    is23_400bunches,
    l1topo_disabled,
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

    CP::SysWriteDecorHandle<bool> m_selected_el {"selected_el_%SYS%", this};
    CP::SysWriteDecorHandle<bool> m_selected_mu {"selected_mu_%SYS%", this};
    CP::SysWriteDecorHandle<bool> m_selected_tau {"selected_tau_%SYS%", this};

    /// \brief Setup sys-aware output decorations
    CP::SysFilterReporterParams m_filterParams {this, "HHbbtautau selection"};

    ToolHandle<Trig::IMatchingTool> m_matchingTool{this, "trigMatchingTool", "",
	"Trigger matching tool"};

    Gaudi::Property<bool> m_useDiTauTrigMatch{this, "diTauTrigMatch", true, "Run di-tau trigger matching"};
    
    /// \brief Booleans
    /*
     * We have a lot of booleans. Most of these have to be saved in the output root file and some used for the cutflow algorithm.
     * In both cases, additionally to the value of the boolean, its name is required.
     * In order to easily access both and having both connected, we use the enum Booleans defined above.
     * We then define a map m_bools mapping the enum to the boolean value and a map m_boolnames mapping the enum to the boolean name.
     * Additionally, m_Bbranches is a map of SysWriteDecorHandles taking care of saving the variables.
     */
    std::unordered_map < HHBBTT::Booleans, CP::SysWriteDecorHandle<bool> > m_Bbranches;
    std::unordered_map < HHBBTT::Booleans, bool > m_bools;
    std::unordered_map < HHBBTT::Booleans, std::string > m_boolnames{
    {HHBBTT::pass_trigger_SLT, "pass_trigger_SLT"},
    {HHBBTT::pass_trigger_LTT, "pass_trigger_LTT"},
    {HHBBTT::pass_trigger_STT, "pass_trigger_STT"},
    {HHBBTT::pass_trigger_DTT, "pass_trigger_DTT"},
    {HHBBTT::pass_trigger_DTT_2016, "pass_trigger_DTT_2016"},
    {HHBBTT::pass_trigger_DTT_4J12, "pass_trigger_DTT_4J12"},
    {HHBBTT::pass_trigger_DTT_L1Topo, "pass_trigger_DTT_L1Topo"},
    {HHBBTT::TWO_JETS, "TWO_JETS"},
    {HHBBTT::TWO_BJETS, "TWO_BJETS"},
    {HHBBTT::ONE_BJET, "ONE_BJET"},
    {HHBBTT::MBB_MASS, "MBB_MASS"},
    {HHBBTT::N_LEPTONS_CUT_LEPHAD, "N_LEPTONS_CUT_LEPHAD"},
    {HHBBTT::ONE_TAU, "ONE_TAU"},
    {HHBBTT::OS_CHARGE_LEPHAD, "OS_CHARGE_LEPHAD"},
    {HHBBTT::OS_CHARGE_LEPTONS, "OS_CHARGE_LEPTONS"},
    {HHBBTT::pass_baseline_SLT, "pass_baseline_SLT"},
    {HHBBTT::pass_baseline_LTT, "pass_baseline_LTT"},
    {HHBBTT::pass_SLT, "pass_SLT"},
    {HHBBTT::pass_LTT, "pass_LTT"},
    {HHBBTT::pass_SLT_1B, "pass_SLT_1B"},
    {HHBBTT::pass_LTT_1B, "pass_LTT_1B"},
    {HHBBTT::N_LEPTONS_CUT_HADHAD, "N_LEPTONS_CUT_HADHAD"},
    {HHBBTT::TWO_TAU, "TWO_TAU"},
    {HHBBTT::OS_CHARGE_HADHAD, "OS_CHARGE_HADHAD"},
    {HHBBTT::pass_baseline_STT, "pass_baseline_STT"},
    {HHBBTT::pass_baseline_DTT_2016, "pass_baseline_DTT_2016"},
    {HHBBTT::pass_baseline_DTT_4J12, "pass_baseline_DTT_4J12"},
    {HHBBTT::pass_baseline_DTT_L1Topo, "pass_baseline_DTT_L1Topo"},
    {HHBBTT::pass_baseline_DTT, "pass_baseline_DTT"},
    {HHBBTT::pass_STT, "pass_STT"},
    {HHBBTT::pass_DTT_2016, "pass_DTT_2016"},
    {HHBBTT::pass_DTT_4J12, "pass_DTT_4J12"},
    {HHBBTT::pass_DTT_L1Topo, "pass_DTT_L1Topo"},
    {HHBBTT::pass_DTT, "pass_DTT"},
    {HHBBTT::pass_STT_1B, "pass_STT_1B"},
    {HHBBTT::pass_DTT_2016_1B, "pass_DTT_2016_1B"},
    {HHBBTT::pass_DTT_4J12_1B, "pass_DTT_4J12_1B"},
    {HHBBTT::pass_DTT_L1Topo_1B, "pass_DTT_L1Topo_1B"},
    {HHBBTT::pass_DTT_1B, "pass_DTT_1B"},
    {HHBBTT::pass_ZCR, "pass_ZCR"},
    {HHBBTT::pass_TopEMuCR, "pass_TopEMuCR"},
    {HHBBTT::is15, "is15"},
    {HHBBTT::is16, "is16"},
    {HHBBTT::is17, "is17"},
    {HHBBTT::is18, "is18"},
    {HHBBTT::is22, "is22"},
    {HHBBTT::is23, "is23"},
    {HHBBTT::is16PeriodA, "is16PeriodA"},
    {HHBBTT::is16PeriodB_D3, "is16PeriodB_D3"},
    {HHBBTT::is16PeriodD4_end, "is16PeriodD4_end"},
    {HHBBTT::is17PeriodB1_B4, "is17PeriodB1_B4"},
    {HHBBTT::is17PeriodB5_B7, "is17PeriodB5_B7"},
    {HHBBTT::is17PeriodB8_end, "is17PeriodB8_end"},
    {HHBBTT::is18PeriodB_end, "is18PeriodB_end"},
    {HHBBTT::is18PeriodK_end, "is18PeriodK_end"},
    {HHBBTT::is22_75bunches, "is22_75bunches"},
    {HHBBTT::is23_75bunches, "is23_75bunches"},
    {HHBBTT::is23_400bunches, "is23_400bunches"},
    {HHBBTT::l1topo_disabled, "l1topo_disabled"},
    };

    /// \brief Cutflow Variables
    CutManager m_bbttCuts;
    Gaudi::Property<std::vector<std::string>> m_inputCutList{this, "cutList", {}};
    std::vector<HHBBTT::Booleans> m_inputCutKeys;
    Gaudi::Property<bool> m_saveCutFlow{this, "saveCutFlow", false};
    long long int m_total_events{0};

    /// \brief Internal variables

    std::unordered_map<HHBBTT::TriggerChannel, std::unordered_map<HHBBTT::Var, float>> m_pt_threshold;

    StatusCode initialiseCutflow();
    void applyTriggerSelection(const xAOD::EventInfo* event,
			       const xAOD::Electron* ele, const xAOD::Muon* mu,
			       const xAOD::TauJet* tau0, const xAOD::TauJet* tau1,
			       const xAOD::Jet* jet0, const xAOD::Jet* jet1,
			       const CP::SystematicSet& sys);
    void applySingleLepTriggerSelection(const xAOD::EventInfo* event,
					const xAOD::Electron* ele,
					const xAOD::Muon* mu,
					const CP::SystematicSet& sys);
    void applyLepHadTriggerSelection(const xAOD::EventInfo* event,
				     const xAOD::Electron* ele, const xAOD::Muon* mu,
				     const xAOD::TauJet* tau,
				     const xAOD::Jet* jet0, const xAOD::Jet* jet1,
				     const CP::SystematicSet& sys);
    void applySingleTauTriggerSelection(const xAOD::EventInfo* event,
					const xAOD::TauJet* tau0,
					const CP::SystematicSet& sys);
    void applyDiTauTriggerSelection(const xAOD::EventInfo* event,
				    const xAOD::TauJet* tau0, const xAOD::TauJet* tau1,
				    const xAOD::Jet* jet0, const xAOD::Jet* jet1,
				    const CP::SystematicSet& sys);

    void setRunNumberQuantities(unsigned int rdmNumber);

  };
}

#endif
