/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration   
*/

#include "TriggerDecoratorAlg.h"
#include "TriggerUtils.h"
#include <AthenaKernel/Units.h>

namespace VBSVV4q
{

  TriggerDecoratorAlg::TriggerDecoratorAlg(const std::string &name,
					   ISvcLocator *pSvcLocator)
      : EL::AnaAlgorithm(name, pSvcLocator){}

  StatusCode TriggerDecoratorAlg::initialize()
  {
    ATH_CHECK(m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK(m_year.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK(m_jetsHandle.initialize(m_systematicsList));

    ////////////////// Trigger ///////////////////
    
    // make trigger decorators
    for (auto trig : m_triggers){
      CP::SysReadDecorHandle<bool> deco {this, "trig"+trig, trig, "Name of trigger"};
      m_triggerdecos.emplace(trig, deco);
      ATH_CHECK(m_triggerdecos.at(trig).initialize(m_systematicsList, m_eventHandle));
    }

    // make trigger flags
    for (const auto& [channel, name] : m_triggerChannels) {
      m_trig_bools.emplace(channel, false);
      CP::SysWriteDecorHandle<bool> whandle{"pass_trigger_"+name+"_%SYS%", this};
      m_trig_branches.emplace(channel, whandle);
      ATH_CHECK(m_trig_branches.at(channel).initialize(m_systematicsList, m_eventHandle));
    }

    ////////////////////////////////

    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode TriggerDecoratorAlg::execute()
  {
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector()){
      // Retrive inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetsHandle.retrieve (jets, sys));

      for (const auto& [channel, name] : m_triggerChannels) m_trig_bools.at(channel) = false;

      // check if trigger is fired or not
      checkSingleJetTrigger(event, sys);

      // check trigger matching
      //for(auto jet : *jets )
      //  checkSingleJetTriggerMatching(event, jet, sys);

      for (auto& [channel, var] : m_trig_bools) {
        m_trig_branches.at(channel).set(*event, var, sys);
      }

    }
    return StatusCode::SUCCESS;
  }

  void TriggerDecoratorAlg::checkSingleJetTrigger
  (const xAOD::EventInfo *event, const CP::SystematicSet& sys)
  {

    // Check single jets triggers
    int year = m_year.get(*event, sys);
    std::vector<std::string> trigger_list = getSingleLargeRJetsTriggers(year);
    
    bool trigPassed_SJT = false;
    for(const auto& trig : trigger_list){
      bool pass = m_triggerdecos.at("trigPassed_" + trig).get(*event, sys);
      trigPassed_SJT |= pass;
    }

    if(trigPassed_SJT){
      m_trig_bools.at(VBSVV4q::SJT) = true;
    }

    return;
  }

  void TriggerDecoratorAlg::checkSingleJetTriggerMatching
  (const xAOD::EventInfo *event, const xAOD::Jet *jet, const CP::SystematicSet& sys)
  {

    // Check single jets triggers
    int year = m_year.get(*event, sys);
    std::vector<std::string> trigger_list = getSingleLargeRJetsTriggers(year);
    
    bool trigPassed_SJT = false;
    if(jet){
      for(const auto& trig : trigger_list){
        bool pass = m_triggerdecos.at("trigPassed_" + trig).get(*event, sys);
        if (pass){
          bool match = m_matchingTool->match(*jet, trig);
          trigPassed_SJT |= match;
        }
      }
    }

    if(trigPassed_SJT){
      m_trig_bools.at(VBSVV4q::SJTM) = true;
    }

    return;
  }

}