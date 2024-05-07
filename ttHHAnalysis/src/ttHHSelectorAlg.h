/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!

#ifndef SELECTIONFLAGSTTHHALG_H
#define SELECTIONFLAGSTTHHALG_H

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <AsgDataHandles/ReadDecorHandle.h>

#include <AthContainers/ConstDataVector.h>

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <FourMomUtils/xAODP4Helpers.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <EasyjetHub/CutManager.h>

#include <SystematicsHandles/SysFilterReporterParams.h>

#include <algorithm>

class CutManager;

namespace ttHH
{

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

      const std::vector<std::string> m_STANDARD_CUTS{
          "PASS_TRIGGER",
          "PASS_BASELINE"
      };

      void evaluateCuts(const xAOD::JetContainer& bjets,
			const xAOD::MuonContainer& muons,
			const xAOD::ElectronContainer& electrons,
			CutManager& ttHHCuts);

    private :
      // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
      // "someInfo"};

      /// \brief Setup syst-aware input container handles
      CutManager m_ttHHCuts;
      CP::SysListHandle m_systematicsList {this};

      CP::SysReadHandle<xAOD::JetContainer>
      m_bjetHandle{ this, "bjets", "",   "BJet container to read" };

      CP::SysReadHandle<xAOD::JetContainer>
      m_jetHandle{ this, "jets", "",   "Jet container to read" };

      CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

      CP::SysReadHandle<xAOD::ElectronContainer>
      m_electronHandle{ this, "electrons", "",   "Electron container to read" };

      CP::SysReadHandle<xAOD::MuonContainer>
      m_muonHandle{ this, "muons", "",   "Muon container to read" };

      Gaudi::Property<std::string> m_eleWPName
      { this, "eleWP", "","Electron ID + Iso working point" };
      CP::SysReadDecorHandle<char> m_eleWPDecorHandle{"", this};
		      
      Gaudi::Property<std::string> m_muonWPName
      { this, "muonWP", "","Muon ID + Iso cuts" };
      CP::SysReadDecorHandle<char> m_muonWPDecorHandle{"", this};

      CP::SysWriteDecorHandle<bool> m_selected_el {"selected_el_%SYS%", this};
      CP::SysWriteDecorHandle<bool> m_selected_mu {"selected_mu_%SYS%", this};

      CP::SysReadDecorHandle<bool> m_passTriggerDilep {this, "passTriggerDilep", "ttHH_pass_trigger_dilep", "events pass any dilepton triggers"};
      CP::SysReadDecorHandle<bool> m_passTriggerSinglep {this, "passTriggerSinglep", "ttHH_pass_trigger_singlep", "events pass any singlep triggers"};

      CP::SysFilterReporterParams m_filterParams {this, "ttHH selection"};

      Gaudi::Property<bool> m_bypass
      { this, "bypass", false, "Run selector algorithm in pass-through mode" };

      std::vector<std::string> m_inputCutList{};

      bool m_saveCutFlow;
      long long int m_total_events{0};

      bool m_nLeptons;

      std::unordered_map<std::string, CP::SysWriteDecorHandle<bool> > m_Bbranches;

      CP::SysWriteDecorHandle<bool> m_passallcuts {"PassAllCuts_%SYS%", this};
  };

}

#endif // SELECTIONFLAGSTTHHALG_H
