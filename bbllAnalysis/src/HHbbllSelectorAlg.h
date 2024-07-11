/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!

#ifndef HHBBLLANALYSIS_HHBBLLSELECTORALG
#define HHBBLLANALYSIS_HHBBLLSELECTORALG

#include "AnaAlgorithm/AnaAlgorithm.h"

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

namespace HHBBLL
{
  enum TriggerChannel
  {
    SLT,
    DLT,
    ASLT1_em,
    ASLT1_me,
    ASLT2,
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
    Pass_ll,
    IS_SF,
    IS_ee,
    IS_mm,
    IS_em,
    pass_trigger_SLT,
    pass_trigger_DLT,
    pass_trigger_ASLT1_em,
    pass_trigger_ASLT1_me,
    pass_trigger_ASLT2,
    EXACTLY_TWO_LEPTONS,
    PASS_TRIGGER,
    TWO_OPPOSITE_CHARGE_LEPTONS,
    EXACTLY_TWO_B_JETS,
    DILEPTON_MASS_SR1,
    VBFVETO_SR1,
    DILEPTON_MASS_SR2,
    DIBJET_MASS_SR2,
    pass_ZHF_CR1,
    pass_ZHF_CR2,
  };

  /// \brief An algorithm for counting containers
  class HHbbllSelectorAlg final : public EL::AnaAlgorithm {

    public:
      HHbbllSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

      /// \brief Initialisation method, for setting up tools and other persistent
      /// configs
      StatusCode initialize() override;
      /// \brief Execute method, for actions to be taken in the event loop
      StatusCode execute() override;
      /// \brief This is the mirror of initialize() and is called after all events are processed.
      StatusCode finalize() override; ///I added this to write the cutflow histogram.

      const std::vector<std::string> m_STANDARD_CUTS{
        "EXACTLY_TWO_LEPTONS",    
        "PASS_TRIGGER",
        "TWO_OPPOSITE_CHARGE_LEPTONS",
        "EXACTLY_TWO_B_JETS",
        "DILEPTON_MASS_SR1",
        "VBFVETO_SR1",
        "DILEPTON_MASS_SR2",
	"DIBJET_MASS_SR2"
      };


    private :
      // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
      // "someInfo"};

      Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

      Gaudi::Property<bool> m_bypass
      { this, "bypass", false, "Run selector algorithm in pass-through mode" };

      /// \brief Setup syst-aware input container handles
      CP::SysListHandle m_systematicsList {this};

      CP::SysReadHandle<xAOD::JetContainer>
      m_jetHandle{ this, "jets", "bbllAnalysisJets_%SYS%", "Jet container to read" };

      CP::SysReadDecorHandle<char> 
      m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};

      CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

      CP::SysReadHandle<xAOD::ElectronContainer>
      m_electronHandle{ this, "electrons", "bbllAnalysisElectrons_%SYS%", "Electron container to read" };

      CP::SysReadHandle<xAOD::MuonContainer>
      m_muonHandle{ this, "muons", "bbllAnalysisMuons_%SYS%", "Muon container to read" };

      CP::SysReadHandle<xAOD::MissingETContainer>
      m_metHandle{ this, "met", "AnalysisMET_%SYS%", "MET container to read" };

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
      
      CP::SysFilterReporterParams m_filterParams {this, "HHbbll selection"};

      std::unordered_map<HHBBLL::TriggerChannel, std::string> m_triggerChannels = 
      {
        {HHBBLL::SLT, "SLT"},
        {HHBBLL::DLT, "DLT"},
        {HHBBLL::ASLT1_em, "ASLT1_em"},
        {HHBBLL::ASLT1_me, "ASLT1_me"},
        {HHBBLL::ASLT2, "ASLT2"},
      };

      Gaudi::Property<std::vector<std::string>> m_triggers 
      { this, "triggerLists", {}, "Name list of trigger" };

      std::unordered_map<std::string, CP::SysReadDecorHandle<bool> > m_triggerdecos;

      ToolHandle<Trig::IMatchingTool> m_matchingTool
      { this, "trigMatchingTool", "", "Trigger matching tool"};

      long long int m_total_events{0};

