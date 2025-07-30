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


    // Initialization for deriving jet Trigger SF
    if( m_doL1SF ){
      m_triggerSFTypeList.emplace_back( HH4B::L1, HH4B::SF_NOSYS);
      m_triggerSFTypeList.emplace_back( HH4B::L1, HH4B::SF_STAT);
      m_triggerSFTypeList.emplace_back( HH4B::L1, HH4B::SF_SYST);
    } 
    if( m_doHLTSF ){
      m_triggerSFTypeList.emplace_back( HH4B::HLT, HH4B::SF_NOSYS);
      m_triggerSFTypeList.emplace_back( HH4B::HLT, HH4B::SF_STAT);
      m_triggerSFTypeList.emplace_back( HH4B::HLT, HH4B::SF_SYST);
    } 

    // make trigger decorators
    for (const auto& trig : m_triggers){

      // CP alg should convert trigger names
      std::string modifiedTrigName = trig;
      std::replace(modifiedTrigName.begin(), modifiedTrigName.end(), '-', '_');
      std::replace(modifiedTrigName.begin(), modifiedTrigName.end(), '.', 'p');

      std::string triggerDecorName = "trigPassed_"+modifiedTrigName;
      std::string triggerSFIndivDecorName = "trigSF_"+modifiedTrigName;
      SG::ReadDecorHandleKey< xAOD::EventInfo > triggerDecorKey("EventInfo." + triggerDecorName);

      m_triggerdecoKeys.emplace(trig, triggerDecorKey);
      ATH_CHECK(m_triggerdecoKeys.at(trig).initialize());


      for( const auto &[tt, ts] : m_triggerSFTypeList) {
        std::string key = "EventInfo."+triggerSFIndivDecorName;
        if( tt == HH4B::L1 && ts == HH4B::SF_NOSYS ) key += "_L1SF";
        else if( tt == HH4B::L1 && ts == HH4B::SF_STAT ) key += "_L1SF_stats__1up";
        else if( tt == HH4B::L1 && ts == HH4B::SF_SYST ) key += "_L1SF_syst__1up";
        else if( tt == HH4B::HLT && ts == HH4B::SF_NOSYS) key += "_HLTSF";
        else if( tt == HH4B::HLT && ts == HH4B::SF_STAT ) key += "_HLTSF_stats__1up";
        else if( tt == HH4B::HLT && ts == HH4B::SF_SYST ) key += "_HLTSF_syst__1up";
        else continue;
        m_triggerSFIndivDecoKeysCont[std::make_pair(tt,ts)].emplace(trig, SG::ReadDecorHandleKey< xAOD::EventInfo > (key));
        ATH_CHECK(m_triggerSFIndivDecoKeysCont[std::make_pair(tt,ts)].at(trig).initialize());
      }
    }

    for (const auto& [channel, name] : m_triggerChannels){
      SG::WriteDecorHandleKey<xAOD::EventInfo> deco;
      deco = "EventInfo.pass_trigger_"+name;
      m_pass_DecorKey.emplace(channel, deco);
      ATH_CHECK(m_pass_DecorKey.at(channel).initialize());
    }

    ATH_CHECK(m_bucketDecoratorKey.initialize());
    for(const auto &t : m_triggerSFTypeList){
      m_trigSFDecoratorKeys[t] = {"EventInfo."+getSFDecorKeyName(t)};
      ATH_CHECK(m_trigSFDecoratorKeys[t].initialize());
    }
    
    return StatusCode::SUCCESS;
  }

  StatusCode TriggerDecoratorAlg::execute(const EventContext& ctx) const
  {
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey,ctx);
    ATH_CHECK (eventInfo.isValid());

    SG::ReadHandle<xAOD::JetContainer> jets(m_jetsKey,ctx);
    ATH_CHECK (jets.isValid());

    SG::WriteDecorHandle<xAOD::EventInfo, int> bucketDecorator(m_bucketDecoratorKey, ctx);
    std::map<triggerSFType, SG::WriteDecorHandle<xAOD::EventInfo, float>> trigSFDecorators;
    for( const auto &t : m_triggerSFTypeList ){
      SG::WriteDecorHandle<xAOD::EventInfo, float> deco(m_trigSFDecoratorKeys.at(t), ctx);
      trigSFDecorators.emplace(t, deco);
    }

    trigReadDecoMap triggerdecos;
    for (const auto& [name, key] : m_triggerdecoKeys){
      triggerdecos.emplace(name, key);
    }


    triggerSFIndivReadDecoMap triggersf_decos;
    for( const auto &t : m_triggerSFTypeList) {
      for (const auto& [name, key] : m_triggerSFIndivDecoKeysCont.at(t)){
        triggersf_decos[t].emplace(name, key);
      }
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

    evaluateTriggerBuckets(eventInfo.cptr(), year, jets, triggersf_decos, pass_decos, bucketDecorator, trigSFDecorators);


    return StatusCode::SUCCESS;
  }

  void TriggerDecoratorAlg::evaluateTriggerBuckets(const xAOD::EventInfo* eventInfo, 
                                                    const SG::ReadDecorHandle<xAOD::EventInfo, unsigned int>& year, 
                                                    SG::ReadHandle<xAOD::JetContainer> jets, 
                                                    const triggerSFIndivReadDecoMap& triggersf_decos,
                                                    passWriteDecoMap& pass_decos, 
                                                    SG::WriteDecorHandle<xAOD::EventInfo, int>& bucketDecorator,
                                                    std::map<triggerSFType, SG::WriteDecorHandle<xAOD::EventInfo, float>>& trigSFDecorators
                                                    ) const {

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
            bucketDecorator(*eventInfo) = 1;
        } else if (bucket2) {
            bucketDecorator(*eventInfo) = 2;
        } else bucketDecorator(*eventInfo) = 0;
    } else {
      if( year(*eventInfo) == 2022 && pass_decos.at(HH4B::J80)(*eventInfo))
        bucketDecorator(*eventInfo) = 1;
      else if( year(*eventInfo) == 2023 && pass_decos.at(HH4B::J75)(*eventInfo))
        bucketDecorator(*eventInfo) = 1;
      else
        bucketDecorator(*eventInfo) = 0;
    }

    if (m_isMC) {
      // Derivation of trigger SF
      const auto& trigNameBucket = getTriggerPaths(m_triggerMap, year(*eventInfo), bucketDecorator(*eventInfo));
      if( trigNameBucket.size() > 0) {
        for( auto const &t : m_triggerSFTypeList)
          trigSFDecorators.at(t)(*eventInfo) = triggersf_decos.at(t).at(trigNameBucket.at(0))(*eventInfo);
      } else {
        // Failed in bucketing
        for( auto const &t : m_triggerSFTypeList) 
          trigSFDecorators.at(t)(*eventInfo) = 0.0;
      }
    }
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

  std::string TriggerDecoratorAlg::getSFDecorKeyName( const triggerSFType t) const
  {
    const static std::map<HH4B::TriggerType, std::string> typeToStr = {{HH4B::L1, "l1"}, {HH4B::HLT, "hlt"}};
    const static std::map<HH4B::TriggerSFSystType, std::string> sfsystypeToStr = {{HH4B::SF_NOSYS, "nosys"}, {HH4B::SF_STAT, "stat__1up"}, {HH4B::SF_SYST, "syst__1up"}};
    return "trigger_smallrjet_sf_"+typeToStr.at(t.first)+"_"+sfsystypeToStr.at(t.second);
  };

  const std::vector<std::string>& TriggerDecoratorAlg::getTriggerPaths(
      const triggerMap& trigger_map,
      int year,
      HH4B::TriggerChannel triggerType) const
  {
    static const std::vector<std::string> emptyVec;

    auto yearIt = trigger_map.find(year);
    if (yearIt == trigger_map.end()) return emptyVec;

    const auto& channel_map = yearIt->second;
    auto channelIt = channel_map.find(triggerType);
    if (channelIt == channel_map.end()) return emptyVec;

    return channelIt->second;
  }
  const std::vector<std::string>& TriggerDecoratorAlg::getTriggerPaths(
      const triggerMap& trigger_map,
      int year,
      int bucket) const
  {
    static const std::vector<std::string> emptyVec;

    auto yearIt = trigger_map.find(year);
    if (yearIt == trigger_map.end()) return emptyVec;

    const auto& channel_map = yearIt->second;

    const auto primaryTriggerMap = [](int y){
      if( y < 2022) return HH4B::DIB_SINGJ;
      else if( y == 2022 ) return HH4B::J80;
      else if( y == 2023 ) return HH4B::J75;
      return HH4B::SINGB;
    };

    HH4B::TriggerChannel triggerType = HH4B::SINGB;
    if( bucket == 1 ) triggerType = primaryTriggerMap(year);
    else if (bucket == 2) triggerType = HH4B::DIB_DIJ;

    auto channelIt = channel_map.find(triggerType);
    if (triggerType == HH4B::SINGB || channelIt == channel_map.end()) return emptyVec;

    return channelIt->second;
  }

}
