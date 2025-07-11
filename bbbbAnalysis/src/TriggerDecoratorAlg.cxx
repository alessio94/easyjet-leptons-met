/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TriggerDecoratorAlg.h"

namespace HH4B
{
  TriggerDecoratorAlg::TriggerDecoratorAlg(const std::string &name,
					   ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator) { }

  StatusCode TriggerDecoratorAlg ::initialize()
  {
    ATH_CHECK(m_eventInfoKey.initialize());

    m_yearKey = "EventInfo.dataTakingYear";
    ATH_CHECK(m_yearKey.initialize());

    ATH_CHECK (m_jetsKey.initialize());

    // make trigger decorators
    for (const auto& trig : m_triggers){

      // CP alg should convert trigger names
      std::string modifiedTrigName = trig;
      std::replace(modifiedTrigName.begin(), modifiedTrigName.end(), '-', '_');
      std::replace(modifiedTrigName.begin(), modifiedTrigName.end(), '.', 'p');

      std::string triggerDecorName = "trigPassed_"+modifiedTrigName;
      SG::ReadDecorHandleKey< xAOD::EventInfo > triggerDecorKey("EventInfo." + triggerDecorName);

      m_triggerdecoKeys.emplace(trig, triggerDecorKey);
      ATH_CHECK(m_triggerdecoKeys.at(trig).initialize());
    }

    for (const auto& [channel, name] : m_triggerChannels){
      SG::WriteDecorHandleKey<xAOD::EventInfo> deco;
      deco = "EventInfo.pass_trigger_"+name;
      m_pass_DecorKey.emplace(channel, deco);
      ATH_CHECK(m_pass_DecorKey.at(channel).initialize());
    }

    ATH_CHECK(m_bucketDecoratorKey.initialize());
    
    return StatusCode::SUCCESS;
  }

  StatusCode TriggerDecoratorAlg::execute(const EventContext& ctx) const
  {
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey,ctx);
    ATH_CHECK (eventInfo.isValid());

    SG::ReadHandle<xAOD::JetContainer> jets(m_jetsKey,ctx);
    ATH_CHECK (jets.isValid());

    SG::WriteDecorHandle<xAOD::EventInfo, int> m_bucketDecorator(m_bucketDecoratorKey, ctx);

    trigReadDecoMap triggerdecos;
    for (const auto& [name, key] : m_triggerdecoKeys){
      triggerdecos.emplace(name, key);
    }

    passWriteDecoMap pass_decos;
    for (const auto& [channel, key] : m_pass_DecorKey){
      pass_decos.emplace(channel, key);
      pass_decos.at(channel)(*eventInfo) = false;
    }

    SG::ReadDecorHandle<xAOD::EventInfo, unsigned int> year(m_yearKey);