      std::unordered_map<HHBBLL::Booleans, CP::SysWriteDecorHandle<bool> > m_Bbranches;
      std::unordered_map<HHBBLL::Booleans, bool> m_bools;
      std::unordered_map<HHBBLL::Booleans, std::string> m_boolnames{
        {HHBBLL::Pass_ll, "Pass_ll"},
        {HHBBLL::IS_SF, "IS_SF"},
        {HHBBLL::IS_ee, "IS_ee"},
        {HHBBLL::IS_mm, "IS_mm"},
        {HHBBLL::IS_em, "IS_em"},
        {HHBBLL::pass_trigger_SLT, "pass_trigger_SLT"},
        {HHBBLL::pass_trigger_DLT, "pass_trigger_DLT"},
        {HHBBLL::pass_trigger_ASLT1_em, "pass_trigger_ASLT1_em"},
        {HHBBLL::pass_trigger_ASLT1_me, "pass_trigger_ASLT1_me"},
        {HHBBLL::pass_trigger_ASLT2, "pass_trigger_ASLT2"},
        {HHBBLL::EXACTLY_TWO_LEPTONS, "EXACTLY_TWO_LEPTONS"},
        {HHBBLL::PASS_TRIGGER, "PASS_TRIGGER"},
        {HHBBLL::TWO_OPPOSITE_CHARGE_LEPTONS, "TWO_OPPOSITE_CHARGE_LEPTONS"},
        {HHBBLL::EXACTLY_TWO_B_JETS, "EXACTLY_TWO_B_JETS"},
        {HHBBLL::DILEPTON_MASS_SR1, "DILEPTON_MASS_SR1"},
        {HHBBLL::VBFVETO_SR1, "VBFVETO_SR1"},
        {HHBBLL::DILEPTON_MASS_SR2, "DILEPTON_MASS_SR2"},
        {HHBBLL::DIBJET_MASS_SR2, "DIBJET_MASS_SR2"},
	{HHBBLL::pass_ZHF_CR1, "pass_ZHF_CR1"},
	{HHBBLL::pass_ZHF_CR2, "pass_ZHF_CR2"},
      };

      CutManager m_bbllCuts;
      Gaudi::Property<std::vector<std::string>> m_inputCutList{this, "cutList", {}};
      std::vector<HHBBLL::Booleans> m_inputCutKeys;
      Gaudi::Property<bool> m_saveCutFlow{this, "saveCutFlow", false};
      CP::SysWriteDecorHandle<bool> m_passallcuts {"PassAllCuts_%SYS%", this};

      std::unordered_map<HHBBLL::TriggerChannel, std::unordered_map<HHBBLL::Var, float>> m_pt_threshold;

      void evaluateTriggerCuts
	(const xAOD::EventInfo* event,
	 const xAOD::Electron* ele0, const xAOD::Electron* ele1,
	 const xAOD::Muon* mu0, const xAOD::Muon* mu1,
	 CutManager& bbllCuts, const CP::SystematicSet& sys);
      void evaluateSingleLeptonTrigger
	(const xAOD::EventInfo* event, 
	 const xAOD::Electron* ele, const xAOD::Muon* mu,
	 const CP::SystematicSet& sys);
      void evaluateDiLeptonTrigger
	(const xAOD::EventInfo* event,
	 const xAOD::Electron* ele0, const xAOD::Electron* ele1,
	 const xAOD::Muon* mu0, const xAOD::Muon* mu1,
	 const CP::SystematicSet& sys);
      void evaluateAsymmetricLeptonTrigger
	(const xAOD::EventInfo* event,
	 const xAOD::Electron* ele, const xAOD::Muon* mu,
	 const CP::SystematicSet& sys);

      void evaluateLeptonCuts(const xAOD::ElectronContainer& electrons,
                          const xAOD::MuonContainer& muons, CutManager& bbllCuts);
      void evaluateJetCuts(const ConstDataVector<xAOD::JetContainer>& bjets,
                          const ConstDataVector<xAOD::JetContainer>& nonbjets, CutManager& bbllCuts);
      void evaluateBJetLeptonCuts(const ConstDataVector<xAOD::JetContainer>& bjets, const xAOD::JetContainer& jets,
                          const xAOD::ElectronContainer& electrons, const xAOD::MuonContainer& muons);
      void setThresholds(const xAOD::EventInfo* event,
			 const CP::SystematicSet& sys);
  };

}

#endif // HHBBLLANALYSIS_HHBBLLSELECTORALG

