/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VBSHIGGSANALYSIS_FULLLEPSELECTORALG_H
#define VBSHIGGSANALYSIS_FULLLEPSELECTORALG_H

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

#include "TriggerMatchingTool/IMatchingTool.h"
#include <EasyjetHub/CutManager.h>


namespace VBSHIGGS{
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

  enum Booleans{
    PASS_TRIGGER,
    pass_trigger_SLT,
    pass_trigger_DLT,
    pass_trigger_ASLT1_em,
    pass_trigger_ASLT1_me,
    pass_trigger_ASLT2,
    AT_LEAST_TWO_LEPTONS,
    EXACTLY_TWO_LEPTONS,
    TWO_SS_CHARGE_LEPTONS,
    TWO_OS_CHARGE_LEPTONS,
    PASS_MET,
    AT_LEAST_ONE_B_JET,
    EXACTLY_TWO_B_JETS,
    PASS_DELTA_R_BB,
    PASS_mBB,
    PASS_VBS_BASELINE,
    IS_SF,
    IS_ee,
    IS_mm,
    IS_em,
    Pass_ll,
  };
  /// \brief An algorithm for counting containers
  class FullLepSelectorAlg final : public EL::AnaAlgorithm{
    public:
      FullLepSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

      /// \brief Initialisation method, for setting up tools and other persistent
      /// configs
      StatusCode initialize() override;
      /// \brief Execute method, for actions to be taken in the event loop
      StatusCode execute() override;
      /// \brief This is the mirror of initialize() and is called after all events are processed.
      StatusCode finalize() override; ///I added this to write the cutflow histogram.
      const std::vector<std::string> m_STANDARD_CUTS{
        "PASS_TRIGGER",
        "AT_LEAST_TWO_LEPTONS",
        "EXACTLY_TWO_LEPTONS",    
        "TWO_SS_CHARGE_LEPTONS",
        "TWO_OS_CHARGE_LEPTONS",
        "PASS_MET",
        "AT_LEAST_ONE_B_JET",
        "EXACTLY_TWO_B_JETS",
        "PASS_DELTA_R_BB",
        "PASS_mBB",
        "PASS_VBS_BASELINE"
      };
      std::vector<std::string> m_Bvarnames{
        "Pass_ll", 
        "IS_SF",
        "IS_ee",
        "IS_mm",
        "IS_em",
      };

      std::unordered_map<VBSHIGGS::TriggerChannel, std::unordered_map<VBSHIGGS::Var, float>> m_pt_threshold;

      void evaluateTriggerCuts(const xAOD::EventInfo *event,
                              const xAOD::ElectronContainer *electrons , const xAOD::MuonContainer *muons,
                              const CP::SystematicSet& sys);

      void evaluateSingleLeptonTrigger(const xAOD::EventInfo* event, const xAOD::Electron* ele, const xAOD::Muon* mu, const CP::SystematicSet& sys);
      void evaluateDiLeptonTrigger(const xAOD::EventInfo* event, const xAOD::Electron* ele0, const xAOD::Electron* ele1, const xAOD::Muon* mu0, const xAOD::Muon* mu1, const CP::SystematicSet& sys);
      void evaluateAsymmetricLeptonTrigger(const xAOD::EventInfo* event, const xAOD::Electron* ele, const xAOD::Muon* mu, const CP::SystematicSet& sys);
      void setThresholds(const xAOD::EventInfo* event, const CP::SystematicSet& sys);

      void leptonSelection(const xAOD::ElectronContainer* electrons,const xAOD::MuonContainer* muons, const xAOD::MissingET *met);
      void bjetSelection(std::vector<const xAOD::Jet*> bjets);
      void vbsjetsSelection(const xAOD::JetContainer * vbsjets);
      
      Gaudi::Property<bool> m_bypass{ this, "bypass", false, "Run selector algorithm in pass-through mode" };

      /// \brief Setup syst-aware input container handles
      CutManager m_vbshiggsCuts;

      CP::SysListHandle m_systematicsList {this};

      CP::SysReadHandle<xAOD::JetContainer> m_signaljetHandle{ this, "signaljets", "vbshiggsAnalysisSignalJets_%SYS%", "Signal Jet container to read" };

      CP::SysReadHandle<xAOD::JetContainer> m_vbsjetHandle{ this, "vbsjets", "vbshiggsAnalysisVBSJets_%SYS%", "VBS Jet container to read" };

      CP::SysReadDecorHandle<char>  m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};

      CP::SysReadHandle<xAOD::EventInfo> m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

      CP::SysReadHandle<xAOD::ElectronContainer> m_electronHandle{ this, "electrons", "vbshiggsAnalysisElectrons_%SYS%", "Electron container to read" };

