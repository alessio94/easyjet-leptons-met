/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef BBBBTTANALYSIS_HHHBBBBTTSELECTORALG
#define BBBBTTANALYSIS_HHHBBBBTTSELECTORALG

#include <memory>

#include "AnaAlgorithm/AnaAlgorithm.h"
#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AsgDataHandles/ReadDecorHandle.h>

#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysFilterReporterParams.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODTau/TauJetContainer.h>

#include <EasyjetHub/CutManager.h>

#include "HHHbbbbttEnums.h"

namespace HHHBBBBTT
{

  /// \brief An algorithm for counting containers
  class HHHbbbbttSelectorAlg final : public EL::AnaAlgorithm
  {
    /// \brief The standard constructor
public:
    HHHbbbbttSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

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

    std::vector<HHHBBBBTT::Channel> m_channels;

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };
    Gaudi::Property<bool> m_useTriggerSel
      { this, "useTriggerSelections", true, "Apply trigger-related selections" };
    Gaudi::Property<bool> m_doAntiIDRegions
      { this, "doAntiIDRegions", false, "Select anti-ID taus for fake estimates" };
    Gaudi::Property<bool> m_do0BRegions
      { this, "do0BRegions", false, "Add 0B signal regions" };
    Gaudi::Property<bool> m_do1BRegions
      { this, "do1BRegions", false, "Add 1B signal regions" };
    Gaudi::Property<bool> m_do2BRegions
      { this, "do2BRegions", false, "Add 2B signal regions" };
    Gaudi::Property<bool> m_do3BRegions
      { this, "do3BRegions", false, "Add 3B signal regions" };
    Gaudi::Property<bool> m_useNonIsoLeptons
      { this, "useNonIsoLeptons", false, "use NonIso lepton wps" };

    Gaudi::Property<bool> m_bypass
      { this, "bypass", false, "Run selector algorithm in pass-through mode" };

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::JetContainer>
      m_jetHandle{ this, "jets", "bbbbttAnalysisJets_%SYS%", "Jet container to read" };
    
