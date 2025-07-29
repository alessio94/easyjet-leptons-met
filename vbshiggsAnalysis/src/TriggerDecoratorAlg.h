/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

*/


// Always protect against multiple includes!
#ifndef VBSHIGGSANALYSIS_TRIGGERDECORATORALG
#define VBSHIGGSANALYSIS_TRIGGERDECORATORALG

#include "AnaAlgorithm/AnaAlgorithm.h"

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysFilterReporterParams.h>
#include <AsgDataHandles/ReadDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>
#include "TriggerMatchingTool/IMatchingTool.h"

#include <EasyjetHub/CutManager.h>
#include <algorithm>
#include "vbshiggsEnums.h"

namespace VBSHIGGS
{
  class TriggerDecoratorAlg final : public EL::AnaAlgorithm
  {
    public:
      TriggerDecoratorAlg(const std::string &name, ISvcLocator *pSvcLocator);

      StatusCode initialize() override;
      StatusCode execute() override;
      CP::SysListHandle m_systematicsList {this};

      std::unordered_map<VBSHIGGS::TriggerChannel, std::unordered_map<VBSHIGGS::Var_Trigger, float>> m_pt_threshold;

      CP::SysReadHandle<xAOD::EventInfo> m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

      CP::SysReadHandle<xAOD::ElectronContainer> m_electronHandle{ this, "electrons", "",   "Electron container to read" };

      CP::SysReadHandle<xAOD::MuonContainer> m_muonHandle{ this, "muons", "",   "Muon container to read" };

      CP::SysReadDecorHandle<unsigned int> m_year {this, "year", "dataTakingYear", ""};

      Gaudi::Property<std::vector<std::string>> m_triggers 
      { this, "triggerLists", {}, "Name list of trigger" };

      std::unordered_map<std::string, CP::SysReadDecorHandle<bool> > m_triggerdecos;

      ToolHandle<Trig::IMatchingTool> m_matchingTool
      { this, "trigMatchingTool", "", "Trigger matching tool"};

      Gaudi::Property<std::vector<std::string>> m_eleTrigSF
      { this, "eleTriggerSF", {}, "List of electron trigger SF" };
      std::unordered_map<std::string, CP::SysReadDecorHandle<float>> m_eleTriggerSF;

      Gaudi::Property<std::vector<std::string>> m_muTrigSF
      { this, "muTriggerSF", {}, "List of muon trigger SF" };
      std::unordered_map<std::string, CP::SysReadDecorHandle<float>> m_muTriggerSF;

      CP::SysWriteDecorHandle<float> m_eventTriggerSF {this, "event_trigEffSF", "event_trigEffSF_%SYS%", "Event-level trigger scale factor"};

      Gaudi::Property<bool> m_saveHighLevelVariables
      { this, "saveHighLevelVariables", false, "Flag to save high level variables"};

      std::unordered_map<VBSHIGGS::TriggerChannel, CP::SysWriteDecorHandle<bool> > m_trig_branches;
      std::unordered_map<VBSHIGGS::TriggerChannel, bool> m_trig_bools;
      std::unordered_map<VBSHIGGS::TriggerChannel, std::string> m_triggerChannels = 
      {
        {VBSHIGGS::SLT, "SLT"},
      };

      std::unordered_map<VBSHIGGS::RunBooleans, std::string> m_runBooleans =
      {
        {VBSHIGGS::is22_75bunches, "is2022_75bunches"},
        {VBSHIGGS::is23_75bunches, "is2023_75bunches"},
        {VBSHIGGS::is23_400bunches, "is2023_400bunches"},
      };
      std::map<VBSHIGGS::RunBooleans, SG::ReadDecorHandleKey<xAOD::EventInfo>> m_runBooleans_key;
      typedef std::unordered_map<VBSHIGGS::RunBooleans, SG::ReadDecorHandle<xAOD::EventInfo, bool> > runBoolReadDecoMap;

      bool ele0_passSET = false;
      bool ele1_passSET = false;
      bool mu0_passSMT = false;
      bool mu1_passSMT = false;

