/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef BBTTANALYSIS_HHBBTTSELECTORALG
#define BBTTANALYSIS_HHBBTTSELECTORALG

#include <memory>

#include "AnaAlgorithm/AnaAlgorithm.h"
#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AsgDataHandles/ReadDecorHandle.h>

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

#include <EasyjetHub/CutManager.h>


#include "HHbbttChannels.h"

namespace HHBBTT
{

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
    Gaudi::Property<bool> m_useTriggerSel
      { this, "useTriggerSelections", true, "Apply trigger-related selections" };

    Gaudi::Property<std::vector<int>> m_years
      { this, "Years", false, "which years are running" };
    int m_year;
    
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
    CP::SysReadDecorHandle<char> m_tauWPDecorHandle{"", this};

    CP::SysReadDecorHandle<char> 
    m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};

    CP::SysReadDecorHandle<unsigned int>
    m_runNumber {this, "runNumber", "runNumber", "Runnumber"};

    CP::SysReadDecorHandle<unsigned int>
    m_rdmRunNumber {this, "randomRunNumber", "RandomRunNumber", "Random run number for MC"};

    Gaudi::Property<std::string> m_eleWPName
      { this, "eleWP", "","Electron ID + Iso working point" };
    CP::SysReadDecorHandle<char> m_eleWPDecorHandle{"", this};
		      
    Gaudi::Property<std::string> m_muonWPName
      { this, "muonWP", "","Muon ID + Iso cuts" };
    CP::SysReadDecorHandle<char> m_muonWPDecorHandle{"", this};

    CP::SysWriteDecorHandle<bool> m_selected_el {"selected_el_%SYS%", this};
    CP::SysWriteDecorHandle<bool> m_selected_mu {"selected_mu_%SYS%", this};
    CP::SysWriteDecorHandle<bool> m_selected_tau {"selected_tau_%SYS%", this};

    std::unordered_map<HHBBTT::TriggerChannel, std::string> m_triggerChannels =
      {
	{HHBBTT::SLT, "SLT"},
	{HHBBTT::LTT, "LTT"},
	{HHBBTT::ETT, "ETT"},
	{HHBBTT::ETT_4J12, "ETT_4J12"},
	{HHBBTT::MTT_2016, "MTT_2016"},
	{HHBBTT::MTT_high, "MTT_high"},
	{HHBBTT::MTT_low, "MTT_low"},
	{HHBBTT::STT, "STT"},
	{HHBBTT::DTT, "DTT"},
	{HHBBTT::DTT_2016, "DTT_2016"},
	{HHBBTT::DTT_4J12, "DTT_4J12"},
	{HHBBTT::DTT_L1Topo, "DTT_L1Topo"}
      };

    std::unordered_map<HHBBTT::TriggerChannel,
      SG::ReadDecorHandleKey<xAOD::EventInfo> > m_trigPass_DecorKey;

    std::unordered_map<HHBBTT::TriggerChannel,
      SG::ReadDecorHandleKey<xAOD::MuonContainer> > m_mu_trigMatch_DecorKey;
    std::unordered_map<HHBBTT::TriggerChannel,
      SG::ReadDecorHandleKey<xAOD::ElectronContainer> > m_ele_trigMatch_DecorKey;
    std::unordered_map<HHBBTT::TriggerChannel,
      SG::ReadDecorHandleKey<xAOD::TauJetContainer> > m_tau_trigMatch_DecorKey;

    /// \brief Setup sys-aware output decorations
    CP::SysFilterReporterParams m_filterParams {this, "HHbbtautau selection"};
    
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
    };

    /// \brief Cutflow Variables
    CutManager m_bbttCuts;
    Gaudi::Property<std::vector<std::string>> m_inputCutList{this, "cutList", {}};
    std::vector<HHBBTT::Booleans> m_inputCutKeys;
    Gaudi::Property<bool> m_saveCutFlow{this, "saveCutFlow", false};
    long long int m_total_events{0};
    float m_total_mcEventWeight{0.f};
    CP::SysReadDecorHandle<float>
      m_generatorWeight{ this, "generatorWeight", "generatorWeight_%SYS%", "MC event weights" };

    /// \brief Internal variables

    std::unordered_map<HHBBTT::TriggerChannel, std::unordered_map<HHBBTT::Var, float>> m_pt_threshold;

    StatusCode initialiseCutflow();

    typedef std::unordered_map<HHBBTT::TriggerChannel, SG::ReadDecorHandle<xAOD::EventInfo, bool> > trigPassReadDecoMap;
    typedef std::unordered_map<HHBBTT::TriggerChannel, SG::ReadDecorHandle<xAOD::MuonContainer, bool> > muTrigMatchReadDecoMap;
    typedef std::unordered_map<HHBBTT::TriggerChannel, SG::ReadDecorHandle<xAOD::ElectronContainer, bool> > eleTrigMatchReadDecoMap;
    typedef std::unordered_map<HHBBTT::TriggerChannel, SG::ReadDecorHandle<xAOD::TauJetContainer, bool> > tauTrigMatchReadDecoMap;

    void applyTriggerSelection
      (const xAOD::EventInfo* event, const trigPassReadDecoMap& triggerdecos,
       const xAOD::Electron* ele, const eleTrigMatchReadDecoMap& ele_trigMatchDecos,
       const xAOD::Muon* mu, const muTrigMatchReadDecoMap& mu_trigMatchDecos,
       const xAOD::TauJet* tau0, const xAOD::TauJet* tau1,
       const tauTrigMatchReadDecoMap& tau_trigMatchDecos,
       const xAOD::Jet* jet0, const xAOD::Jet* jet1);

    void applySingleLepTriggerSelection
      (const xAOD::EventInfo* event, const trigPassReadDecoMap& triggerdecos,
       const xAOD::Electron* ele, const eleTrigMatchReadDecoMap& ele_trigMatchDecos,
       const xAOD::Muon* mu, const muTrigMatchReadDecoMap& mu_trigMatchDecos);
    void applyLepHadTriggerSelection
      (const xAOD::EventInfo* event, const trigPassReadDecoMap& triggerdecos,
       const xAOD::Electron* ele, const eleTrigMatchReadDecoMap& ele_trigMatchDecos,
       const xAOD::Muon* mu, const muTrigMatchReadDecoMap& mu_trigMatchDecos,
       const xAOD::TauJet* tau, const tauTrigMatchReadDecoMap& tau_trigMatchDecos,
       const xAOD::Jet* jet0, const xAOD::Jet* jet1);
    void applySingleTauTriggerSelection
      (const xAOD::EventInfo* event, const trigPassReadDecoMap& triggerdecos,
       const xAOD::TauJet* tau, const tauTrigMatchReadDecoMap& tau_trigMatchDecos);
    void applyDiTauTriggerSelection
      (const xAOD::EventInfo* event, const trigPassReadDecoMap& triggerdecos,
       const xAOD::TauJet* tau0, const xAOD::TauJet* tau1,
       const tauTrigMatchReadDecoMap& tau_trigMatchDecos,
       const xAOD::Jet* jet0, const xAOD::Jet* jet1);

    void setRunNumberQuantities(unsigned int runNumber);

  };
}

#endif
