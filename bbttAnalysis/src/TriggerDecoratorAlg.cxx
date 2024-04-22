/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TriggerDecoratorAlg.h"


namespace HHBBTT
{
  TriggerDecoratorAlg::TriggerDecoratorAlg(const std::string &name,
					   ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator) { }

  StatusCode TriggerDecoratorAlg ::initialize()
  {
    ATH_CHECK (m_eventInfoKey.initialize());

    m_yearKey = "EventInfo.dataTakingYear";
    ATH_CHECK(m_yearKey.initialize());

    for(const auto& [runBool, name] : m_runBooleans) {
      SG::ReadDecorHandleKey<xAOD::EventInfo> deco;
      deco = "EventInfo." + name;
      m_runBooleans_key.emplace(runBool, deco);
      ATH_CHECK(m_runBooleans_key.at(runBool).initialize());
    }

    ATH_CHECK (m_muonsKey.initialize());
    ATH_CHECK (m_elesKey.initialize());
    ATH_CHECK (m_tausKey.initialize());

    ATH_CHECK (m_matchingTool.retrieve());

    // make trigger decorators
    for (const auto& trig : m_triggers){
      SG::ReadDecorHandleKey<xAOD::EventInfo> deco;
      deco = "EventInfo." + trig;
      m_triggerdecoKeys.emplace(trig, deco);
      ATH_CHECK(m_triggerdecoKeys.at(trig).initialize());
    }

    for (const auto& [channel, name] : m_triggerChannels){
      SG::WriteDecorHandleKey<xAOD::EventInfo> deco;
      deco = "EventInfo.pass_trigger_"+name;
      m_pass_DecorKey.emplace(channel, deco);
      ATH_CHECK(m_pass_DecorKey.at(channel).initialize());
    }

    SG::WriteDecorHandleKey<xAOD::MuonContainer> mu_SLT;
    mu_SLT = m_muonsKey.key() + ".trigMatch_SLT";
    m_mu_trigMatch_DecorKey.emplace(HHBBTT::SLT, mu_SLT);
    SG::WriteDecorHandleKey<xAOD::MuonContainer> mu_LTT;
    mu_LTT = m_muonsKey.key() + ".trigMatch_LTT";
    m_mu_trigMatch_DecorKey.emplace(HHBBTT::LTT, mu_LTT);
    SG::WriteDecorHandleKey<xAOD::MuonContainer> mu_MTT_2016;
    mu_MTT_2016 = m_muonsKey.key() + ".trigMatch_MTT_2016";
    m_mu_trigMatch_DecorKey.emplace(HHBBTT::MTT_2016, mu_MTT_2016);
    SG::WriteDecorHandleKey<xAOD::MuonContainer> mu_MTT_high;
    mu_MTT_high = m_muonsKey.key() + ".trigMatch_MTT_high";
    m_mu_trigMatch_DecorKey.emplace(HHBBTT::MTT_high, mu_MTT_high);
    SG::WriteDecorHandleKey<xAOD::MuonContainer> mu_MTT_low;
    mu_MTT_low = m_muonsKey.key() + ".trigMatch_MTT_low";
    m_mu_trigMatch_DecorKey.emplace(HHBBTT::MTT_low, mu_MTT_low);
    
    for (const auto& [channel, key] : m_mu_trigMatch_DecorKey){
      ATH_CHECK(m_mu_trigMatch_DecorKey.at(channel).initialize());
    }

    SG::WriteDecorHandleKey<xAOD::ElectronContainer> ele_SLT;
    ele_SLT = m_elesKey.key() + ".trigMatch_SLT";
    m_ele_trigMatch_DecorKey.emplace(HHBBTT::SLT, ele_SLT);
    SG::WriteDecorHandleKey<xAOD::ElectronContainer> ele_LTT;
    ele_LTT = m_elesKey.key() + ".trigMatch_LTT";
    m_ele_trigMatch_DecorKey.emplace(HHBBTT::LTT, ele_LTT);
    SG::WriteDecorHandleKey<xAOD::ElectronContainer> ele_ETT;
    ele_ETT = m_elesKey.key() + ".trigMatch_ETT";
    m_ele_trigMatch_DecorKey.emplace(HHBBTT::ETT, ele_ETT);
    SG::WriteDecorHandleKey<xAOD::ElectronContainer> ele_ETT_4J12;
    ele_ETT_4J12 = m_elesKey.key() + ".trigMatch_ETT_4J12";
    m_ele_trigMatch_DecorKey.emplace(HHBBTT::ETT_4J12, ele_ETT_4J12);

    for (const auto& [channel, key] : m_ele_trigMatch_DecorKey){
      ATH_CHECK(m_ele_trigMatch_DecorKey.at(channel).initialize());
    }

    for (const auto& [channel, name] : m_triggerChannels){
      if(channel==HHBBTT::SLT) continue;
      SG::WriteDecorHandleKey<xAOD::TauJetContainer> deco;
      deco = m_tausKey.key() + ".trigMatch_"+name;
      m_tau_trigMatch_DecorKey.emplace(channel, deco);
      ATH_CHECK(m_tau_trigMatch_DecorKey.at(channel).initialize());
    }
    
    return StatusCode::SUCCESS;
  }


