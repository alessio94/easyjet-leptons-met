/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

  BJetTriggerDecoratorAlg:
  An alg that saves b-jet trigger matching info as decorations
*/

// Always protect against multiple includes!
#ifndef BBTT_BJETTRIGGERDECORATORALG
#define BBTT_BJETTRIGGERDECORATORALG

#include <string>
#include <map>
#include <unordered_map>
#include <vector>

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>

#include "HHbbttEnums.h"
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysListHandle.h>

namespace HHBBTT
{
  
  class BJetTriggerDecoratorAlg final : public AthReentrantAlgorithm
  {
    
  public:
    BJetTriggerDecoratorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) const override;

  private:

    CP::SysListHandle m_systematicsList {this};
    CP::SysReadHandle<xAOD::JetContainer> m_jetHandle {
      this, "jets", "bbttAnalysisJets_%SYS%", "the jet collection to run on"};
    CP::SysWriteDecorHandle<bool> m_bjet_trigMatch_Write{
      this, "trigMatch_DBT", "passTrigMatch_DBT_%SYS%", "aggregated b-jet trigger matching"};

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey
      { this, "event", "EventInfo", "EventInfo to read" };
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_yearKey;

    Gaudi::Property<std::vector<std::string>> m_triggers 
      { this, "triggerLists", {}, "Name list of trigger" };
   
    const std::unordered_map<HHBBTT::RunBooleans, std::string> m_runBooleans =
      {
        {HHBBTT::is23_from1200bunches, "is2023_from1200bunches"},
      };

    std::map<HHBBTT::RunBooleans, SG::ReadDecorHandleKey<xAOD::EventInfo>> m_runBooleans_key;
    std::unordered_map<std::string, SG::ReadDecorHandleKey<xAOD::EventInfo>> m_triggerdecoKeys;
    std::unordered_map<std::string, CP::SysReadDecorHandle<char>> m_bjet_trigMatch_Read;
    
    typedef std::unordered_map<HHBBTT::RunBooleans, SG::ReadDecorHandle<xAOD::EventInfo, bool> > runBoolReadDecoMap;
    typedef std::unordered_map<std::string, SG::ReadDecorHandle<xAOD::EventInfo, bool> > trigReadDecoMap;
    typedef std::unordered_map<std::string, CP::SysReadDecorHandle<char> > bjetReadTrigMatchMap;
    typedef CP::SysWriteDecorHandle<bool> bjetTrigMatchWriteDeco;

    void checkDiBJetMatchingTriggers(
      int year,
      const xAOD::EventInfo* eventInfo,
      const runBoolReadDecoMap& runBoolDecos,
      const trigReadDecoMap& triggerdecos,
      const xAOD::JetContainer* jets,
      const bjetTrigMatchWriteDeco& bjet_trigMatchDeco,
      const bjetReadTrigMatchMap& bjetTrigMatch,
      const CP::SystematicSet& sys) const;
  };
}

#endif