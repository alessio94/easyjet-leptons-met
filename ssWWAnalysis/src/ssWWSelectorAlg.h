/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!

#ifndef SSWWANALYSIS_SSWWSELECTORALG
#define SSWWANALYSIS_SSWWSELECTORALG

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
#include <xAODMissingET/MissingETContainer.h>

#include "TriggerMatchingTool/IMatchingTool.h"
#include <EasyjetHub/CutManager.h>

namespace ssWWVBS
{
  enum Channel
  {
    SR,
    WZCR,
    misIDCR,
    incVR,
    LowDyVR,
    LowMjjVR,
    LowNjVR,
    tFakeVR,
    tEWKVR,
    lllVR,
    ZeeVR,
    DijetsCR,
  };

  enum TriggerChannel
  {
    SLT,
    prescaleSLT
  };

  enum Var {
    ele = 0,
    mu = 1,
    leadingele = 2,
    leadingmu = 3,
    subleadingele = 4,
    subleadingmu = 5,
  };

  enum Booleans
  {
    IS_ee,
    IS_mm,
    IS_em,
    IS_me,
    
    pass_trigger_SLT,
    pass_trigger_prescaleSLT,

    PASS_TRIGGER,
    PASS_TWO_LEPTONS,
    PASS_LEPTON_ID,
    EXACTLY_TWO_LEPTONS,
    TWO_SAME_CHARGE_LEPTONS,
    DILEPTON_MASS_THRESHOLD,
    DILEPTON_MASS_SIDEBAND_EE,
    MET,
    AT_LEAST_TWO_JETS,
    DIJETS_MASS_LOW,
    DIJETS_DELTA_RAPIDITY,
    BJET_VETO,
    DIJETS_MASS_HIGH,
    pass_SR,

    PASS_THREE_LEPTONS,
    EXACTLY_THREE_LEPTONS,
    pass_WZCR,
    pass_misIDCR,
    pass_incVR,
    pass_LowDyVR,
    pass_LowMjjVR,
    pass_LowNjVR,
    pass_tFakeVR,
    pass_tEWKVR,
    pass_lllVR,
    pass_ZeeVR,
    pass_DijetsCR,

    EXACTLY_ONE_LEPTON,

  };

  /// \brief An algorithm for counting containers
  class ssWWSelectorAlg final : public EL::AnaAlgorithm {

    public:
      ssWWSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

      /// \brief Initialisation method, for setting up tools and other persistent
      /// configs
      StatusCode initialize() override;
      /// \brief Execute method, for actions to be taken in the event loop
      StatusCode execute() override;
      /// \brief This is the mirror of initialize() and is called after all events are processed.
      StatusCode finalize() override; ///I added this to write the cutflow histogram.

    private :
      float Z_mass = 91.;
      double mjj = -99;
      float delta_yjj = 0;

      const std::vector<std::string> m_STANDARD_CUTS{
        "PASS_TRIGGER",
        "PASS_TWO_LEPTONS",
        "PASS_LEPTON_ID",
        "EXACTLY_TWO_LEPTONS",
        "TWO_SAME_CHARGE_LEPTONS",
        "DILEPTON_MASS_THRESHOLD",
        "DILEPTON_MASS_SIDEBAND_EE",
        "MET",
        "AT_LEAST_TWO_JETS",
        "DIJETS_MASS_LOW",
        "DIJETS_DELTA_RAPIDITY",
        "BJET_VETO",
        "DIJETS_MASS_HIGH",
      };

      Gaudi::Property<std::vector<std::string>> m_channel_names
      { this, "channels", {}, "Which channel to run" };

      std::vector<ssWWVBS::Channel> m_channels;

      Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

      Gaudi::Property<bool> m_bypass
      { this, "bypass", false, "Run selector algorithm in pass-through mode" };

      /// \brief Setup syst-aware input container handles
      CP::SysListHandle m_systematicsList {this};

      CP::SysReadHandle<xAOD::JetContainer>
      m_jetHandle{ this, "jets", "ssWWAnalysisJets_%SYS%", "Jet container to read" };

      CP::SysReadDecorHandle<char> 
      m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};

      CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

      CP::SysReadHandle<xAOD::ElectronContainer>
      m_electronHandle{ this, "electrons", "ssWWAnalysisElectrons_%SYS%", "Electron container to read" };

