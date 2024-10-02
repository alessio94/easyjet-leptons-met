/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!

#ifndef SELECTIONFLAGS4BALG_H
#define SELECTIONFLAGS4BALG_H

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysFilterReporterParams.h>

// For custom event weights
#include <AsgDataHandles/ReadDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>

#include <EasyjetHub/CutManager.h>

class CutManager;

namespace HH4B
{
  /// \brief An algorithm for counting containers
  class bbbbSelectorAlg final : public AthHistogramAlgorithm {

    public:
      bbbbSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

      /// \brief Initialisation method, for setting up tools and other persistent
      /// configs
      StatusCode initialize() override;
      /// \brief Execute method, for actions to be taken in the event loop
      StatusCode execute() override;
      /// \brief This is the mirror of initialize() and is called after all events are processed.
      StatusCode finalize() override; ///I added this to write the cutflow histogram.

    private :

      const std::vector<std::string> m_STANDARD_CUTS{
          "PASS_TRIGGER",
          "AT_LEAST_FOUR_JETS_OR_TWO_LRJETS",
          "AT_LEAST_FOUR_JETS_TWO_BJETS_OR_TWO_LRJETS",
          "AT_LEAST_TWO_LRJETS",
          "AT_LEAST_FOUR_JETS",
          "AT_LEAST_TWO_BJETS",
      };

      void evaluateTriggerCuts(const xAOD::EventInfo& event, 
                          const std::vector<std::string> &Triggers, CutManager& bbbbCuts);
      void evaluateJetCuts(const int nBoostedJets, const int nResolvedJets, const int nBJets, CutManager& bbbbCuts);

      /// \brief Setup syst-aware input container handles
      CutManager m_bbbbCuts;
      CP::SysListHandle m_systematicsList {this};

      CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

      CP::SysReadHandle<xAOD::JetContainer>
        m_lRjetHandle{ this, "largeRJetToCount", "", "Large-R jet container to count" };
  
      CP::SysReadHandle<xAOD::JetContainer>
        m_jetHandle{ this, "smallRJetToCount", "", "Small-R jet container to count" }; // For counting only

      Gaudi::Property<std::string> m_btagSelDecor {this, "btagSelDecor", "", "B-tagging working point."};
      SG::AuxElement::ConstAccessor<char> m_acc_btagSel{"btagSelAcc"};

      Gaudi::Property<std::vector<std::string>> m_inputCutList{this, "cutList", {}};

      Gaudi::Property<std::vector<std::string>> m_Triggers{this, "Triggers", {}};

      std::unordered_map<std::string,  SG::ReadDecorHandleKey<xAOD::EventInfo>> m_triggerDecorKeys;

      Gaudi::Property<bool> m_saveCutFlow{this, "saveCutFlow", false};
      long long int m_total_events{0};
      double m_total_mcEventWeight{0.0};

      Gaudi::Property<bool> m_isMC
        { this, "isMC", false, "Is this simulation?" };

      CP::SysFilterReporterParams m_filterParams {this, "bbbb selection"};
      Gaudi::Property<bool> m_bypass
        { this, "bypass", false, "Run the selector algorithm in run-through mode" };

      std::unordered_map<std::string, CP::SysWriteDecorHandle<bool> > m_Bbranches;

      CP::SysWriteDecorHandle<bool> m_passallcuts {"PassAllCuts_%SYS%", this};

      std::vector<float> eventWeights{};

      CP::SysReadDecorHandle<float>
        m_generatorWeight{ this, "generatorWeight", "generatorWeight_%SYS%", "MC event weights" };

  };

}

#endif // SELECTIONFLAGS4BALG_H
