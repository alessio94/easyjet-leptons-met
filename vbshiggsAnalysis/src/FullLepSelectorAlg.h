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
#include "vbshiggsEnums.h"


namespace VBSHIGGS{

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
        "PASS_AT_LEAST_TWO_LEPTONS",
        "PASS_EXACTLY_TWO_LEPTONS",    
        "PASS_TWO_SS_CHARGE_LEPTONS",
        "PASS_TWO_OS_CHARGE_LEPTONS",
        "PASS_MET",
        "PASS_RES_AT_LEAST_ONE_B_JET",
        "PASS_RES_EXACTLY_ONE_B_JET",
        "PASS_RES_EXACTLY_TWO_B_JETS",
        "PASS_RES_NOADD_B_JET",
        "PASS_ONE_LARGE_JET",
        "PASS_TWO_SIGNAL_JETS",
        "PASS_DELTA_R_BB",
        "PASS_RES_H_WINDOW",
        "PASS_VBS_BASELINE",
        "PASS_RES_BASELINE",
        "PASS_MERG_BASELINE"
      };
      std::vector<std::string> m_Bvarnames{
        "Pass_ll", 
        "IS_SF",
        "IS_ee",
        "IS_mm",
        "IS_em",
      };
     
      void setThresholds(const xAOD::EventInfo* event, const CP::SystematicSet& sys);
      void leptonSelection(const xAOD::ElectronContainer* electrons,const xAOD::MuonContainer* muons, const xAOD::MissingET *met);
      void vbsjetsSelection(const xAOD::JetContainer * vbsjets);
      void resolvedSelection(const xAOD::JetContainer *HJets, std::vector<const xAOD::Jet*> bjets, const CP::SystematicSet& sys);
      void boostedSelection(const xAOD::JetContainer *largeJets, const CP::SystematicSet& sys);
      void eventCategorisation();
      
      Gaudi::Property<bool> m_bypass{ this, "bypass", false, "Run selector algorithm in pass-through mode" };

      /// \brief Setup syst-aware input container handles
      CutManager m_vbshiggsCuts;

      CP::SysListHandle m_systematicsList {this};

      CP::SysReadHandle<xAOD::JetContainer> m_largejetHandle{ this, "largejets", "vbshiggsAnalysisLargeJets_%SYS%", "Large R Jet container to read"};
      CP::SysReadDecorHandle<float> m_GN2Xv01_phbb = {this, "phbb", "GN2Xv01_phbb", "GN2Xv01_phbb"};
      CP::SysReadDecorHandle<float> m_GN2Xv01_phcc = {this, "phcc", "GN2Xv01_phcc", "GN2Xv01_phcc"};
      CP::SysReadDecorHandle<float> m_GN2Xv01_pqcd = {this, "pqcd", "GN2Xv01_pqcd", "GN2Xv01_pqcd"};
      CP::SysReadDecorHandle<float> m_GN2Xv01_ptop = {this, "ptop", "GN2Xv01_ptop", "GN2Xv01_ptop"};

      CP::SysReadHandle<xAOD::JetContainer> m_signaljetHandle{ this, "signaljets", "vbshiggsAnalysisSignalJets_%SYS%", "Signal Jet container to read" };

      CP::SysReadHandle<xAOD::JetContainer> m_HCandHandle{ this, "higgsCandidates", "vbshiggsAnalysisHJets_%SYS%", "Higgs Candidates container to read"};

      CP::SysReadHandle<xAOD::JetContainer> m_vbsjetHandle{ this, "vbsjets", "vbshiggsAnalysisVBSJets_%SYS%", "VBS Jet container to read" };

      CP::SysReadDecorHandle<char>  m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};

      CP::SysReadHandle<xAOD::EventInfo> m_eventHandle{ this, "event", "EventInfo",  "EventInfo container to read" };

      CP::SysReadHandle<xAOD::ElectronContainer> m_electronHandle{ this, "electrons", "vbshiggsAnalysisElectrons_%SYS%",  "Electron container to read" };

      CP::SysReadHandle<xAOD::MuonContainer> m_muonHandle{ this, "muons", "vbshiggsAnalysisMuons_%SYS%",   "Muon container to read" };

      CP::SysReadHandle<xAOD::MissingETContainer> m_metHandle{ this, "met", "AnalysisMET_%SYS%",   "MET container to read" };

      CP::SysReadDecorHandle<bool> m_passTriggerSLT {this, "passTriggerSLT", "pass_trigger_SLT_%SYS%", "events pass any singlep triggers"};
      CP::SysReadDecorHandle<bool> m_passTriggerDLT {this, "passTriggerDLT", "pass_trigger_DLT_%SYS%", "events pass any dilepton triggers"};
      CP::SysReadDecorHandle<bool> m_passTriggerASLT1_em {this, "passTriggerASLT1_em", "pass_trigger_ASLT1_em_%SYS%", "events pass ASLT1_em triggers"};
      CP::SysReadDecorHandle<bool> m_passTriggerASLT1_me {this, "passTriggerASLT1_me", "pass_trigger_ASLT1_me_%SYS%", "events pass ASLT1_me triggers"};
      CP::SysReadDecorHandle<bool> m_passTriggerASLT2 {this, "passTriggerASLT2", "pass_trigger_ASLT2_%SYS%", "events pass ASLT2 triggers"};

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
        {VBSHIGGS::PASS_AT_LEAST_TWO_LEPTONS, "PASS_AT_LEAST_TWO_LEPTONS"},
        {VBSHIGGS::PASS_EXACTLY_TWO_LEPTONS, "PASS_EXACTLY_TWO_LEPTONS"},
        {VBSHIGGS::PASS_TWO_SS_CHARGE_LEPTONS, "PASS_TWO_SS_CHARGE_LEPTONS"},
        {VBSHIGGS::PASS_TWO_OS_CHARGE_LEPTONS, "PASS_TWO_OS_CHARGE_LEPTONS"},
        {VBSHIGGS::PASS_MET, "PASS_MET"},
        {VBSHIGGS::PASS_RES_AT_LEAST_ONE_B_JET, "PASS_RES_AT_LEAST_ONE_B_JET"},
        {VBSHIGGS::PASS_RES_EXACTLY_ONE_B_JET, "PASS_RES_EXACTLY_ONE_B_JET"},
        {VBSHIGGS::PASS_RES_NOADD_B_JET, "PASS_RES_NOADD_B_JET"},
        {VBSHIGGS::PASS_RES_EXACTLY_TWO_B_JETS, "PASS_RES_EXACTLY_TWO_B_JETS"},
        {VBSHIGGS::PASS_ONE_LARGE_JET, "PASS_ONE_LARGE_JET"},
        {VBSHIGGS::PASS_TWO_SIGNAL_JETS, "PASS_TWO_SIGNAL_JETS"},
        {VBSHIGGS::PASS_DELTA_R_BB, "PASS_DELTA_R_BB"},
        {VBSHIGGS::PASS_RES_H_WINDOW, "PASS_RES_H_WINDOW"},
        {VBSHIGGS::PASS_VBS_BASELINE, "PASS_VBS_BASELINE"},
        {VBSHIGGS::PASS_RES_BASELINE, "PASS_RES_BASELINE"},
        {VBSHIGGS::PASS_MERG_BASELINE, "PASS_MERG_BASELINE"},
        {VBSHIGGS::Pass_ll, "Pass_ll"},
        {VBSHIGGS::IS_SF, "IS_SF"},
        {VBSHIGGS::IS_ee, "IS_ee"},
        {VBSHIGGS::IS_mm, "IS_mm"},
        {VBSHIGGS::IS_em, "IS_em"},
      };

      CP::SysReadDecorHandle<unsigned int> m_year {this, "year", "dataTakingYear", ""};

      Gaudi::Property<std::vector<std::string>> m_triggers 
      { this, "triggerLists", {}, "Name list of trigger" };
  };
}
#endif