      CP::SysReadHandle<xAOD::MuonContainer>
      m_muonHandle{ this, "muons", "ssWWAnalysisMuons_%SYS%", "Muon container to read" };

      CP::SysReadHandle<xAOD::MissingETContainer>
      m_metHandle{ this, "met", "AnalysisMET_%SYS%", "MET container to read" };

      Gaudi::Property<std::string> m_eleWPName
  {this, "eleWP", "","Electron ID + Iso working point" };
      CP::SysReadDecorHandle<char> m_eleWPDecorHandle{"", this};

      Gaudi::Property<std::string> m_muonWPName
  {this, "muonWP", "","Muon ID + Iso cuts" };
      CP::SysReadDecorHandle<char> m_muonWPDecorHandle{"", this};

      CP::SysReadDecorHandle<unsigned int> m_year
	{this, "year", "dataTakingYear", ""};

      CP::SysReadDecorHandle<bool> m_is17_periodB5_B8
	{this, "is2017_periodB5_B8", "is2017_periodB5_B8", ""};
      CP::SysReadDecorHandle<bool> m_is22_75bunches
	{this, "is2022_75bunches", "is2022_75bunches", ""};
      CP::SysReadDecorHandle<bool> m_is23_75bunches
	{this, "is2023_75bunches", "is2023_75bunches", ""};
      CP::SysReadDecorHandle<bool> m_is23_400bunches
	{this, "is2023_400bunches", "is2023_400bunches", ""};
      
      CP::SysFilterReporterParams m_filterParams {this, "ssWW selection"};

      std::unordered_map<std::string,  SG::ReadDecorHandleKey<xAOD::EventInfo>> m_triggerDecorKeys;

      std::unordered_map<ssWWVBS::TriggerChannel, std::string> m_triggerChannels = 
      {
        {ssWWVBS::SLT, "SLT"},
        {ssWWVBS::prescaleSLT, "prescaleSLT"}
      };

      Gaudi::Property<std::vector<std::string>> m_triggers 
      { this, "triggerLists", {}, "Name list of trigger" };

      std::unordered_map<std::string, CP::SysReadDecorHandle<bool> > m_triggerdecos;

      ToolHandle<Trig::IMatchingTool> m_matchingTool
      { this, "trigMatchingTool", "", "Trigger matching tool"};

      long long int m_total_events{0};

      std::unordered_map<ssWWVBS::Booleans, CP::SysWriteDecorHandle<bool> > m_Bbranches;
      std::unordered_map<ssWWVBS::Booleans, bool> m_bools;
      std::unordered_map<ssWWVBS::Booleans, std::string> m_boolnames{
        {ssWWVBS::IS_ee, "IS_ee"},
        {ssWWVBS::IS_mm, "IS_mm"},
        {ssWWVBS::IS_em, "IS_em"},
        {ssWWVBS::IS_me, "IS_me"},
        {ssWWVBS::pass_trigger_SLT, "pass_trigger_SLT"},
        {ssWWVBS::pass_trigger_prescaleSLT, "pass_trigger_prescaleSLT"},
        {ssWWVBS::PASS_TRIGGER, "PASS_TRIGGER"},
        {ssWWVBS::PASS_TWO_LEPTONS, "PASS_TWO_LEPTONS"},
        {ssWWVBS::PASS_LEPTON_ID, "PASS_LEPTON_ID"},
        {ssWWVBS::EXACTLY_TWO_LEPTONS, "EXACTLY_TWO_LEPTONS"},
        {ssWWVBS::TWO_SAME_CHARGE_LEPTONS, "TWO_SAME_CHARGE_LEPTONS"},
        {ssWWVBS::DILEPTON_MASS_THRESHOLD, "DILEPTON_MASS_THRESHOLD"},
        {ssWWVBS::DILEPTON_MASS_SIDEBAND_EE, "DILEPTON_MASS_SIDEBAND_EE"},
        {ssWWVBS::MET, "MET"},
        {ssWWVBS::AT_LEAST_TWO_JETS, "AT_LEAST_TWO_JETS"},
        {ssWWVBS::DIJETS_MASS_LOW, "DIJETS_MASS_LOW"},
        {ssWWVBS::DIJETS_DELTA_RAPIDITY, "DIJETS_DELTA_RAPIDITY"},
        {ssWWVBS::BJET_VETO, "BJET_VETO"},
        {ssWWVBS::DIJETS_MASS_HIGH, "DIJETS_MASS_HIGH"},
        {ssWWVBS::PASS_THREE_LEPTONS, "PASS_THREE_LEPTONS"},
        {ssWWVBS::EXACTLY_THREE_LEPTONS, "EXACTLY_THREE_LEPTONS"},
        {ssWWVBS::pass_WZCR, "pass_WZCR"},
        {ssWWVBS::pass_SR, "pass_SR"},
        {ssWWVBS::pass_misIDCR, "pass_misIDCR"},
        {ssWWVBS::pass_incVR, "pass_incVR"},
        {ssWWVBS::pass_LowDyVR, "pass_LowDyVR"},
        {ssWWVBS::pass_LowMjjVR, "pass_LowMjjVR"},
        {ssWWVBS::pass_LowNjVR, "pass_LowNjVR"},
        {ssWWVBS::pass_tFakeVR, "pass_tFakeVR"},
        {ssWWVBS::pass_tEWKVR, "pass_tEWKVR"},
        {ssWWVBS::pass_lllVR, "pass_lllVR"},
        {ssWWVBS::pass_ZeeVR, "pass_ZeeVR"},
        {ssWWVBS::pass_DijetsCR, "pass_DijetsCR"},
        {ssWWVBS::EXACTLY_ONE_LEPTON, "EXACTLY_ONE_LEPTON"},
      };