  StatusCode TriggerDecoratorAlg::execute(const EventContext& ctx) const
  {

    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey,ctx);
    ATH_CHECK (eventInfo.isValid());

    SG::ReadHandle<xAOD::MuonContainer> muons(m_muonsKey,ctx);
    ATH_CHECK (muons.isValid());

    SG::ReadHandle<xAOD::ElectronContainer> electrons(m_elesKey,ctx);
    ATH_CHECK (electrons.isValid());
    
    SG::ReadHandle<xAOD::TauJetContainer> taus(m_tausKey,ctx);
    ATH_CHECK (taus.isValid());

    runBoolReadDecoMap runBoolDecos;
    for (const auto& [runBool, key] : m_runBooleans_key){
      runBoolDecos.emplace(runBool, key);
    }

    trigReadDecoMap triggerdecos;
    for (const auto& [name, key] : m_triggerdecoKeys){
      triggerdecos.emplace(name, key);
    }

    passWriteDecoMap pass_decos;
    for (const auto& [channel, key] : m_pass_DecorKey){
      pass_decos.emplace(channel, key);
      pass_decos.at(channel)(*eventInfo) = false;
    }
    
    muTrigMatchWriteDecoMap mu_trigMatchDecos;
    for (const auto& [channel, key] : m_mu_trigMatch_DecorKey){
      mu_trigMatchDecos.emplace(channel, key);
      for(const xAOD::Muon* mu : *muons){
	mu_trigMatchDecos.at(channel)(*mu) = false;
      }
    }

    eleTrigMatchWriteDecoMap ele_trigMatchDecos;
    for (const auto& [channel, key] : m_ele_trigMatch_DecorKey){
      ele_trigMatchDecos.emplace(channel, key);
      for(const xAOD::Electron* ele : *electrons){
	ele_trigMatchDecos.at(channel)(*ele) = false;
      }
    }

    tauTrigMatchWriteDecoMap tau_trigMatchDecos;
    for (const auto& [channel, key] : m_tau_trigMatch_DecorKey){
      tau_trigMatchDecos.emplace(channel, key);
      for(const xAOD::TauJet* tau : *taus){
	tau_trigMatchDecos.at(channel)(*tau) = false;
      }
    }

    SG::ReadDecorHandle<xAOD::EventInfo, unsigned int> year(m_yearKey);

    checkSingleMuTriggers(year(*eventInfo), eventInfo.cptr(),
			  runBoolDecos, triggerdecos, pass_decos,
			  muons.cptr(), mu_trigMatchDecos);

