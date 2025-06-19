/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!

#ifndef SELECTIONFLAGSTTHHALG_H
#define SELECTIONFLAGSTTHHALG_H

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysFilterReporterParams.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODMissingET/MissingETContainer.h>
#include "TriggerMatchingTool/IMatchingTool.h"

#include <EasyjetHub/CutManager.h>

#include <algorithm>

namespace ttHH
{

  enum Trigger_Matching_Tool{
    pass_matching_trigger_singlep, 
  };

  /// \brief An algorithm for counting containers
  class ttHHSelectorAlg final : public AthHistogramAlgorithm {

    public:
      ttHHSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

      /// \brief Initialisation method, for setting up tools and other persistent
      /// configs
      StatusCode initialize() override;
      /// \brief Execute method, for actions to be taken in the event loop
      StatusCode execute() override;
      /// \brief This is the mirror of initialize() and is called after all events are processed.
      StatusCode finalize() override; ///I added this to write the cutflow histogram.

    private:
      const std::vector<std::string> m_STANDARD_CUTS{
          "PASS_TRIGGER",
          "PASS_BASELINE", 
          "PASS_TRIGGER_MATCHING"
      };

      void evaluateCuts(const xAOD::JetContainer& jets,
		        const xAOD::JetContainer& bjets,
			const xAOD::MuonContainer& muons,
			const xAOD::ElectronContainer& electrons,
			CutManager& ttHHCuts);
      
      void evaluateTriggerMatchingCuts(const std::vector<std::string> &m_leptonTriggers, 
                                      const xAOD::MuonContainer* muons,  const xAOD::ElectronContainer* electrons,
                                      CutManager& ttHHCuts);

      StatusCode initialiseCutflow();

      /// \brief Setup syst-aware input container handles
      CutManager m_ttHHCuts;
      CP::SysListHandle m_systematicsList {this};

      CP::SysReadHandle<xAOD::JetContainer>
      m_bjetHandle{ this, "bjets", "pairedttHHAnalysisJets_%SYS%", "BJet container to read" };

      CP::SysReadHandle<xAOD::JetContainer>
      m_jetHandle{ this, "jets", "ttHHAnalysisJets_%SYS%", "Jet container to read" };

      CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

      CP::SysReadHandle<xAOD::ElectronContainer>
      m_electronHandle{ this, "electrons", "ttHHAnalysisElectrons_%SYS%", "Electron container to read" };

      CP::SysReadHandle<xAOD::MuonContainer>
      m_muonHandle{ this, "muons", "ttHHAnalysisMuons_%SYS%", "Muon container to read" };

      Gaudi::Property<std::string> m_eleWPName
      { this, "eleWP", "","Electron ID + Iso working point" };
      CP::SysReadDecorHandle<char> m_eleWPDecorHandle{"", this};
		      
      Gaudi::Property<std::string> m_muonWPName
      { this, "muonWP", "","Muon ID + Iso cuts" };
      CP::SysReadDecorHandle<char> m_muonWPDecorHandle{"", this};

      CP::SysWriteDecorHandle<bool> m_tight_selected_el {"tight_selected_el_%SYS%", this};
      CP::SysWriteDecorHandle<bool> m_tight_selected_mu {"tight_selected_mu_%SYS%", this};

      CP::SysReadDecorHandle<bool> m_passTriggerSinglep {this, "passTriggerSinglep", "ttHH_pass_trigger_singlep", "events pass any singlep triggers"};

      CP::SysFilterReporterParams m_filterParams {this, "ttHH selection"};

      Gaudi::Property<bool> m_bypass
      { this, "bypass", false, "Run selector algorithm in pass-through mode" };

      std::vector<std::string> m_inputCutList{};

      std::vector<std::string> m_leptonTriggers;

      bool m_saveCutFlow;
      long long int m_total_events{0};

      std::unordered_map<std::string, CP::SysWriteDecorHandle<bool> > m_Bbranches;
      
      std::unordered_map < ttHH::Trigger_Matching_Tool, bool > m_triggers_matchs;
      std::unordered_map < ttHH::Trigger_Matching_Tool, std::string > m_triggermatchingnames{
          {ttHH::pass_matching_trigger_singlep, "pass_matching_trigger_singlep"},
      };

      CP::SysWriteDecorHandle<bool> m_passallcuts {"PassAllCuts_%SYS%", this};
      
      ToolHandle<Trig::IMatchingTool> m_matchingTool{this, "trigMatchingTool", "",
	    "Trigger matching tool"};
  };

}

#endif // SELECTIONFLAGSTTHHALG_H
