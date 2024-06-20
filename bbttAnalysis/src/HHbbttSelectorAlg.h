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

#include "HHbbttEnums.h"

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
    Gaudi::Property<bool> m_useTriggerSel
      { this, "useTriggerSelections", true, "Apply trigger-related selections" };
    Gaudi::Property<bool> m_doAntiIDRegions
      { this, "doAntiIDRegions", false, "Select anti-ID taus for fake estimates" };
    
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

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

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
	{HHBBTT::DTT_L1Topo, "DTT_L1Topo"},
	{HHBBTT::DTT_4J12_delayed, "DTT_4J12_delayed"},
	{HHBBTT::DTT_L1Topo_delayed, "DTT_L1Topo_delayed"},
	{HHBBTT::DBT, "DBT"},
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
    {HHBBTT::pass_trigger_SR, "pass_trigger_SR"},
    {HHBBTT::pass_trigger_SLT, "pass_trigger_SLT"},
    {HHBBTT::pass_trigger_LTT, "pass_trigger_LTT"},
    {HHBBTT::pass_trigger_STT, "pass_trigger_STT"},
    {HHBBTT::pass_trigger_DTT, "pass_trigger_DTT"},
    {HHBBTT::pass_trigger_DTT_2016, "pass_trigger_DTT_2016"},
    {HHBBTT::pass_trigger_DTT_4J12, "pass_trigger_DTT_4J12"},
    {HHBBTT::pass_trigger_DTT_L1Topo, "pass_trigger_DTT_L1Topo"},
    {HHBBTT::pass_trigger_DTT_4J12_delayed, "pass_trigger_DTT_4J12_delayed"},
    {HHBBTT::pass_trigger_DTT_L1Topo_delayed, "pass_trigger_DTT_L1Topo_delayed"},
    {HHBBTT::pass_trigger_DBT, "pass_trigger_DBT"},
    {HHBBTT::TWO_JETS, "TWO_JETS"},
    {HHBBTT::TWO_BJETS, "TWO_BJETS"},
    {HHBBTT::ONE_BJET, "ONE_BJET"},
    {HHBBTT::N_LEPTONS_CUT_LEPHAD, "N_LEPTONS_CUT_LEPHAD"},
    {HHBBTT::ONE_TAU, "ONE_TAU"},
    {HHBBTT::OS_CHARGE_LEPHAD, "OS_CHARGE_LEPHAD"},
    {HHBBTT::OS_CHARGE_LEPTONS, "OS_CHARGE_LEPTONS"},
    {HHBBTT::pass_baseline_SLT, "pass_baseline_SLT"},
    {HHBBTT::pass_baseline_LTT, "pass_baseline_LTT"},
    {HHBBTT::pass_SLT_2B, "pass_SLT_2B"},
    {HHBBTT::pass_LTT_2B, "pass_LTT_2B"},
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
    {HHBBTT::pass_baseline_DBT, "pass_baseline_DBT"},
    {HHBBTT::pass_baseline_SR, "pass_baseline_SR"},
    {HHBBTT::pass_STT_2B, "pass_STT_2B"},
    {HHBBTT::pass_DTT_2016_2B, "pass_DTT_2016_2B"},
    {HHBBTT::pass_DTT_4J12_2B, "pass_DTT_4J12_2B"},
    {HHBBTT::pass_DTT_L1Topo_2B, "pass_DTT_L1Topo_2B"},
    {HHBBTT::pass_DTT_4J12_delayed_2B, "pass_DTT_4J12_delayed_2B"},
    {HHBBTT::pass_DTT_L1Topo_delayed_2B, "pass_DTT_L1Topo_delayed_2B"},
    {HHBBTT::pass_DTT_2B, "pass_DTT_2B"},
    {HHBBTT::pass_DBT_2B, "pass_DBT_2B"},
    {HHBBTT::pass_SR_2B, "pass_SR_2B"},
    {HHBBTT::pass_STT_1B, "pass_STT_1B"},
    {HHBBTT::pass_DTT_2016_1B, "pass_DTT_2016_1B"},
    {HHBBTT::pass_DTT_4J12_1B, "pass_DTT_4J12_1B"},
    {HHBBTT::pass_DTT_L1Topo_1B, "pass_DTT_L1Topo_1B"},
    {HHBBTT::pass_DTT_4J12_delayed_1B, "pass_DTT_4J12_delayed_1B"},
    {HHBBTT::pass_DTT_L1Topo_delayed_1B, "pass_DTT_L1Topo_delayed_1B"},
    {HHBBTT::pass_DTT_1B, "pass_DTT_1B"},
    {HHBBTT::pass_DBT_1B, "pass_DBT_1B"},
    {HHBBTT::pass_SR_1B, "pass_SR_1B"},
    {HHBBTT::pass_baseline_LepHad, "pass_baseline_LepHad"},
    {HHBBTT::pass_baseline_HadHad, "pass_baseline_HadHad"},
    {HHBBTT::pass_LepHad_2B, "pass_LepHad_2B"},
    {HHBBTT::pass_HadHad_2B, "pass_HadHad_2B"},
    {HHBBTT::pass_LepHad_1B, "pass_LepHad_1B"},
    {HHBBTT::pass_HadHad_1B, "pass_HadHad_1B"},
    {HHBBTT::pass_LepHad, "pass_LepHad"},
    {HHBBTT::pass_HadHad, "pass_HadHad"},
    {HHBBTT::pass_ZCR, "pass_ZCR"},
    {HHBBTT::pass_TopEMuCR, "pass_TopEMuCR"},
    };

    /// \brief Cutflow Variables
    CutManager m_bbttCuts;
    Gaudi::Property<std::vector<std::string>> m_inputCutList{this, "cutList", {}};
    std::vector<HHBBTT::Booleans> m_inputCutKeys;
    Gaudi::Property<bool> m_saveCutFlow{this, "saveCutFlow", false};
    long long int m_total_events{0};
    double m_total_mcEventWeight{0.0};
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
    void applyDiBJetTriggerSelection
      (const xAOD::EventInfo* event, const trigPassReadDecoMap& triggerdecos,
       const xAOD::Jet* jet0, const xAOD::Jet* jet1);

    void setThresholds(const xAOD::EventInfo* event,
		       const CP::SystematicSet& sys);

  };
}

#endif
