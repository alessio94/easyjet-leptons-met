/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VBSHIGGSANALYSIS_SEMILEPSELECTORALG_H
#define VBSHIGGSANALYSIS_SEMILEPSELECTORALG_H

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

#include <EasyjetHub/CutManager.h>


namespace VBSHIGGS{

  enum Booleans_semiLep{
    TwoVBSJets,
    TwoBJets,
    LepSelection,
    JetSelection,
    METSelection
  };

  /// \brief An algorithm for counting containers
  class SemiLepSelectorAlg final : public EL::AnaAlgorithm{
    public:
      SemiLepSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

      /// \brief Initialisation method, for setting up tools and other persistent
      /// configs
      StatusCode initialize() override;
      /// \brief Execute method, for actions to be taken in the event loop
      StatusCode execute() override;
      /// \brief This is the mirror of initialize() and is called after all events are processed.
      StatusCode finalize() override; ///I added this to write the cutflow histogram.

      const std::vector<std::string> m_STANDARD_CUTS{
        "TwoVBSJets",
        "TwoBJets",
        "LepSelection",
        "JetSelection",
        "METSelection",
      };

      void leptonSelection(const xAOD::ElectronContainer& electrons,const xAOD::MuonContainer& muons, const xAOD::MissingET *met);
      void vbsjetsSelection(const xAOD::JetContainer * vbsjets);
      void bjetSelection(std::vector<const xAOD::Jet*> bjets);
      void jetSelection(const xAOD::JetContainer * signaljets, const xAOD::JetContainer * vbsjets);

      // \brief Setup syst-aware input container handles
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
      std::vector<VBSHIGGS::Booleans_semiLep> m_inputCutKeys;
      Gaudi::Property<bool> m_saveCutFlow{this, "saveCutFlow", false};
      Gaudi::Property<bool> m_bypass{ this, "bypass", false, "Run selector algorithm in pass-through mode" };

       long long int m_total_events{0};

      std::unordered_map<VBSHIGGS::Booleans_semiLep, CP::SysWriteDecorHandle<bool> > m_Bbranches;
      std::unordered_map<VBSHIGGS::Booleans_semiLep, bool> m_bools;
       CP::SysWriteDecorHandle<bool> m_passallcuts {"PassAllCuts_%SYS%", this};
      std::unordered_map<VBSHIGGS::Booleans_semiLep, std::string> m_boolnames{
        {VBSHIGGS::TwoVBSJets, "TwoVBSJets"},
        {VBSHIGGS::TwoBJets, "TwoBJets"},
        {VBSHIGGS::LepSelection, "LepSelection"},
        {VBSHIGGS::JetSelection, "JetSelection"},
        {VBSHIGGS::METSelection, "METSelection"},
      };
  };
}
#endif