      CutManager m_ssWWCuts;
      Gaudi::Property<std::vector<std::string>> m_inputCutList{this, "cutList", {}};
      std::vector<ssWWVBS::Booleans> m_inputCutKeys;
      Gaudi::Property<bool> m_saveCutFlow{this, "saveCutFlow", false};
      CP::SysWriteDecorHandle<bool> m_passallcuts {"PassAllCuts_%SYS%", this};

      CP::SysWriteDecorHandle<bool> m_ele_selected {"ele_is_selected_%SYS%", this};
      CP::SysWriteDecorHandle<bool> m_mu_selected {"mu_is_selected_%SYS%", this};
      

      std::unordered_map<ssWWVBS::TriggerChannel, std::unordered_map<ssWWVBS::Var, float>> m_pt_threshold;

      void evaluateTriggerCuts
	(const xAOD::EventInfo* event,
	 const xAOD::Electron* ele0, const xAOD::Electron* ele1,
	 const xAOD::Muon* mu0, const xAOD::Muon* mu1,
	 CutManager& ssWWCuts, const CP::SystematicSet& sys);
      
      void evaluateSingleLeptonTrigger
	(const xAOD::EventInfo* event, 
	 const xAOD::Electron* ele, const xAOD::Muon* mu,
	 const CP::SystematicSet& sys);
      void evaulateLeptonIDCuts
  (const xAOD::Electron*& ele0, const xAOD::Electron*& ele1,
   const xAOD::Muon*& mu0, const xAOD::Muon*& mu1,
   CutManager& ssWWCuts, const CP::SystematicSet& sys);
      void evaluateLeptonCuts(const xAOD::ElectronContainer& electrons,const xAOD::MuonContainer& muons,
                          const xAOD::Electron* ele0, const xAOD::Electron* ele1,
                          const xAOD::Muon* mu0, const xAOD::Muon* mu1,
                          CutManager& ssWWCuts);
      void evaluateMetCuts(const xAOD::MissingET* met, CutManager& ssWWCuts);
      void evaluateJetCuts(const ConstDataVector<xAOD::JetContainer>& nonbjets, CutManager& ssWWCuts);
      void evaluateBJetLeptonCuts(const ConstDataVector<xAOD::JetContainer>& bjets,
                          const xAOD::ElectronContainer& electrons, const xAOD::MuonContainer& muons,
                          CutManager& ssWWCuts);
      void setThresholds(const xAOD::EventInfo* event,
			 const CP::SystematicSet& sys);
      void evaluatePrescaleTriggerCuts(const xAOD::EventInfo *event, const xAOD::ElectronContainer& electrons,
                         const xAOD::MuonContainer& muons, const CP::SystematicSet& sys);

      StatusCode initialiseCutflow();
  };

}

#endif // SSWWANALYSIS_SSWWSELECTORALG

