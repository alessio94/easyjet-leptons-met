/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "BJetTriggerDecoratorAlg.h"
#include "TriggerUtils.h"
#include <StoreGate/StoreGateSvc.h>


namespace HHBBTT
{
  BJetTriggerDecoratorAlg::BJetTriggerDecoratorAlg(const std::string &name,
					   ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator) { }

  StatusCode BJetTriggerDecoratorAlg::initialize()
  {
    ATH_CHECK (m_eventInfoKey.initialize());
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));

    m_yearKey = "EventInfo.dataTakingYear";
    ATH_CHECK(m_yearKey.initialize());

    if (m_triggers.empty()) {
      ATH_MSG_WARNING("No b-jet triggers were configured for BJetTriggerDecoratorAlg");
    }

    for (const auto& trig : m_triggers) {
      SG::ReadDecorHandleKey<xAOD::EventInfo> deco;
      deco = "EventInfo.trigPassed_" + trig;
      m_triggerdecoKeys.emplace(trig, deco);
      ATH_CHECK(m_triggerdecoKeys.at(trig).initialize());

      CP::SysReadDecorHandle<char> rhandle{"ftag_bTagTrigMatching_" + trig + "_%SYS%",
        this};
      m_bjet_trigMatch_Read.emplace(trig, rhandle);
      ATH_CHECK(m_bjet_trigMatch_Read.at(trig).initialize(m_systematicsList,
        m_jetHandle));
    }

    for(const auto& [runBool, name] : m_runBooleans) {
      SG::ReadDecorHandleKey<xAOD::EventInfo> deco;
      deco = "EventInfo." + name;
      m_runBooleans_key.emplace(runBool, deco);
      ATH_CHECK(m_runBooleans_key.at(runBool).initialize());
    }

    ATH_CHECK(m_bjet_trigMatch_Write.initialize(m_systematicsList, m_jetHandle));
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode BJetTriggerDecoratorAlg::execute (const EventContext& ctx) const
  {
    SG::ReadHandle<xAOD::EventInfo> eventInfo{m_eventInfoKey, ctx};
    ATH_CHECK(eventInfo.isValid());

    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      runBoolReadDecoMap runBoolDecos;
      for (const auto& [runBool, key] : m_runBooleans_key) {
        runBoolDecos.emplace(runBool, key);
      }

      trigReadDecoMap triggerdecos;
      for (const auto& [name, key] : m_triggerdecoKeys) {
        triggerdecos.emplace(name, key);
      }

      bjetReadTrigMatchMap bjetTrigMatch;
      for (const auto& [trigger, key] : m_bjet_trigMatch_Read) {
        bjetTrigMatch.emplace(trigger, key);
      }

      for (const xAOD::Jet* jet : *jets) {
        m_bjet_trigMatch_Write.set(*jet, false, sys);
      }

      if (m_triggers.empty()) {
        continue;
      }

      SG::ReadDecorHandle<xAOD::EventInfo, unsigned int> year{m_yearKey, ctx};

      checkDiBJetMatchingTriggers(year(*eventInfo), eventInfo.cptr(),
                                  runBoolDecos, triggerdecos, jets, 
                                  m_bjet_trigMatch_Write, bjetTrigMatch, sys);
    }

    return StatusCode::SUCCESS;
  }


  void BJetTriggerDecoratorAlg::checkDiBJetMatchingTriggers
  (int year, const xAOD::EventInfo* eventInfo,
   const runBoolReadDecoMap& runBoolDecos, const trigReadDecoMap& triggerdecos,
   const xAOD::JetContainer* jets,
   const bjetTrigMatchWriteDeco& bjet_trigMatchDeco,
   const bjetReadTrigMatchMap& bjetTrigMatch,
   const CP::SystematicSet& sys) const {

    std::vector<std::string> dib_paths;
    getDiBJetTriggers(year, eventInfo, runBoolDecos, dib_paths);

    for(const auto& trig : dib_paths){
      bool pass = triggerdecos.at(trig)(*eventInfo);
      if (pass){
        for (const xAOD::Jet *jet : *jets){
          if (static_cast<bool>(bjetTrigMatch.at(trig).get(*jet, sys))) {
            bjet_trigMatchDeco.set(*jet, true, sys);
          }
        }
      }
    }
  }
}