      std::vector<std::string> ele0_trigPassed;
      std::vector<std::string> ele1_trigPassed;
      std::vector<std::string> mu0_trigPassed;
      std::vector<std::string> mu1_trigPassed;

      std::vector<std::string> ele0_trigMatched;
      std::vector<std::string> ele1_trigMatched;
      std::vector<std::string> mu0_trigMatched;
      std::vector<std::string> mu1_trigMatched;

      CP::SysWriteDecorHandle<bool> ele0_passSET_decor {this, "ele0_passSET", "ele0_passSET_%SYS%", "Flag if Ele0 passes any Single Electron Trigger"};
      CP::SysWriteDecorHandle<bool> ele1_passSET_decor {this, "ele1_passSET", "ele1_passSET_%SYS%", "Flag if Ele1 passes any Single Electron Trigger"};
      CP::SysWriteDecorHandle<bool> mu0_passSMT_decor {this, "mu0_passSMT", "mu0_passSMT_%SYS%", "Flag if Mu0 passes any Single Muon Trigger"};
      CP::SysWriteDecorHandle<bool> mu1_passSMT_decor {this, "mu1_passSMT", "mu1_passSMT_%SYS%", "Flag if Mu1 passes any Single Muon Trigger"};

      CP::SysWriteDecorHandle<std::vector<std::string>> ele0_trigPassed_decor {this, "ele0_trigPassed", "ele0_trigPassed_%SYS%", "List of Triggers that Ele0 Passes"};
      CP::SysWriteDecorHandle<std::vector<std::string>> ele1_trigPassed_decor {this, "ele1_trigPassed", "ele1_trigPassed_%SYS%", "List of Triggers that Ele1 Passes"};
      CP::SysWriteDecorHandle<std::vector<std::string>> mu0_trigPassed_decor {this, "mu0_trigPassed", "mu0_trigPassed_%SYS%", "List of Triggers that Mu0 Passes"};
      CP::SysWriteDecorHandle<std::vector<std::string>> mu1_trigPassed_decor {this, "mu1_trigPassed", "mu1_trigPassed_%SYS%", "List of Triggers that Mu1 Passes"};

      CP::SysWriteDecorHandle<std::vector<std::string>> ele0_trigMatched_decor {this, "ele0_trigMatched", "ele0_trigMatched_%SYS%", "List of Triggers that Ele0 Matches"};
      CP::SysWriteDecorHandle<std::vector<std::string>> ele1_trigMatched_decor {this, "ele1_trigMatched", "ele1_trigMatched_%SYS%", "List of Triggers that Ele1 Matches"};
      CP::SysWriteDecorHandle<std::vector<std::string>> mu0_trigMatched_decor {this, "mu0_trigMatched", "mu0_trigMatched_%SYS%", "List of Triggers that Mu0 Matches"};
      CP::SysWriteDecorHandle<std::vector<std::string>> mu1_trigMatched_decor {this, "mu1_trigMatched", "mu1_trigMatched_%SYS%", "List of Triggers that Mu1 Matches"};

      void evaluateTriggerCuts(const xAOD::EventInfo *event,
			       const runBoolReadDecoMap& runBoolDecos,
			       const xAOD::ElectronContainer *electrons ,
			       const xAOD::MuonContainer *muons,
			       const CP::SystematicSet& sys);
      void evaluateSingleLeptonTrigger(const xAOD::EventInfo* event,
				       const runBoolReadDecoMap& runBoolDecos,
				       const xAOD::Electron* ele, const xAOD::Muon* mu,
				       const CP::SystematicSet& sys, 
               std::vector<std::string>& ele_trigPassed, std::vector<std::string>& mu_trigPassed,
               std::vector<std::string>& ele_trigMatched, std::vector<std::string>& mu_trigMatched,
               bool& ele_passSET, bool& mu_passSMT);
      void setThresholds(const xAOD::EventInfo* event,
			 const runBoolReadDecoMap& runBoolDecos,
			 const CP::SystematicSet& sys);
  };
}

#endif