    if (year(*eventInfo) < 2022) {
        const auto& SINGbTrigPaths = getTriggerPaths(m_triggerMap, year(*eventInfo), HH4B::SINGB);
        const auto& diBsingJTrigPaths = getTriggerPaths(m_triggerMap, year(*eventInfo), HH4B::DIB_SINGJ);
        const auto& DIbHTTrigPaths = getTriggerPaths(m_triggerMap, year(*eventInfo), HH4B::DIB_HT);
        const auto& diBdiJTrigPaths = getTriggerPaths(m_triggerMap, year(*eventInfo), HH4B::DIB_DIJ);

        if (!SINGbTrigPaths.empty()) evaluateTriggerCuts(eventInfo.cptr(), SINGbTrigPaths, triggerdecos, pass_decos, HH4B::SINGB);
        if (!diBsingJTrigPaths.empty()) evaluateTriggerCuts(eventInfo.cptr(), diBsingJTrigPaths, triggerdecos, pass_decos, HH4B::DIB_SINGJ);
        if (!DIbHTTrigPaths.empty()) evaluateTriggerCuts(eventInfo.cptr(), DIbHTTrigPaths, triggerdecos, pass_decos, HH4B::DIB_HT);
        if (!diBdiJTrigPaths.empty()) evaluateTriggerCuts(eventInfo.cptr(), diBdiJTrigPaths, triggerdecos, pass_decos, HH4B::DIB_DIJ);

    } else if (year(*eventInfo) == 2022) {

        const auto& J80TrigPaths = getTriggerPaths(m_triggerMap, year(*eventInfo), HH4B::J80);

        if (!J80TrigPaths.empty()) evaluateTriggerCuts(eventInfo.cptr(), J80TrigPaths, triggerdecos, pass_decos, HH4B::J80);

    } else if (year(*eventInfo) == 2023) {

        const auto& J75TrigPaths = getTriggerPaths(m_triggerMap, year(*eventInfo), HH4B::J75);
        const auto& J80TrigPaths = getTriggerPaths(m_triggerMap, year(*eventInfo), HH4B::J80);

        if (!J75TrigPaths.empty()) evaluateTriggerCuts(eventInfo.cptr(), J75TrigPaths, triggerdecos, pass_decos, HH4B::J75);
        if (!J80TrigPaths.empty()) evaluateTriggerCuts(eventInfo.cptr(), J80TrigPaths, triggerdecos, pass_decos, HH4B::J80);
    }    
    

    if (jets->size() < 2) return StatusCode::SUCCESS;  

    evaluateTriggerBuckets(eventInfo.cptr(), year, jets, pass_decos, m_bucketDecorator);


    return StatusCode::SUCCESS;
  }

  void TriggerDecoratorAlg::evaluateTriggerBuckets(const xAOD::EventInfo* eventInfo, 
                                                    const SG::ReadDecorHandle<xAOD::EventInfo, unsigned int>& year, 
                                                    SG::ReadHandle<xAOD::JetContainer> jets, 
                                                    passWriteDecoMap& pass_decos, 
                                                    SG::WriteDecorHandle<xAOD::EventInfo, int>& m_bucketDecorator) const {

    std::vector<double> jetPts;

    if (year(*eventInfo) < 2022){

        for (const xAOD::Jet* jet : *jets)
        {
            jetPts.push_back(jet->jetP4("NoBJetCalibMomentum").Pt());
        }
        std::sort(jetPts.rbegin(), jetPts.rend());

        // Ensure the jets list is padded to at least 3 elements
        const double PAD_VALUE = -999.0;
        while (jetPts.size() < 3) {
            jetPts.push_back(PAD_VALUE);
        }

        // Calculate leading and third pT
        double lead_pt = jetPts[0] * 1e-3;
        double third_pt = jetPts[2] * 1e-3;

        // Apply kinematic cuts
        bool b1_mask = (lead_pt > 170.0) && (third_pt > 70.0);

        bool bucket1 = b1_mask && pass_decos.at(HH4B::DIB_SINGJ)(*eventInfo);
        bool bucket2 = !b1_mask && pass_decos.at(HH4B::DIB_DIJ)(*eventInfo);

        if (bucket1) {
            m_bucketDecorator(*eventInfo) = 1;
        } else if (bucket2) {
            m_bucketDecorator(*eventInfo) = 2;
        } else m_bucketDecorator(*eventInfo) = 0;
    } else m_bucketDecorator(*eventInfo) = 0;
  }

  void TriggerDecoratorAlg::evaluateTriggerCuts(const xAOD::EventInfo* eventInfo, const std::vector<std::string> &Triggers,
                                                  const trigReadDecoMap& triggerdecos, passWriteDecoMap& pass_decos, 
                                                  HH4B::TriggerChannel flag) const {

    for (const std::string &trigger : Triggers)
    {
      bool pass = false;
      auto it = triggerdecos.find(trigger);
      if (it != triggerdecos.end()) {
          pass = it->second(*eventInfo);
      }
      if (pass) {
        pass_decos.at(flag)(*eventInfo) = true;
        break;
      }
    }
  }

}