    CP::SysReadHandle<xAOD::ElectronContainer>
    m_electronHandle{ this, "electrons", "bbbbttAnalysisElectrons_%SYS%", "Electron container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
    m_muonHandle{ this, "muons", "bbbbttAnalysisMuons_%SYS%", "Muon container to read" };

    CP::SysReadHandle<xAOD::TauJetContainer>
    m_tauHandle{ this, "taus", "bbbbttAnalysisTaus_%SYS%", "Tau container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    Gaudi::Property<std::string> m_tauWPName
      { this, "tauWP", "", "Tau ID working point" };
    CP::SysReadDecorHandle<char> m_tauWPDecorHandle{"", this};
    CP::SysReadDecorHandle<char> m_antiTauDecorHandle{"isAntiTau", this};

    CP::SysReadDecorHandle<char> 
    m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};

    CP::SysReadDecorHandle<unsigned int> m_year
      {this, "year", "dataTakingYear", ""};

    CP::SysReadDecorHandle<bool> m_is2016_periodA
      {this, "is2016_periodA", "is2016_periodA", ""};
    CP::SysReadDecorHandle<bool> m_is2016_periodB_D3
      {this, "is2016_periodB_D3", "is2016_periodB_D3", ""};
    CP::SysReadDecorHandle<bool> m_is2022_75bunches
      {this, "is2022_75bunches", "is2022_75bunches", ""};

    Gaudi::Property<std::vector<std::string>> m_eleWPNames
      { this, "eleWPs", {},"Electron working point names" };
    Gaudi::Property<std::vector<std::string>> m_muonWPNames
      { this, "muonWPs", {},"Muon working point names" };

    typedef std::unordered_map<HHHBBBBTT::LepSelWpDeco, CP::SysReadDecorHandle<char>> leptonDecoMap;
    leptonDecoMap m_eleWPDecorHandleMap;   
    leptonDecoMap m_muonWPDecorHandleMap;

    CP::SysWriteDecorHandle<bool> m_selected_el {"selected_el_%SYS%", this};
    CP::SysWriteDecorHandle<bool> m_selected_el_isIso {"selected_el_isIso_%SYS%", this};
   
    CP::SysWriteDecorHandle<bool> m_selected_mu {"selected_mu_%SYS%", this};
    CP::SysWriteDecorHandle<bool> m_selected_mu_isIso {"selected_mu_isIso_%SYS%", this};
   
    CP::SysWriteDecorHandle<bool> m_selected_tau {"selected_tau_%SYS%", this};

    std::unordered_map<HHHBBBBTT::TriggerChannel, std::string> m_triggerChannels =
      {
	{HHHBBBBTT::SLT, "SLT"},
	{HHHBBBBTT::LTT, "LTT"},
	{HHHBBBBTT::ETT, "ETT"},
	{HHHBBBBTT::ETT_4J12, "ETT_4J12"},
	{HHHBBBBTT::MTT_2016, "MTT_2016"},
	{HHHBBBBTT::MTT_high, "MTT_high"},
	{HHHBBBBTT::MTT_low, "MTT_low"},
	{HHHBBBBTT::STT, "STT"},
	{HHHBBBBTT::DTT, "DTT"},
	{HHHBBBBTT::DTT_2016, "DTT_2016"},
	{HHHBBBBTT::DTT_4J12, "DTT_4J12"},
	{HHHBBBBTT::DTT_L1Topo, "DTT_L1Topo"},
	{HHHBBBBTT::DTT_4J12_delayed, "DTT_4J12_delayed"},
	{HHHBBBBTT::DTT_L1Topo_delayed, "DTT_L1Topo_delayed"},
	{HHHBBBBTT::DBT, "DBT"},
  {HHHBBBBTT::BTT, "BTT"},
      };

    std::unordered_map<HHHBBBBTT::TriggerChannel,
      SG::ReadDecorHandleKey<xAOD::EventInfo> > m_trigPass_DecorKey;

    std::unordered_map<HHHBBBBTT::TriggerChannel,
      SG::ReadDecorHandleKey<xAOD::MuonContainer> > m_mu_trigMatch_DecorKey;
    std::unordered_map<HHHBBBBTT::TriggerChannel,
      SG::ReadDecorHandleKey<xAOD::ElectronContainer> > m_ele_trigMatch_DecorKey;
    std::unordered_map<HHHBBBBTT::TriggerChannel,
      SG::ReadDecorHandleKey<xAOD::TauJetContainer> > m_tau_trigMatch_DecorKey;

    /// \brief Setup sys-aware output decorations
    CP::SysFilterReporterParams m_filterParams {this, "HHHbbbbtautau selection"};
    
    /// \brief Booleans
    /*
     * We have a lot of booleans. Most of these have to be saved in the output root file and some used for the cutflow algorithm.
     * In both cases, additionally to the value of the boolean, its name is required.
     * In order to easily access both and having both connected, we use the enum Booleans defined above.
     * We then define a map m_bools mapping the enum to the boolean value and a map m_boolnames mapping the enum to the boolean name.
     * Additionally, m_Bbranches is a map of SysWriteDecorHandles taking care of saving the variables.
     */
    std::unordered_map < HHHBBBBTT::Booleans, CP::SysWriteDecorHandle<bool> > m_Bbranches;
    std::unordered_map < HHHBBBBTT::Booleans, bool > m_bools;
    std::unordered_map < HHHBBBBTT::Booleans, std::string > m_boolnames{
    {HHHBBBBTT::pass_trigger_SR, "pass_trigger_SR"},
    {HHHBBBBTT::pass_trigger_SLT, "pass_trigger_SLT"},
    {HHHBBBBTT::pass_trigger_LTT, "pass_trigger_LTT"},
    {HHHBBBBTT::pass_trigger_STT, "pass_trigger_STT"},
    {HHHBBBBTT::pass_trigger_DTT, "pass_trigger_DTT"},
    {HHHBBBBTT::pass_trigger_DTT_2016, "pass_trigger_DTT_2016"},
    {HHHBBBBTT::pass_trigger_DTT_4J12, "pass_trigger_DTT_4J12"},
    {HHHBBBBTT::pass_trigger_DTT_L1Topo, "pass_trigger_DTT_L1Topo"},
    {HHHBBBBTT::pass_trigger_DTT_4J12_delayed, "pass_trigger_DTT_4J12_delayed"},
    {HHHBBBBTT::pass_trigger_DTT_L1Topo_delayed, "pass_trigger_DTT_L1Topo_delayed"},
    {HHHBBBBTT::pass_trigger_DBT, "pass_trigger_DBT"},
    {HHHBBBBTT::pass_trigger_BTT, "pass_trigger_BTT"},
    {HHHBBBBTT::TWO_JETS, "TWO_JETS"},
    {HHHBBBBTT::TWO_BJETS, "TWO_BJETS"},
    {HHHBBBBTT::ONE_BJET, "ONE_BJET"},
    {HHHBBBBTT::ZERO_BJETS, "ZERO_BJETS"},
    {HHHBBBBTT::THREE_BJETS, "THREE_BJETS"},
    {HHHBBBBTT::FOUR_BJETS, "FOUR_BJETS"},
    {HHHBBBBTT::MBB_MASS, "MBB_MASS"},
    {HHHBBBBTT::MTAUTAU_VIS_MASS, "MTAUTAU_VIS_MASS"},
    {HHHBBBBTT::N_LEPTONS_CUT_LEPHAD, "N_LEPTONS_CUT_LEPHAD"},
    {HHHBBBBTT::N_LEPTONS_CUT_ANTIISOLEPHAD, "N_LEPTONS_CUT_ANTIISOLEPHAD"},
    {HHHBBBBTT::ONE_TAU, "ONE_TAU"},
    {HHHBBBBTT::OS_CHARGE_LEPHAD, "OS_CHARGE_LEPHAD"},
    {HHHBBBBTT::OS_CHARGE_LEPTONS, "OS_CHARGE_LEPTONS"},
    {HHHBBBBTT::pass_baseline_SLT, "pass_baseline_SLT"},
    {HHHBBBBTT::pass_baseline_LTT, "pass_baseline_LTT"},
    {HHHBBBBTT::pass_SLT_4B, "pass_SLT_4B"},
    {HHHBBBBTT::pass_LTT_4B, "pass_LTT_4B"},
    {HHHBBBBTT::pass_SLT_3B, "pass_SLT_3B"},
    {HHHBBBBTT::pass_LTT_3B, "pass_LTT_3B"},
    {HHHBBBBTT::pass_SLT_2B, "pass_SLT_2B"},
    {HHHBBBBTT::pass_LTT_2B, "pass_LTT_2B"},
    {HHHBBBBTT::pass_SLT_1B, "pass_SLT_1B"},
    {HHHBBBBTT::pass_LTT_1B, "pass_LTT_1B"},
    {HHHBBBBTT::pass_SLT_0B, "pass_SLT_0B"},
    {HHHBBBBTT::pass_LTT_0B, "pass_LTT_0B"},
    {HHHBBBBTT::N_LEPTONS_CUT_HADHAD, "N_LEPTONS_CUT_HADHAD"},
    {HHHBBBBTT::TWO_TAU, "TWO_TAU"},
    {HHHBBBBTT::OS_CHARGE_HADHAD, "OS_CHARGE_HADHAD"},
    {HHHBBBBTT::pass_baseline_STT, "pass_baseline_STT"},
    {HHHBBBBTT::pass_baseline_DTT_2016, "pass_baseline_DTT_2016"},
    {HHHBBBBTT::pass_baseline_DTT_4J12, "pass_baseline_DTT_4J12"},
    {HHHBBBBTT::pass_baseline_DTT_L1Topo, "pass_baseline_DTT_L1Topo"},
    {HHHBBBBTT::pass_baseline_DTT, "pass_baseline_DTT"},
    {HHHBBBBTT::pass_baseline_DBT, "pass_baseline_DBT"},
    {HHHBBBBTT::pass_baseline_BTT, "pass_baseline_BTT"},
    {HHHBBBBTT::pass_baseline_SR, "pass_baseline_SR"},
    {HHHBBBBTT::pass_STT_4B, "pass_STT_4B"},
    {HHHBBBBTT::pass_DTT_2016_4B, "pass_DTT_2016_4B"},
    {HHHBBBBTT::pass_DTT_4J12_4B, "pass_DTT_4J12_4B"},
    {HHHBBBBTT::pass_DTT_L1Topo_4B, "pass_DTT_L1Topo_4B"},
    {HHHBBBBTT::pass_DTT_4J12_delayed_4B, "pass_DTT_4J12_delayed_4B"},
    {HHHBBBBTT::pass_DTT_L1Topo_delayed_4B, "pass_DTT_L1Topo_delayed_4B"},
    {HHHBBBBTT::pass_DTT_4B, "pass_DTT_4B"},
    {HHHBBBBTT::pass_DBT_4B, "pass_DBT_4B"},
    {HHHBBBBTT::pass_BTT_4B, "pass_BTT_4B"},
    {HHHBBBBTT::pass_SR_4B, "pass_SR_4B"},
    {HHHBBBBTT::pass_STT_3B, "pass_STT_3B"},
    {HHHBBBBTT::pass_DTT_2016_3B, "pass_DTT_2016_3B"},
    {HHHBBBBTT::pass_DTT_4J12_3B, "pass_DTT_4J12_3B"},
    {HHHBBBBTT::pass_DTT_L1Topo_3B, "pass_DTT_L1Topo_3B"},
    {HHHBBBBTT::pass_DTT_4J12_delayed_3B, "pass_DTT_4J12_delayed_3B"},
    {HHHBBBBTT::pass_DTT_L1Topo_delayed_3B, "pass_DTT_L1Topo_delayed_3B"},
    {HHHBBBBTT::pass_DTT_3B, "pass_DTT_3B"},
    {HHHBBBBTT::pass_DBT_3B, "pass_DBT_3B"},
    {HHHBBBBTT::pass_BTT_3B, "pass_BTT_3B"},
    {HHHBBBBTT::pass_SR_3B, "pass_SR_3B"},
    {HHHBBBBTT::pass_STT_2B, "pass_STT_2B"},
    {HHHBBBBTT::pass_DTT_2016_2B, "pass_DTT_2016_2B"},
    {HHHBBBBTT::pass_DTT_4J12_2B, "pass_DTT_4J12_2B"},
    {HHHBBBBTT::pass_DTT_L1Topo_2B, "pass_DTT_L1Topo_2B"},
    {HHHBBBBTT::pass_DTT_4J12_delayed_2B, "pass_DTT_4J12_delayed_2B"},
    {HHHBBBBTT::pass_DTT_L1Topo_delayed_2B, "pass_DTT_L1Topo_delayed_2B"},
    {HHHBBBBTT::pass_DTT_2B, "pass_DTT_2B"},
    {HHHBBBBTT::pass_DBT_2B, "pass_DBT_2B"},
    {HHHBBBBTT::pass_BTT_2B, "pass_BTT_2B"},
    {HHHBBBBTT::pass_SR_2B, "pass_SR_2B"},
    {HHHBBBBTT::pass_STT_1B, "pass_STT_1B"},
    {HHHBBBBTT::pass_DTT_2016_1B, "pass_DTT_2016_1B"},
    {HHHBBBBTT::pass_DTT_4J12_1B, "pass_DTT_4J12_1B"},
    {HHHBBBBTT::pass_DTT_L1Topo_1B, "pass_DTT_L1Topo_1B"},
    {HHHBBBBTT::pass_DTT_4J12_delayed_1B, "pass_DTT_4J12_delayed_1B"},
    {HHHBBBBTT::pass_DTT_L1Topo_delayed_1B, "pass_DTT_L1Topo_delayed_1B"},
    {HHHBBBBTT::pass_DTT_1B, "pass_DTT_1B"},
    {HHHBBBBTT::pass_DBT_1B, "pass_DBT_1B"},
    {HHHBBBBTT::pass_BTT_1B, "pass_BTT_1B"},
    {HHHBBBBTT::pass_SR_1B, "pass_SR_1B"},
    {HHHBBBBTT::pass_STT_0B, "pass_STT_0B"},
    {HHHBBBBTT::pass_DTT_2016_0B, "pass_DTT_2016_0B"},
    {HHHBBBBTT::pass_DTT_4J12_0B, "pass_DTT_4J12_0B"},
    {HHHBBBBTT::pass_DTT_L1Topo_0B, "pass_DTT_L1Topo_0B"},
    {HHHBBBBTT::pass_DTT_4J12_delayed_0B, "pass_DTT_4J12_delayed_0B"},
    {HHHBBBBTT::pass_DTT_L1Topo_delayed_0B, "pass_DTT_L1Topo_delayed_0B"},
    {HHHBBBBTT::pass_DTT_0B, "pass_DTT_0B"},
    {HHHBBBBTT::pass_DBT_0B, "pass_DBT_0B"},
    {HHHBBBBTT::pass_BTT_0B, "pass_BTT_0B"},
    {HHHBBBBTT::pass_SR_0B, "pass_SR_0B"},
    {HHHBBBBTT::pass_baseline_LepHad, "pass_baseline_LepHad"},
    {HHHBBBBTT::pass_baseline_HadHad, "pass_baseline_HadHad"},
    {HHHBBBBTT::pass_LepHad_4B, "pass_LepHad_4B"},
    {HHHBBBBTT::pass_HadHad_4B, "pass_HadHad_4B"},
    {HHHBBBBTT::pass_LepHad_3B, "pass_LepHad_3B"},
    {HHHBBBBTT::pass_HadHad_3B, "pass_HadHad_3B"},
    {HHHBBBBTT::pass_LepHad_2B, "pass_LepHad_2B"},
    {HHHBBBBTT::pass_HadHad_2B, "pass_HadHad_2B"},
    {HHHBBBBTT::pass_LepHad_1B, "pass_LepHad_1B"},
    {HHHBBBBTT::pass_HadHad_1B, "pass_HadHad_1B"},
    {HHHBBBBTT::pass_LepHad_0B, "pass_LepHad_0B"},
    {HHHBBBBTT::pass_HadHad_0B, "pass_HadHad_0B"},
    {HHHBBBBTT::pass_LepHad, "pass_LepHad"},
    {HHHBBBBTT::pass_HadHad, "pass_HadHad"},
    {HHHBBBBTT::pass_ZCR, "pass_ZCR"},
    {HHHBBBBTT::pass_TopEMuCR, "pass_TopEMuCR"},
    {HHHBBBBTT::pass_AntiIsoLepHad, "pass_AntiIsoLepHad"},
    };

    /// \brief Cutflow Variables
    CutManager m_bbbbttCuts;
    Gaudi::Property<std::vector<std::string>> m_inputCutList{this, "cutList", {}};
    std::vector<HHHBBBBTT::Booleans> m_inputCutKeys;
    Gaudi::Property<bool> m_saveCutFlow{this, "saveCutFlow", false};
    long long int m_total_events{0};
    double m_total_mcEventWeight{0.0};
    CP::SysReadDecorHandle<float>
      m_generatorWeight{ this, "generatorWeight", "generatorWeight_%SYS%", "MC event weights" };

    /// \brief Internal variables

    std::unordered_map<HHHBBBBTT::TriggerChannel, std::unordered_map<HHHBBBBTT::Var, float>> m_pt_threshold;

    StatusCode initialiseCutflow();

    typedef std::unordered_map<HHHBBBBTT::TriggerChannel, SG::ReadDecorHandle<xAOD::EventInfo, bool> > trigPassReadDecoMap;
    typedef std::unordered_map<HHHBBBBTT::TriggerChannel, SG::ReadDecorHandle<xAOD::MuonContainer, bool> > muTrigMatchReadDecoMap;
    typedef std::unordered_map<HHHBBBBTT::TriggerChannel, SG::ReadDecorHandle<xAOD::ElectronContainer, bool> > eleTrigMatchReadDecoMap;
    typedef std::unordered_map<HHHBBBBTT::TriggerChannel, SG::ReadDecorHandle<xAOD::TauJetContainer, bool> > tauTrigMatchReadDecoMap;

    void applyTriggerSelection
      (const xAOD::EventInfo* event, const trigPassReadDecoMap& triggerdecos,
       const xAOD::Electron* ele, const eleTrigMatchReadDecoMap& ele_trigMatchDecos,
       const xAOD::Muon* mu, const muTrigMatchReadDecoMap& mu_trigMatchDecos,
       const xAOD::TauJet* tau0, const xAOD::TauJet* tau1,
       const tauTrigMatchReadDecoMap& tau_trigMatchDecos,
       const xAOD::Jet* jet0, const xAOD::Jet* jet1,
       const xAOD::Jet* eta_lt2p5_jet0, const xAOD::Jet* eta_lt2p5_jet1,
       const xAOD::Jet* eta_lt2p8_jet0);

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
       const xAOD::Jet* jet0, const xAOD::Jet* eta_lt2p5_jet0, 
       const xAOD::Jet* eta_lt2p5_jet1, const xAOD::Jet* eta_lt2p8_jet0);
    void applyDiBJetTriggerSelection
      (const xAOD::EventInfo* event, const trigPassReadDecoMap& triggerdecos,
       const xAOD::Jet* eta_lt2p5_jet0, const xAOD::Jet* eta_lt2p5_jet1);
    void applyBjetTauTriggerSelection
      (const xAOD::EventInfo* event, const trigPassReadDecoMap& triggerdecos,
       const xAOD::TauJet* tau0, const xAOD::Jet* eta_lt2p5_jet0);

    void setThresholds(const xAOD::EventInfo* event,
		       const CP::SystematicSet& sys);
    
    void fillLeptonWpDecoMap(const std::vector<std::string>& wpNames, leptonDecoMap& decoMap);
  };
}

#endif
