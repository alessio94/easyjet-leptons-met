/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!

#ifndef SELECTIONFLAGSYYBBALG_H
#define SELECTIONFLAGSYYBBALG_H

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysFilterReporterParams.h>

// For custom event weights
#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

#include <AthContainers/ConstDataVector.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/PhotonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>

#include "TriggerMatchingTool/IMatchingTool.h"

#include <EasyjetHub/CutManager.h>

class CutManager;

namespace HHBBYY
{
    enum Booleans
    {
        is15,
        is16,
        pass_trigger_single_photon,
        pass_trigger_diphoton,
        pass_matching_trigger_single_photon,
        pass_matching_trigger_diphoton,
    };

  /// \brief An algorithm for counting containers
  class bbyySelectorAlg final : public AthHistogramAlgorithm {

    public:
      bbyySelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

      /// \brief Initialisation method, for setting up tools and other persistent
      /// configs
      StatusCode initialize() override;
      /// \brief Execute method, for actions to be taken in the event loop
      StatusCode execute() override;
      /// \brief This is the mirror of initialize() and is called after all events are processed.
      StatusCode finalize() override; ///I added this to write the cutflow histogram.

      const std::vector<std::string> m_STANDARD_CUTS{
          "PASS_TRIGGER",
          "TWO_LOOSE_PHOTONS",
          "PASS_TRIGGER_MATCHING",
	  "TWO_TIGHTID_ISO_PHOTONS",
          "TWO_TIGHTID_PHOTONS",
          "TWO_ISO_PHOTONS",
          "PASS_RELPT",
          "DIPHOTON_MASS",
          "EXACTLY_ZERO_LEPTONS",
          "AT_LEAST_TWO_JETS",
          "LESS_THAN_SIX_CENTRAL_JETS",
          "AT_LEAST_ONE_B_JET",
          "AT_LEAST_TWO_B_JETS",
          "EXACTLY_ONE_B_JET",
          "EXACTLY_TWO_B_JETS",
          "EXACTLY_ONE_OR_TWO_B_JETS",
      };

      void evaluateTriggerCuts(const xAOD::EventInfo& eventInfo, 
                          const std::vector<std::string> &photonTriggers, CutManager& bbyyCuts);
      void evaluateTriggerMatchingCuts(const std::vector<std::string> &photonTriggers, 
                                        const xAOD::PhotonContainer* photons, CutManager& bbyyCuts);
      void evaluatePhotonCuts(const std::vector<const xAOD::Photon*>& photons, int n_TightID_NonIso_photons, int n_Loose_Iso_photons, int n_TightID_Iso_photons, CutManager& bbyyCuts);
      void evaluateLeptonCuts(const xAOD::ElectronContainer& electrons,
                          const xAOD::MuonContainer& muons, CutManager& bbyyCuts);
      void evaluateJetCuts(const ConstDataVector<xAOD::JetContainer>& bjets,
                          const xAOD::JetContainer& jets, CutManager& bbyyCuts);

    private :
      // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
      // "someInfo"};

      /// \brief Setup syst-aware input container handles
      CutManager m_bbyyCuts;
      CP::SysListHandle m_systematicsList {this};

      CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };
  
      CP::SysReadHandle<xAOD::JetContainer>
      m_jetHandle{ this, "jets", "bbyyAnalysisJets_%SYS%", "Jet container to read" };

      CP::SysReadDecorHandle<char> 
      m_isBtag {this, "bTagWPDecorName", "", "Name of input decorator for b-tagging"};

      CP::SysReadHandle<xAOD::PhotonContainer>
      m_photonHandle{ this, "photons", "bbyyAnalysisPhotons_%SYS%", "Photons container to read" };

      CP::SysReadHandle<xAOD::ElectronContainer>
      m_electronHandle{ this, "electrons", "bbyyAnalysisElectrons_%SYS%", "Electron container to read" };

      CP::SysReadHandle<xAOD::MuonContainer>
      m_muonHandle{ this, "muons", "bbyyAnalysisMuons_%SYS%", "Muon container to read" };

