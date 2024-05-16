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

#include <EasyjetHub/CutManager.h>


namespace VBSHIGGS{
  enum Booleans{
    PASS_TRIGGER,
    EXACTLY_TWO_LEPTONS,
    TWO_SS_CHARGE_LEPTONS,
    PASS_MET,
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
        "EXACTLY_TWO_LEPTONS",    
        "TWO_SS_CHARGE_LEPTONS",
        "PASS_MET",
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

      void leptonSelection(const xAOD::ElectronContainer& electrons,const xAOD::MuonContainer& muons, const xAOD::MissingET *met);
      void bjetSelection(std::vector<const xAOD::Jet*> bjets);
      void vbsjetsSelection(std::vector<const xAOD::Jet*> nonbjets);
      
      Gaudi::Property<bool> m_bypass{ this, "bypass", false, "Run selector algorithm in pass-through mode" };

      /// \brief Setup syst-aware input container handles
      CutManager m_vbshiggsCuts;

      CP::SysListHandle m_systematicsList {this};

      CP::SysReadHandle<xAOD::JetContainer> m_jetHandle{ this, "jets", "",   "Jet container to read" };

      CP::SysReadDecorHandle<char>  m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};

      CP::SysReadHandle<xAOD::EventInfo> m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

      CP::SysReadHandle<xAOD::ElectronContainer> m_electronHandle{ this, "electrons", "",   "Electron container to read" };

      CP::SysReadHandle<xAOD::MuonContainer> m_muonHandle{ this, "muons", "",   "Muon container to read" };

      CP::SysReadHandle<xAOD::MissingETContainer> m_metHandle{ this, "met", "AnalysisMET",   "MET container to read" };

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
        {VBSHIGGS::EXACTLY_TWO_LEPTONS, "EXACTLY_TWO_LEPTONS"},
        {VBSHIGGS::TWO_SS_CHARGE_LEPTONS, "TWO_SS_CHARGE_LEPTONS"},
        {VBSHIGGS::PASS_MET, "PASS_MET"},
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

  };
}
#endif