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

      std::unordered_map<VBSHIGGS::TriggerChannel, CP::SysWriteDecorHandle<bool> > m_trig_branches;
      std::unordered_map<VBSHIGGS::TriggerChannel, bool> m_trig_bools;
      std::unordered_map<VBSHIGGS::TriggerChannel, std::string> m_triggerChannels = 
      {
        {VBSHIGGS::SLT, "SLT"},
        {VBSHIGGS::DLT, "DLT"},
        {VBSHIGGS::ASLT1_em, "ASLT1_em"},
        {VBSHIGGS::ASLT1_me, "ASLT1_me"},
        {VBSHIGGS::ASLT2, "ASLT2"},
      };

      std::unordered_map<VBSHIGGS::RunBooleans, std::string> m_runBooleans =
      {
        {VBSHIGGS::is17_periodB5_B8, "is2017_periodB5_B8"},
        {VBSHIGGS::is22_75bunches, "is2022_75bunches"},
        {VBSHIGGS::is23_75bunches, "is2023_75bunches"},
        {VBSHIGGS::is23_400bunches, "is2023_400bunches"},
      };
      std::map<VBSHIGGS::RunBooleans, SG::ReadDecorHandleKey<xAOD::EventInfo>> m_runBooleans_key;
      typedef std::unordered_map<VBSHIGGS::RunBooleans, SG::ReadDecorHandle<xAOD::EventInfo, bool> > runBoolReadDecoMap;

      void evaluateTriggerCuts(const xAOD::EventInfo *event,
			       const runBoolReadDecoMap& runBoolDecos,
			       const xAOD::ElectronContainer *electrons ,
			       const xAOD::MuonContainer *muons,
			       const CP::SystematicSet& sys);
      void evaluateSingleLeptonTrigger(const xAOD::EventInfo* event,
				       const runBoolReadDecoMap& runBoolDecos,
				       const xAOD::Electron* ele, const xAOD::Muon* mu,
				       const CP::SystematicSet& sys);
      void evaluateDiLeptonTrigger(const xAOD::EventInfo* event,
				   const runBoolReadDecoMap& runBoolDecos,
				   const xAOD::Electron* ele0, const xAOD::Electron* ele1,
				   const xAOD::Muon* mu0, const xAOD::Muon* mu1,
				   const CP::SystematicSet& sys);
      void evaluateAsymmetricLeptonTrigger(const xAOD::EventInfo* event,
					   const xAOD::Electron* ele, const xAOD::Muon* mu,
					   const CP::SystematicSet& sys);
      void setThresholds(const xAOD::EventInfo* event,
			 const runBoolReadDecoMap& runBoolDecos,
			 const CP::SystematicSet& sys);
  };
}

#endif
