/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!

#ifndef ZCHARMANALYSIS_ZCHARMSELECTORALG
#define ZCHARMANALYSIS_ZCHARMSELECTORALG

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
#include <xAODMissingET/MissingETContainer.h>
#include <xAODTruth/TruthEvent.h>
#include <xAODTruth/TruthEventContainer.h>

#include "TriggerMatchingTool/IMatchingTool.h"
#include <EasyjetHub/CutManager.h>
#include "ZCharmEnums.h"

namespace ZCC
{

  /// \brief An algorithm for counting containers
  class ZCharmSelectorAlg final : public EL::AnaAlgorithm {

    public:
      ZCharmSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

      /// \brief Initialisation method, for setting up tools and other persistent
      /// configs
      StatusCode initialize() override;
      /// \brief Execute method, for actions to be taken in the event loop
      StatusCode execute() override;
      /// \brief This is the mirror of initialize() and is called after all events are processed.
      StatusCode finalize() override; ///I added this to write the cutflow histogram.

    private :

      const std::vector<std::string> m_BASELINE_CUTS{
        "PASS_TRIGGER",
        "EXACTLY_TWO_LEPTONS",
        "OPPOSITE_CHARGE_LEPTONS",
        "DILEPTON_MASS_WINDOW",
        "MET",
      };

      Gaudi::Property<bool> m_bypass
      { this, "bypass", false, "Run selector algorithm in pass-through mode" };

      /// \brief Setup syst-aware input container handles
      CP::SysListHandle m_systematicsList {this};

      CP::SysReadHandle<xAOD::JetContainer>
      m_jetHandle{ this, "jets", "ZCharmAnalysisJets_%SYS%", "Jet container to read" };

      CP::SysReadHandle<xAOD::JetContainer> 
      m_largejetHandle{ this, "largejets", "ZCharmAnalysisLargeJets_%SYS%", "Large R Jet container to read"};

      CP::SysReadDecorHandle<char> 
      m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};
 
      Gaudi::Property<std::vector<std::string>> m_PCBTnames
        {this, "PCBTDecorList", {}, "Name list of pseudo-continuous b-tagging decorator"};
      std::unordered_map<std::string, CP::SysReadDecorHandle<int>> m_PCBTs;

      CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

      CP::SysReadHandle<xAOD::ElectronContainer>
      m_electronHandle{ this, "electrons", "ZCharmAnalysisElectrons_%SYS%", "Electron container to read" };

      CP::SysReadHandle<xAOD::MuonContainer>
      m_muonHandle{ this, "muons", "ZCharmAnalysisMuons_%SYS%", "Muon container to read" };

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
      
      CP::SysFilterReporterParams m_filterParams {this, "ZCharm selection"};

      std::unordered_map<std::string,  SG::ReadDecorHandleKey<xAOD::EventInfo>> m_triggerDecorKeys;

      std::unordered_map<ZCC::TriggerChannel, std::string> m_triggerChannels = 
      {
        {ZCC::SLT, "SLT"},
        {ZCC::DLT, "DLT"},
      };

      SG::ReadDecorHandleKey<xAOD::TruthEventContainer> m_passTruthCutsKey {this, "PassTruthCuts", "TruthEvents.PassTruthCuts", "Name of the truth cuts decorator"};
      
      Gaudi::Property<std::vector<std::string>> m_triggers 
      { this, "triggerLists", {}, "Name list of trigger" };

      std::unordered_map<std::string, CP::SysReadDecorHandle<bool> > m_triggerdecos;

      ToolHandle<Trig::IMatchingTool> m_matchingTool
      { this, "trigMatchingTool", "", "Trigger matching tool"};

      long long int m_total_events{0};

      std::unordered_map<ZCC::Booleans, CP::SysWriteDecorHandle<bool> > m_Bbranches;
      std::unordered_map<ZCC::Booleans, bool> m_bools;
      std::unordered_map<ZCC::Booleans, std::string> m_boolnames{
        {ZCC::IS_ee, "IS_ee"},
        {ZCC::IS_mm, "IS_mm"},
        {ZCC::IS_em, "IS_em"},

        {ZCC::pass_trigger_SLT, "pass_trigger_SLT"},
        {ZCC::pass_trigger_DLT, "pass_trigger_DLT"},

        {ZCC::PASS_TRIGGER, "PASS_TRIGGER"},
        {ZCC::EXACTLY_TWO_LEPTONS, "EXACTLY_TWO_LEPTONS"},
        {ZCC::OPPOSITE_CHARGE_LEPTONS, "OPPOSITE_CHARGE_LEPTONS"},
        {ZCC::DILEPTON_MASS_WINDOW, "DILEPTON_MASS_WINDOW"},

        {ZCC::MET, "MET"},
        {ZCC::ONE_B_JETS, "ONE_B_JETS"},
        {ZCC::TWO_B_JETS, "TWO_B_JETS"},
        {ZCC::ONE_C_JETS, "ONE_C_JETS"},
        {ZCC::TWO_C_JETS, "TWO_C_JETS"},
        {ZCC::ONE_LARGE_JET, "ONE_LARGE_JET"},
      };

      CutManager m_ZCharmCuts;
      Gaudi::Property<std::vector<std::string>> m_inputCutList{this, "cutList", {}};
      std::vector<ZCC::Booleans> m_inputCutKeys;
      Gaudi::Property<bool> m_saveCutFlow{this, "saveCutFlow", false};
      CP::SysWriteDecorHandle<bool> m_pass_cuts {"PassCuts_%SYS%", this};

      std::unordered_map<ZCC::TriggerChannel, std::unordered_map<ZCC::Var, float>> m_pt_threshold;

      void evaluateTriggerCuts
	(const xAOD::EventInfo* event,
	 const xAOD::Electron* ele0, const xAOD::Electron* ele1,
	 const xAOD::Muon* mu0, const xAOD::Muon* mu1,
	 CutManager& ZCharmCuts, const CP::SystematicSet& sys);

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
                          const xAOD::MuonContainer& muons, const xAOD::MissingET* met, CutManager& ZCharmCuts);
      void evaluateBJetCuts(const ConstDataVector<xAOD::JetContainer>& bjets, CutManager& ZCharmCuts);
      void evaluateCJetCuts(const ConstDataVector<xAOD::JetContainer>& cjets, CutManager& ZCharmCuts);
      void evaluateLargeJetCuts(const xAOD::JetContainer *largeJets);
      void setThresholds(const xAOD::EventInfo* event, const CP::SystematicSet& sys);
      StatusCode initialiseCutflow();
  };

}

#endif // ZCharmANALYSIS_ZCharmSELECTORALG

