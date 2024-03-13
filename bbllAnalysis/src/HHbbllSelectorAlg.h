/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!

#ifndef HHBBLLANALYSIS_HHBBLLSELECTORALG
#define HHBBLLANALYSIS_HHBBLLSELECTORALG

#include <memory>
#include "AnaAlgorithm/AnaAlgorithm.h"
#include <FourMomUtils/xAODP4Helpers.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/ISysHandleBase.h>
#include <SystematicsHandles/SysFilterReporterParams.h>

#include <AthContainers/ConstDataVector.h>
#include <AsgDataHandles/ReadDecorHandle.h>

#include <AsgDataHandles/ReadDecorHandle.h>
#include <AthContainers/ConstDataVector.h>

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODMissingET/MissingETContainer.h>
#include "TriggerMatchingTool/IMatchingTool.h"
#include <EasyjetHub/CutManager.h>

#include "TriggerMatchingTool/IMatchingTool.h"
#include <EasyjetHub/CutManager.h>

namespace HHBBLL
{

  /// \brief An algorithm for counting containers
  class HHbbllSelectorAlg final : public AthHistogramAlgorithm {

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

      std::vector<std::string> m_Bvarnames{
      "Pass_ll", 
      "IS_SF",
      "IS_ee",
      "IS_mm",
      "IS_em",
      };

      void evaluateTriggerCuts(const xAOD::EventInfo& eventInfo, const std::vector<std::string> &passTriggers, const xAOD::Electron* ele0,  
                          const xAOD::Electron* ele1, const xAOD::Muon* mu0, const xAOD::Muon* mu1, CutManager& bbllCuts);
      void evaluateLeptonCuts(const xAOD::ElectronContainer& electrons,
                          const xAOD::MuonContainer& muons, CutManager& bbllCuts);
      void evaluateJetCuts(const ConstDataVector<xAOD::JetContainer>& bjets,
                          const ConstDataVector<xAOD::JetContainer>& nonbjets, CutManager& bbllCuts);
      void evaluateBJetLeptonCuts(const xAOD::EventInfo& eventInfo,
		          const ConstDataVector<xAOD::JetContainer>& bjets,
                          const xAOD::ElectronContainer& electrons, const xAOD::MuonContainer& muons);

    private :
      // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
      // "someInfo"};

      Gaudi::Property<std::vector<int>> m_years
      { this, "Years", false, "which years are running" };

      Gaudi::Property<bool> m_bypass
      { this, "bypass", false, "Run selector algorithm in pass-through mode" };

      /// \brief Setup syst-aware input container handles
      CutManager m_bbllCuts;
      CP::SysListHandle m_systematicsList {this};

      CP::SysReadHandle<xAOD::JetContainer>
      m_jetHandle{ this, "jets", "",   "Jet container to read" };

      CP::SysReadDecorHandle<char> 
      m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};

      CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

      CP::SysReadHandle<xAOD::ElectronContainer>
      m_electronHandle{ this, "electrons", "",   "Electron container to read" };

      CP::SysReadHandle<xAOD::MuonContainer>
      m_muonHandle{ this, "muons", "",   "Muon container to read" };

      CP::SysReadHandle<xAOD::MissingETContainer>
	m_metHandle{ this, "met", "AnalysisMET",   "MET container to read" };

      CP::SysFilterReporterParams m_filterParams {this, "HHbbll selection"};

      std::vector<std::string> m_inputCutList{};

      std::vector<std::string> m_passTriggers;

      std::vector<std::string> m_Bvarname{};

      std::unordered_map<std::string,  SG::ReadDecorHandleKey<xAOD::EventInfo>> m_triggerDecorKeys;

      bool m_saveCutFlow;
      long long int m_total_events{0};

      std::unordered_map<std::string, CP::SysWriteDecorHandle<bool> > m_Bbranches;

      CP::SysWriteDecorHandle<bool> m_passallcuts {"PassAllCuts_%SYS%", this};

      ToolHandle<Trig::IMatchingTool> m_matchingTool{this, "trigMatchingTool", "",
	    "Trigger matching tool"};
  };

}

#endif // HHBBLLANALYSIS_HHBBLLSELECTORALG