      Gaudi::Property<std::string> m_photonWPName
      { this, "photonWP", "", "Photon ID + Iso cuts" };

      Gaudi::Property<std::string> m_photon_TightID_NonIso_WPName
      { this, "photon_TNI_WP", "Tight_NonIso", "Photon tight ID + non Iso working point" };

      Gaudi::Property<std::string> m_photon_LooseID_Iso_WPName
      { this, "photon_LI_WP", "Loose_FixedCutLoose", "Photon loose ID + Iso working point" };

      CP::SysReadDecorHandle<char> m_photonWPDecorHandle{"", this};

      CP::SysReadDecorHandle<char> m_photonTNIWPDecorHandle{"", this};

      CP::SysReadDecorHandle<char> m_photonLIWPDecorHandle{"", this};

      CP::SysWriteDecorHandle<bool> m_selected_ph {"selected_ph_%SYS%", this};

      std::vector<std::string> m_inputCutList{};

      std::vector<std::string> m_photonTriggers;

      std::unordered_map<std::string,  SG::ReadDecorHandleKey<xAOD::EventInfo>> m_triggerDecorKeys;

      bool m_saveCutFlow;
      bool m_saveTriggerInfo;
      long long int m_total_events{0};
      double m_total_mcEventWeight{0.0};

      Gaudi::Property<bool> m_isMC
        { this, "isMC", false, "Is this simulation?" };

      CP::SysFilterReporterParams m_filterParams {this, "bbyy selection"};
      Gaudi::Property<bool> m_bypass
        { this, "bypass", false, "Run the selector algorithm in run-through mode" };
      Gaudi::Property<bool> m_enableSinglePhotonTrigger
        { this, "enableSinglePhotonTrigger", false, "Enable single photon trigger in the CutFlow" };

      std::unordered_map<std::string, CP::SysWriteDecorHandle<bool> > m_Bbranches;
      std::unordered_map < HHBBYY::Booleans, bool > m_bools;
      std::unordered_map < HHBBYY::Booleans, std::string > m_boolnames{
          {HHBBYY::is15, "is15"},
          {HHBBYY::is16, "is16"},
          {HHBBYY::pass_trigger_single_photon, "pass_trigger_single_photon"},
          {HHBBYY::pass_trigger_diphoton, "pass_trigger_diphoton"},
          {HHBBYY::pass_matching_trigger_single_photon, "pass_matching_trigger_single_photon"},
          {HHBBYY::pass_matching_trigger_diphoton, "pass_matching_trigger_diphoton"},
      };

      // map to check if single or diphoton trigger
      std::unordered_map<std::string, std::string> m_triggerMap{
        {"HLT_g120_loose", "single_photon"},
        {"HLT_g140_loose", "single_photon"},
        {"HLT_g35_loose_g25_loose", "diphoton"},
        {"HLT_g35_medium_g25_medium_L12EM20VH", "diphoton"},
        {"HLT_g35_medium_g25_medium_L12eEM24L", "diphoton"}
    };

      CP::SysWriteDecorHandle<bool> m_passallcuts {"PassAllCuts_%SYS%", this};
      ToolHandle<Trig::IMatchingTool> m_matchingTool{this, "trigMatchingTool", "",
	    "Trigger matching tool"};
      CP::SysReadDecorHandle<unsigned int> m_runNumber 
        {this, "runNumber", "runNumber", "Runnumber"};
      CP::SysReadDecorHandle<unsigned int> m_rdmRunNumber 
        {this, "randomRunNumber", "RandomRunNumber", "Random run number for MC"};
      
      void setRunNumberQuantities(unsigned int rdmNumber);

      std::vector<float> eventWeights{};

      Gaudi::Property<std::string> m_specialSysWeight
        { this, "specialSysWeight", "", "Special sys weight Name based on MCChannelNumber"}; 

      CP::SysReadDecorHandle<float>
        m_generatorWeight{ this, "generatorWeight", "generatorWeight_%SYS%", "MC event weights" };

  };

}

#endif // SELECTIONFLAGSYYBBALG_H
