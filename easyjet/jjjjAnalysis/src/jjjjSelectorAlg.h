/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!

#ifndef JJJJANALYSIS_JJJJSELECTORALG
#define JJJJANALYSIS_JJJJSELECTORALG

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
#include "TriggerMatchingTool/IMatchingTool.h"
#include <EasyjetHub/CutManager.h>

namespace jjjj
{
  enum TriggerChannel
  {
    SJT,
  };
  enum Booleans{
    PASS_TRIGGER,
    pass_trigger_SJT,
  };
  enum Var {
    j1 = 0,
  };

  /// \brief An algorithm for counting containers
  class jjjjSelectorAlg final : public EL::AnaAlgorithm {

    public:
      jjjjSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

      /// \brief Initialisation method, for setting up tools and other persistent
      /// configs
      StatusCode initialize() override;
      /// \brief Execute method, for actions to be taken in the event loop
      StatusCode execute() override;
      /// \brief This is the mirror of initialize() and is called after all events are processed.
      StatusCode finalize() override; ///I added this to write the cutflow histogram.

      const std::vector<std::string> m_BASELINE_CUTS{
        "PASS_TRIGGER",
      };


    private :
      // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
      // "someInfo"};

      Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

      Gaudi::Property<bool> m_bypass
      { this, "bypass", false, "Run selector algorithm in pass-through mode" };
      CP::SysReadDecorHandle<unsigned int> m_year
  	{this, "year", "dataTakingYear", ""};

      /// \brief Setup syst-aware input container handles
      CP::SysListHandle m_systematicsList {this};

      CP::SysReadHandle<xAOD::JetContainer>
      m_jetHandle{ this, "jets", "jjjjAnalysisJets_%SYS%", "Jet container to read" };

      CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };
      
      CP::SysFilterReporterParams m_filterParams {this, "jjjj selection"};

      std::unordered_map<std::string,  SG::ReadDecorHandleKey<xAOD::EventInfo>> m_triggerDecorKeys;

      std::unordered_map<jjjj::TriggerChannel, std::string> m_triggerChannels = 
      {
        {jjjj::SJT, "SJT"},
      };

      Gaudi::Property<std::vector<std::string>> m_triggers 
      { this, "triggerLists", {}, "Name list of trigger" };

      std::unordered_map<std::string, CP::SysReadDecorHandle<bool> > m_triggerdecos;

      ToolHandle<Trig::IMatchingTool> m_matchingTool
      { this, "trigMatchingTool", "", "Trigger matching tool"};

      long long int m_total_events{0};

      std::unordered_map<jjjj::Booleans, CP::SysWriteDecorHandle<bool> > m_Bbranches;
      std::unordered_map<jjjj::Booleans, bool> m_bools;
      std::unordered_map<jjjj::Booleans, std::string> m_boolnames{
        {jjjj::pass_trigger_SJT, "pass_trigger_SJT"},
        {jjjj::PASS_TRIGGER, "PASS_TRIGGER"},
      };

      CutManager m_jjjjCuts;
      Gaudi::Property<std::vector<std::string>> m_inputCutList{this, "cutList", {}};
      std::vector<jjjj::Booleans> m_inputCutKeys;
      Gaudi::Property<bool> m_saveCutFlow{this, "saveCutFlow", false};
      CP::SysWriteDecorHandle<bool> m_pass_cuts {"PassCuts_%SYS%", this};

      std::unordered_map<jjjj::TriggerChannel, std::unordered_map<jjjj::Var, float>> m_pt_threshold;

      void evaluateTriggerCuts
    	  (const xAOD::EventInfo* event, const xAOD::Jet* j1,
      	  CutManager& jjjjCuts, const CP::SystematicSet& sys);
      void setThresholds(const xAOD::EventInfo* event, const CP::SystematicSet& sys);

      void evaluateSingleJetTrigger
      	(const xAOD::EventInfo* event, const xAOD::Jet* j1, 
	        const CP::SystematicSet& sys);

  };

}

#endif // jjjjANALYSIS_jjjjSELECTORALG