    checkSingleEleTriggers(year(*eventInfo), eventInfo.cptr(),
			   runBoolDecos, triggerdecos, pass_decos,
			   electrons.cptr(), ele_trigMatchDecos);

    checkMuTauTriggers(year(*eventInfo), eventInfo.cptr(),
		       runBoolDecos, triggerdecos, pass_decos,
		       muons.cptr(), mu_trigMatchDecos,
		       taus.cptr(), tau_trigMatchDecos);

    checkEleTauTriggers(year(*eventInfo), eventInfo.cptr(),
			runBoolDecos, triggerdecos, pass_decos,
			electrons.cptr(), ele_trigMatchDecos,
			taus.cptr(), tau_trigMatchDecos);

    checkSingleTauTriggers(year(*eventInfo), eventInfo.cptr(),
			   runBoolDecos, triggerdecos, pass_decos,
			   taus.cptr(), tau_trigMatchDecos);

    checkDiTauTriggers(year(*eventInfo), eventInfo.cptr(),
		       runBoolDecos, triggerdecos, pass_decos,
		       taus.cptr(), tau_trigMatchDecos);
    
    return StatusCode::SUCCESS;
  }

  void TriggerDecoratorAlg::checkSingleMuTriggers
  (int year, const xAOD::EventInfo* eventInfo,
   const runBoolReadDecoMap& runBoolDecos, const trigReadDecoMap& triggerdecos,
   passWriteDecoMap& pass_decos,
   const xAOD::MuonContainer* muons,
   muTrigMatchWriteDecoMap& mu_trigMatchDecos) const{

    std::vector<std::string> single_mu_paths;

    if(year==2015){
      single_mu_paths = {"HLT_mu20_iloose_L1MU15", "HLT_mu50"};
    }
    else if(2016<=year && year<=2018){
      single_mu_paths = {"HLT_mu26_ivarmedium", "HLT_mu50"};
    }
    else if(2022<=year && year<=2023 &&
	    !runBoolDecos.at(HHBBTT::is22_75bunches)(*eventInfo) &&
	    !runBoolDecos.at(HHBBTT::is23_75bunches)(*eventInfo) &&
	    !runBoolDecos.at(HHBBTT::is23_400bunches)(*eventInfo)){
      single_mu_paths = {
	"HLT_mu24_ivarmedium_L1MU14FCH", "HLT_mu50_L1MU14FCH",
	"HLT_mu60_0eta105_msonly_L1MU14FCH", "HLT_mu60_L1MU14FCH",
	"HLT_mu80_msonly_3layersEC_L1MU14FCH"
      };
    }

    bool trigPassed_SMT = false;

    for(const auto& trig : single_mu_paths){
      bool pass = triggerdecos.at("trigPassed_"+trig)(*eventInfo);
      trigPassed_SMT |= pass;
      if(pass){
	for(const xAOD::Muon* mu : *muons){
	  bool match = m_matchingTool->match(*mu, trig);
	  mu_trigMatchDecos.at(HHBBTT::SLT)(*mu) |= match;
	}
      }
    }

    pass_decos.at(HHBBTT::SLT)(*eventInfo) |= trigPassed_SMT;

  }

  void TriggerDecoratorAlg::checkSingleEleTriggers
  (int year, const xAOD::EventInfo* eventInfo,
   const runBoolReadDecoMap& runBoolDecos, const trigReadDecoMap& triggerdecos,
   passWriteDecoMap& pass_decos,
   const xAOD::ElectronContainer* electrons,
   eleTrigMatchWriteDecoMap& ele_trigMatchDecos) const{

    std::vector<std::string> single_ele_paths;
    
    if(year==2015){
      single_ele_paths = {
	"HLT_e24_lhmedium_L1EM20VH", "HLT_e60_lhmedium",
	"HLT_e120_lhloose"
      };
    }
    else if(2016<=year && year<=2018){
      single_ele_paths = {
	"HLT_e26_lhtight_nod0_ivarloose", "HLT_e60_lhmedium_nod0",
	"HLT_e140_lhloose_nod0"
      };
    }
    else if(runBoolDecos.at(HHBBTT::is22_75bunches)(*eventInfo)){
      single_ele_paths = {
	"HLT_e17_lhvloose_L1EM15VHI", "HLT_e20_lhvloose_L1EM15VH",
	"HLT_e250_etcut_L1EM22VHI"
      };
    }
    else if(year==2022){
      single_ele_paths = {
	"HLT_e26_lhtight_ivarloose_L1EM22VHI", "HLT_e60_lhmedium_L1EM22VHI",
	"HLT_e140_lhloose_L1EM22VHI", "HLT_e300_etcut_L1EM22VHI"
      };
    }
    else if(runBoolDecos.at(HHBBTT::is23_75bunches)(*eventInfo)){
      single_ele_paths = {
	"HLT_e26_lhtight_ivarloose_L1EM22VHI", "HLT_e60_lhmedium_L1EM22VHI",
	"HLT_e140_lhloose_L1EM22VHI", "HLT_e140_lhloose_noringer_L1EM22VHI",
	"HLT_e300_etcut_L1EM22VHI"
      };
    }
    else if(year==2023){
      single_ele_paths = {
	"HLT_e26_lhtight_ivarloose_L1eEM26M", "HLT_e60_lhmedium_L1eEM26M",
	"HLT_e140_lhloose_L1eEM26M", "HLT_e140_lhloose_noringer_L1eEM26M",
	"HLT_e300_etcut_L1eEM26M"
      };
    }

    bool trigPassed_SET = false;
    
    for(const auto& trig : single_ele_paths){
      bool pass = triggerdecos.at("trigPassed_"+trig)(*eventInfo);
      trigPassed_SET |= pass;
      if(pass){
	for(const xAOD::Electron* ele : *electrons){
	  bool match = m_matchingTool->match(*ele, trig);
	  ele_trigMatchDecos.at(HHBBTT::SLT)(*ele) |= match;
	}
      }
    }

    pass_decos.at(HHBBTT::SLT)(*eventInfo) |= trigPassed_SET;
    
  }

  void TriggerDecoratorAlg::checkMuTauTriggers
  (int year, const xAOD::EventInfo* eventInfo,
   const runBoolReadDecoMap& runBoolDecos, const trigReadDecoMap& triggerdecos,
   passWriteDecoMap& pass_decos,
   const xAOD::MuonContainer* muons,
   muTrigMatchWriteDecoMap& mu_trigMatchDecos,
   const xAOD::TauJetContainer* taus,
   tauTrigMatchWriteDecoMap& tau_trigMatchDecos) const {

    std::vector<std::string> mu_tau_paths_2016;
    std::vector<std::string> mu_tau_paths_low;
    std::vector<std::string> mu_tau_paths_high;

    if(year==2015 || runBoolDecos.at(HHBBTT::is16PeriodA)(*eventInfo)){
      mu_tau_paths_2016 = {"HLT_mu14_tau25_medium1_tracktwo"};
    }
    else if(runBoolDecos.at(HHBBTT::is16PeriodB_D3)(*eventInfo) ||
	    runBoolDecos.at(HHBBTT::is16PeriodD4_end)(*eventInfo)){
      mu_tau_paths_2016 = {"HLT_mu14_ivarloose_tau25_medium1_tracktwo"};
    }
    else if(runBoolDecos.at(HHBBTT::is17PeriodB1_B4)(*eventInfo) ||
	    runBoolDecos.at(HHBBTT::is17PeriodB5_B7)(*eventInfo) ||
	    runBoolDecos.at(HHBBTT::is17PeriodB8_end)(*eventInfo)){
      mu_tau_paths_low = {"HLT_mu14_ivarloose_tau25_medium1_tracktwo_L1MU10_TAU12IM_3J12"};
      mu_tau_paths_high = {"HLT_mu14_ivarloose_tau35_medium1_tracktwo"};
    }
    else if(runBoolDecos.at(HHBBTT::is18PeriodB_end)(*eventInfo)){
      mu_tau_paths_low = {"HLT_mu14_ivarloose_tau25_medium1_tracktwoEF_L1MU10_TAU12IM_3J12"};
      mu_tau_paths_high = {"HLT_mu14_ivarloose_tau35_medium1_tracktwoEF"};
      if(runBoolDecos.at(HHBBTT::is18PeriodK_end)(*eventInfo)){
        mu_tau_paths_low.push_back("HLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA_L1MU10_TAU12IM_3J12");
        mu_tau_paths_high.push_back("HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA");
      }
    }
    else if(year<=2022 || year<=2023){
      mu_tau_paths_low = {"HLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1MU8F_TAU12IM_3J12"};
      mu_tau_paths_high = {"HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA_03dRAB_L1MU8F_TAU20IM"};
    }

    std::unordered_map<HHBBTT::TriggerChannel, std::vector<std::string>> mapPaths;
    mapPaths.emplace(HHBBTT::MTT_2016, mu_tau_paths_2016);
    mapPaths.emplace(HHBBTT::MTT_high, mu_tau_paths_high);
    mapPaths.emplace(HHBBTT::MTT_low, mu_tau_paths_low);

    std::unordered_map<HHBBTT::TriggerChannel, bool> mapDecisions;
    mapDecisions.emplace(HHBBTT::MTT_2016, false);
    mapDecisions.emplace(HHBBTT::MTT_high, false);
    mapDecisions.emplace(HHBBTT::MTT_low, false);

    for(const auto& [channel, paths] : mapPaths){
      for(const auto& trig: paths){
	bool pass = triggerdecos.at("trigPassed_"+trig)(*eventInfo);
	mapDecisions.at(channel) |= pass;
	if(pass){
	  for(const xAOD::Muon* mu : *muons){
	    bool match = m_matchingTool->match(*mu, trig);
	    mu_trigMatchDecos.at(channel)(*mu) |= match;
	    mu_trigMatchDecos.at(HHBBTT::LTT)(*mu) |= match;
	  }
	  for(const xAOD::TauJet* tau : *taus){
	    bool match = m_matchingTool->match(*tau, trig, 0.2);
	    tau_trigMatchDecos.at(channel)(*tau) |= match;
	    tau_trigMatchDecos.at(HHBBTT::LTT)(*tau) |= match;
	  }
	}
      }

      pass_decos.at(channel)(*eventInfo) |= mapDecisions.at(channel);
      pass_decos.at(HHBBTT::LTT)(*eventInfo) |= mapDecisions.at(channel);
    }

  }

  void TriggerDecoratorAlg::checkEleTauTriggers
      (int year, const xAOD::EventInfo* eventInfo,
       const runBoolReadDecoMap& runBoolDecos, const trigReadDecoMap& triggerdecos,
       passWriteDecoMap& pass_decos,
       const xAOD::ElectronContainer* electrons,
       eleTrigMatchWriteDecoMap& ele_trigMatchDecos,
       const xAOD::TauJetContainer* taus,
       tauTrigMatchWriteDecoMap& tau_trigMatchDecos) const {

    std::vector<std::string> ele_tau_paths;
    std::vector<std::string> ele_tau_paths_4J12;
 
    if(year==2015 || runBoolDecos.at(HHBBTT::is16PeriodA)(*eventInfo)){
      ele_tau_paths = {"HLT_e17_lhmedium_nod0_tau25_medium1_tracktwo"};
    }
    else if(runBoolDecos.at(HHBBTT::is16PeriodB_D3)(*eventInfo) ||
	    runBoolDecos.at(HHBBTT::is16PeriodD4_end)(*eventInfo)){
      ele_tau_paths = {"HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwo"};
    }
    else if(runBoolDecos.at(HHBBTT::is17PeriodB1_B4)(*eventInfo) ||
	    runBoolDecos.at(HHBBTT::is17PeriodB5_B7)(*eventInfo) ||
	    runBoolDecos.at(HHBBTT::is17PeriodB8_end)(*eventInfo)){
      ele_tau_paths = {"HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwo"};
      ele_tau_paths_4J12 = {"HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwo_L1EM15VHI_2TAU12IM_4J12"};
    }
    else if(runBoolDecos.at(HHBBTT::is18PeriodB_end)(*eventInfo)){
      ele_tau_paths = {"HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwoEF"};
      ele_tau_paths_4J12 = {"HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwoEF_L1EM15VHI_2TAU12IM_4J12"};
      if(runBoolDecos.at(HHBBTT::is18PeriodK_end)(*eventInfo)){
        ele_tau_paths.push_back("HLT_e17_lhmedium_nod0_ivarloose_tau25_mediumRNN_tracktwoMVA");
        ele_tau_paths_4J12.push_back("HLT_e17_lhmedium_nod0_ivarloose_tau25_mediumRNN_tracktwoMVA_L1EM15VHI_2TAU12IM_4J12");
      }
    }
    else if(year==2022){
      ele_tau_paths = {"HLT_e24_lhmedium_ivarloose_tau20_mediumRNN_tracktwoMVA_03dRAB_L1EM22VHI"};
      ele_tau_paths_4J12 = {"HLT_e17_lhmedium_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1EM15VHI_2TAU12IM_4J12"};
    }
    else if(year==2023){
      ele_tau_paths = {"HLT_e24_lhmedium_ivarloose_tau20_mediumRNN_tracktwoMVA_03dRAB_L1eEM26M"};
      ele_tau_paths_4J12 = {"HLT_e17_lhmedium_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1EM15VHI_2TAU12IM_4J12"};
    }

    std::unordered_map<HHBBTT::TriggerChannel, std::vector<std::string>> mapPaths;
    mapPaths.emplace(HHBBTT::ETT, ele_tau_paths);
    mapPaths.emplace(HHBBTT::ETT_4J12, ele_tau_paths_4J12);

    std::unordered_map<HHBBTT::TriggerChannel, bool> mapDecisions;
    mapDecisions.emplace(HHBBTT::ETT, false);
    mapDecisions.emplace(HHBBTT::ETT_4J12, false);

    for(const auto& [channel, paths] : mapPaths){
      for(const auto& trig : paths){
	bool pass = triggerdecos.at("trigPassed_"+trig)(*eventInfo);
	mapDecisions.at(channel) |= pass;
	if(pass){
	  for(const xAOD::Electron* ele : *electrons){
	    bool match = m_matchingTool->match(*ele, trig);
	    ele_trigMatchDecos.at(channel)(*ele) |= match;
	    ele_trigMatchDecos.at(HHBBTT::LTT)(*ele) |= match;
	  }
	  for(const xAOD::TauJet* tau : *taus){
	    bool match = m_matchingTool->match(*tau, trig, 0.2);
	    tau_trigMatchDecos.at(channel)(*tau) |= match;
	    tau_trigMatchDecos.at(HHBBTT::LTT)(*tau) |= match;
	  }
	}
      }

      pass_decos.at(channel)(*eventInfo) |= mapDecisions.at(channel);
      pass_decos.at(HHBBTT::LTT)(*eventInfo) |= mapDecisions.at(channel);
    }

  }


  void TriggerDecoratorAlg::checkSingleTauTriggers
  (int year, const xAOD::EventInfo* eventInfo,
   const runBoolReadDecoMap& runBoolDecos, const trigReadDecoMap& triggerdecos,
   passWriteDecoMap& pass_decos,
   const xAOD::TauJetContainer* taus,
   tauTrigMatchWriteDecoMap& tau_trigMatchDecos) const {

    std::vector<std::string> single_tau_paths;

    if(year==2015 || runBoolDecos.at(HHBBTT::is16PeriodA)(*eventInfo)){
      single_tau_paths = {"HLT_tau80_medium1_tracktwo_L1TAU60"};
    }
    else if(runBoolDecos.at(HHBBTT::is16PeriodB_D3)(*eventInfo)){
      single_tau_paths = {"HLT_tau125_medium1_tracktwo"};
    }
    else if(runBoolDecos.at(HHBBTT::is16PeriodD4_end)(*eventInfo) ||
	    runBoolDecos.at(HHBBTT::is17PeriodB1_B4)(*eventInfo)){
      single_tau_paths = {"HLT_tau160_medium1_tracktwo"};
    }
    else if(runBoolDecos.at(HHBBTT::is17PeriodB5_B7)(*eventInfo) ||
	    runBoolDecos.at(HHBBTT::is17PeriodB8_end)(*eventInfo)){
      single_tau_paths = {"HLT_tau160_medium1_tracktwo_L1TAU100"};
    }
    else if(year==2018){
      single_tau_paths = {"HLT_tau160_medium1_tracktwoEF_L1TAU100"};
      if(runBoolDecos.at(HHBBTT::is18PeriodK_end)(*eventInfo)){
        single_tau_paths.push_back("HLT_tau160_mediumRNN_tracktwoMVA_L1TAU100");
      }
    }
    else if(year==2022){
      single_tau_paths = {"HLT_tau160_mediumRNN_tracktwoMVA_L1TAU100"};
    }
    else if(year==2023){
      single_tau_paths = {"HLT_tau160_mediumRNN_tracktwoMVA_L1TAU100"};
      if(runBoolDecos.at(HHBBTT::is23_first_2400bunches)(*eventInfo)){
        single_tau_paths = {"HLT_tau160_mediumRNN_tracktwoMVA_L1eTAU140"};
      }
    }

    bool trigPassed_STT = false;
    
    for(const auto& trig : single_tau_paths){
      bool pass = triggerdecos.at("trigPassed_"+trig)(*eventInfo);
      trigPassed_STT |= pass;
      if(pass){
	for(const xAOD::TauJet* tau : *taus){
	  bool match = m_matchingTool->match(*tau, trig, 0.2);
	  if(!m_useDiTauTrigMatch) match = true;
	  tau_trigMatchDecos.at(HHBBTT::STT)(*tau) |= match;
	}
      }
    }

    pass_decos.at(HHBBTT::STT)(*eventInfo) |= trigPassed_STT;
     
  }


  void TriggerDecoratorAlg::checkDiTauTriggers
  (int year, const xAOD::EventInfo* eventInfo,
   const runBoolReadDecoMap& runBoolDecos, const trigReadDecoMap& triggerdecos,
   passWriteDecoMap& pass_decos,
   const xAOD::TauJetContainer* taus,
   tauTrigMatchWriteDecoMap& tau_trigMatchDecos) const {

    std::vector<std::string> ditau_paths_2016;
    std::vector<std::string> ditau_paths_L1Topo;
    std::vector<std::string> ditau_paths_4J12;

    if(year==2015){
      ditau_paths_2016 = {"HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1TAU20IM_2TAU12IM"};
    }
    else if(2016<=year && year<=2017){
      if(runBoolDecos.at(HHBBTT::is16PeriodA)(*eventInfo) ||
	 runBoolDecos.at(HHBBTT::is16PeriodB_D3)(*eventInfo) ||
	 runBoolDecos.at(HHBBTT::is16PeriodD4_end)(*eventInfo)){
	ditau_paths_2016 = {"HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo"};
      }
      else if(runBoolDecos.at(HHBBTT::l1topo_disabled)(*eventInfo)){
	ditau_paths_2016 = {"HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo"};
      }

      if(year==2017){
        ditau_paths_4J12 = {"HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1TAU20IM_2TAU12IM_4J12"};
      }

      if(runBoolDecos.at(HHBBTT::is17PeriodB1_B4)(*eventInfo)){
	// For Period B1 to B4 in 2017, should use this trigger but go to L1Topo selection
        ditau_paths_L1Topo = {"HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo"};
      }
      else if(!runBoolDecos.at(HHBBTT::l1topo_disabled)(*eventInfo) &&
	      (runBoolDecos.at(HHBBTT::is17PeriodB5_B7)(*eventInfo) ||
	       runBoolDecos.at(HHBBTT::is17PeriodB8_end)(*eventInfo))){
        ditau_paths_L1Topo = {"HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1DR_TAU20ITAU12I_J25"};
      }
    }
    
    else if(year==2018){
      ditau_paths_L1Topo = {"HLT_tau35_medium1_tracktwoEF_tau25_medium1_tracktwoEF_L1DR_TAU20ITAU12I_J25"};
      ditau_paths_4J12 = {"HLT_tau35_medium1_tracktwoEF_tau25_medium1_tracktwoEF_L1TAU20IM_2TAU12IM_4J12p0ETA23"};
      if(runBoolDecos.at(HHBBTT::is18PeriodK_end)(*eventInfo)){
        ditau_paths_L1Topo.push_back("HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_L1DR_TAU20ITAU12I_J25");
        ditau_paths_4J12.push_back("HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_L1TAU20IM_2TAU12IM_4J12p0ETA23");
      }
    }
    
    else if(year>=2022){
      ditau_paths_L1Topo = {"HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1DR_TAU20ITAU12I_J25"};
      ditau_paths_4J12 = {"HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25"};
      if (runBoolDecos.at(HHBBTT::is23_first_2400bunches)(*eventInfo)){
        ditau_paths_L1Topo = {"HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1DR_TAU20ITAU12I_J25"};
        ditau_paths_4J12 = {"HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25"};
      }
    }
    

    std::unordered_map<HHBBTT::TriggerChannel, std::vector<std::string>> mapPaths;
    mapPaths.emplace(HHBBTT::DTT_2016, ditau_paths_2016);
    mapPaths.emplace(HHBBTT::DTT_4J12, ditau_paths_4J12);
    mapPaths.emplace(HHBBTT::DTT_L1Topo, ditau_paths_L1Topo);

    std::unordered_map<HHBBTT::TriggerChannel, bool> mapDecisions;
    mapDecisions.emplace(HHBBTT::DTT_2016, false);
    mapDecisions.emplace(HHBBTT::DTT_4J12, false);
    mapDecisions.emplace(HHBBTT::DTT_L1Topo, false);


    for(const auto& [channel, paths] : mapPaths){
      for(const auto& trig : paths){
	bool pass = triggerdecos.at("trigPassed_"+trig)(*eventInfo);
	mapDecisions.at(channel) |= pass;
	if(pass){
	  // Naming altered for matching
	  std::string trig2 = trig;
	  if(channel==HHBBTT::DTT_4J12)
	    trig2 = std::regex_replace(trig, std::regex("4J12p0ETA"),
				      "4J12_0ETA");
	  else if(channel==HHBBTT::DTT_L1Topo)
	    trig2 = std::regex_replace(trig,
				       std::regex("L1DR_TAU20ITAU12I_J25"),
				       "L1DR-TAU20ITAU12I-J25");
	  for(const xAOD::TauJet* tau : *taus){
	    bool match = m_matchingTool->match(*tau, trig2, 0.2);
	    if(!m_useDiTauTrigMatch) match = true;
	    tau_trigMatchDecos.at(channel)(*tau) |= match;
	    tau_trigMatchDecos.at(HHBBTT::DTT)(*tau) |= match;
	  }
	}
      }

      pass_decos.at(channel)(*eventInfo) |= mapDecisions.at(channel);
      pass_decos.at(HHBBTT::DTT)(*eventInfo) |= mapDecisions.at(channel);
    }

  }
}
