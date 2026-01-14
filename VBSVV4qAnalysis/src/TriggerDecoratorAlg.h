/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

*/


// Always protect against multiple includes!
#ifndef VBSVV4qANALYSIS_TRIGGERDECORATORALG
#define VBSVV4qANALYSIS_TRIGGERDECORATORALG

#include "AnaAlgorithm/AnaAlgorithm.h"

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysFilterReporterParams.h>
#include <AsgDataHandles/ReadDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include "TriggerMatchingTool/IMatchingTool.h"

#include <EasyjetHub/CutManager.h>
#include <algorithm>

namespace VBSVV4q
{

  enum TriggerChannel
  {
    SJT, SJTM,
  };

  class TriggerDecoratorAlg final : public EL::AnaAlgorithm
  {
    public:
      TriggerDecoratorAlg(const std::string &name, ISvcLocator *pSvcLocator);

      StatusCode initialize() override;
      StatusCode execute() override;
      CP::SysListHandle m_systematicsList {this};

      CP::SysReadHandle<xAOD::EventInfo> m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

      CP::SysReadHandle<xAOD::JetContainer> m_jetsHandle{ this, "jets", "",   "jets container to read" };

      CP::SysReadDecorHandle<unsigned int> m_year {this, "year", "dataTakingYear", ""};

      Gaudi::Property<std::vector<std::string>> m_triggers 
      { this, "triggerLists", {}, "Name list of trigger" };

      std::unordered_map<std::string, CP::SysReadDecorHandle<bool> > m_triggerdecos;

      ToolHandle<Trig::IMatchingTool> m_matchingTool
      { this, "trigMatchingTool", "", "Trigger matching tool"};

      std::unordered_map<VBSVV4q::TriggerChannel, CP::SysWriteDecorHandle<bool> > m_trig_branches;
      std::unordered_map<VBSVV4q::TriggerChannel, bool> m_trig_bools;
      std::unordered_map<VBSVV4q::TriggerChannel, std::string> m_triggerChannels = 
      {
        {VBSVV4q::SJT, "SJT"},
        {VBSVV4q::SJTM, "SJTM"},
      };

      void checkSingleJetTrigger(const xAOD::EventInfo *event, const CP::SystematicSet& sys);
      void checkSingleJetTriggerMatching(const xAOD::EventInfo *event, const xAOD::Jet *jet, const CP::SystematicSet& sys);

  };
}

#endif