      CP::SysReadHandle<xAOD::MuonContainer> m_muonHandle{ this, "muons", "vbshiggsAnalysisMuons_%SYS%", "Muon container to read" };

      CP::SysReadHandle<xAOD::MissingETContainer> m_metHandle{ this, "met", "AnalysisMET_%SYS%", "MET container to read" };

      CP::SysFilterReporterParams m_filterParams {this, "vbshiggs selection"};

      Gaudi::Property<std::vector<std::string>> m_inputCutList{this, "cutList", {}};
      std::vector<VBSHIGGS::Booleans> m_inputCutKeys;
      Gaudi::Property<bool> m_saveCutFlow{this, "saveCutFlow", false};

      long long int m_total_events{0};

      std::unordered_map<VBSHIGGS::Booleans, CP::SysWriteDecorHandle<bool> > m_Bbranches;
      std::unordered_map<VBSHIGGS::Booleans, bool> m_bools;
      CP::SysWriteDecorHandle<bool> m_passallcuts {"PassAllCuts_%SYS%", this};
      std::unordered_map<VBSHIGGS::Booleans, std::string> m_boolnames{
        {VBSHIGGS::PASS_TRIGGER, "PASS_TRIGGER"},
        {VBSHIGGS::pass_trigger_SLT, "pass_trigger_SLT"},
        {VBSHIGGS::pass_trigger_DLT, "pass_trigger_DLT"},
        {VBSHIGGS::pass_trigger_ASLT1_em, "pass_trigger_ASLT1_em"},
        {VBSHIGGS::pass_trigger_ASLT1_me, "pass_trigger_ASLT1_me"},
        {VBSHIGGS::pass_trigger_ASLT2, "pass_trigger_ASLT2"},
        {VBSHIGGS::AT_LEAST_TWO_LEPTONS, "AT_LEAST_TWO_LEPTONS"},
        {VBSHIGGS::EXACTLY_TWO_LEPTONS, "EXACTLY_TWO_LEPTONS"},
        {VBSHIGGS::TWO_SS_CHARGE_LEPTONS, "TWO_SS_CHARGE_LEPTONS"},
        {VBSHIGGS::TWO_OS_CHARGE_LEPTONS, "TWO_OS_CHARGE_LEPTONS"},
        {VBSHIGGS::PASS_MET, "PASS_MET"},
        {VBSHIGGS::AT_LEAST_ONE_B_JET, "AT_LEAST_ONE_B_JET"},
        {VBSHIGGS::EXACTLY_TWO_B_JETS, "EXACTLY_TWO_B_JETS"},
        {VBSHIGGS::PASS_DELTA_R_BB, "PASS_DELTA_R_BB"},
        {VBSHIGGS::PASS_mBB, "PASS_mBB"},
        {VBSHIGGS::PASS_VBS_BASELINE, "PASS_VBS_BASELINE"},
        {VBSHIGGS::Pass_ll, "Pass_ll"},
        {VBSHIGGS::IS_SF, "IS_SF"},
        {VBSHIGGS::IS_ee, "IS_ee"},
        {VBSHIGGS::IS_mm, "IS_mm"},
        {VBSHIGGS::IS_em, "IS_em"},
      };

      CP::SysReadDecorHandle<bool> m_is17_periodB5_B8
      {this, "is2017_periodB5_B8", "is2017_periodB5_B8", ""};
      CP::SysReadDecorHandle<bool> m_is22_75bunches
      {this, "is2022_75bunches", "is2022_75bunches", ""};
      CP::SysReadDecorHandle<bool> m_is23_75bunches
      {this, "is2023_75bunches", "is2023_75bunches", ""};
      CP::SysReadDecorHandle<bool> m_is23_400bunches
      {this, "is2023_400bunches", "is2023_400bunches", ""};
      CP::SysReadDecorHandle<unsigned int> m_year {this, "year", "dataTakingYear", ""};

      Gaudi::Property<std::vector<std::string>> m_triggers 
      { this, "triggerLists", {}, "Name list of trigger" };

      std::unordered_map<std::string, CP::SysReadDecorHandle<bool> > m_triggerdecos;

      ToolHandle<Trig::IMatchingTool> m_matchingTool
      { this, "trigMatchingTool", "", "Trigger matching tool"};

      std::unordered_map<VBSHIGGS::TriggerChannel, std::string> m_triggerChannels = 
      {
        {VBSHIGGS::SLT, "SLT"},
        {VBSHIGGS::DLT, "DLT"},
        {VBSHIGGS::ASLT1_em, "ASLT1_em"},
        {VBSHIGGS::ASLT1_me, "ASLT1_me"},
        {VBSHIGGS::ASLT2, "ASLT2"},
      };

  };
}
#endif
