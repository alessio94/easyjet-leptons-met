/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HH4BANALYSIS_SMALLRJETTRIGGERSFALG
#define HH4BANALYSIS_SMALLRJETTRIGGERSFALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AsgDataHandles/WriteDecorHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysListHandle.h>

#include <AthContainers/ConstDataVector.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <TH2D.h>

namespace HH4B
{
  enum class TrigMatchingLevel
  {
    L1,
    HLT
  };
  class SmallRJetTriggerSFAlg final : public AthHistogramAlgorithm
  {
public:
    SmallRJetTriggerSFAlg(const std::string &name, ISvcLocator *pSvcLocator);

    StatusCode initialize() override;
    StatusCode execute() override;

private:
    // sys-aware input container handles
    CP::SysListHandle m_systematicsList{this};

    CP::SysReadHandle<xAOD::EventInfo> m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };
    CP::SysReadHandle<xAOD::JetContainer> m_inJetHandle{ this, "containerInKey", "", "Input jet container to read"};
    CP::SysWriteHandle<ConstDataVector<xAOD::JetContainer>> m_outJetHandle{this, "containerOutKey", "", "Output jet container to write"};

    Gaudi::Property<std::vector<std::string>> m_triggers{ this, "triggers", {}, "List of triggers"};
    Gaudi::Property<std::vector<unsigned int>> m_years{ this, "years", {}, "List of years"};
    Gaudi::Property<bool> m_doL1SF{this, "doL1SF", false, "Calculate jet trigger L1 SF"};
    Gaudi::Property<bool> m_doHLTSF{this, "doHLTSF", false, "Calculate jet trigger HLT SF"};
    Gaudi::Property<std::string> m_matchingLevel{this, "matchingLevel", "HLT", "do L1 or HLT matching and SF"};

    std::unordered_map<std::string, std::vector<int>> m_triggerLegThresholds;
    std::unordered_map<std::string, SG::ReadDecorHandleKey<xAOD::JetContainer>> m_ThresholdsDecorKey; // list of matched thresholds from JetDecoratorAlg
    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Threshold; // single matched threshold for each jet after event-level ambituity resolution
    std::unordered_map<std::string, CP::SysReadDecorHandle<int>> m_Thresholdread; // read handle for single matched threshold for each jet

    // jet-level jet trigger  SF
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_jetSF;
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_jetSFup;
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_jetSFdown;

    // event-level jet trigger  SF
    std::unordered_map<std::string, SG::WriteDecorHandleKey<xAOD::EventInfo>> m_eventSFKey;
    std::unordered_map<std::string, SG::WriteDecorHandleKey<xAOD::EventInfo>> m_eventSFupKey;
    std::unordered_map<std::string, SG::WriteDecorHandleKey<xAOD::EventInfo>> m_eventSFdownKey;

    // calibration map
    std::unordered_map<int, TH2D*> m_jetTriggerSFMap;

    std::optional<TrigMatchingLevel> m_matchingLevelEnum;
  };
}

#endif