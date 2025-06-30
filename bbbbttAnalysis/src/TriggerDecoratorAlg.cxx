/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TriggerDecoratorAlg.h"
#include "TriggerUtils.h"


namespace HHHBBBBTT
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
      if(channel==HHHBBBBTT::trigMatch_Tau35 || channel==HHHBBBBTT::trigMatch_Tau25) continue;
      SG::WriteDecorHandleKey<xAOD::EventInfo> deco;
      deco = "EventInfo.pass_trigger_"+name;
      m_pass_DecorKey.emplace(channel, deco);
      ATH_CHECK(m_pass_DecorKey.at(channel).initialize());
    }

    SG::WriteDecorHandleKey<xAOD::MuonContainer> mu_SLT;
    mu_SLT = m_muonsKey.key() + ".trigMatch_SLT";
    m_mu_trigMatch_DecorKey.emplace(HHHBBBBTT::SLT, mu_SLT);
    SG::WriteDecorHandleKey<xAOD::MuonContainer> mu_LTT;
    mu_LTT = m_muonsKey.key() + ".trigMatch_LTT";
    m_mu_trigMatch_DecorKey.emplace(HHHBBBBTT::LTT, mu_LTT);
    SG::WriteDecorHandleKey<xAOD::MuonContainer> mu_MTT_2016;
    mu_MTT_2016 = m_muonsKey.key() + ".trigMatch_MTT_2016";
    m_mu_trigMatch_DecorKey.emplace(HHHBBBBTT::MTT_2016, mu_MTT_2016);
    SG::WriteDecorHandleKey<xAOD::MuonContainer> mu_MTT_high;
    mu_MTT_high = m_muonsKey.key() + ".trigMatch_MTT_high";
    m_mu_trigMatch_DecorKey.emplace(HHHBBBBTT::MTT_high, mu_MTT_high);
    SG::WriteDecorHandleKey<xAOD::MuonContainer> mu_MTT_low;
    mu_MTT_low = m_muonsKey.key() + ".trigMatch_MTT_low";
    m_mu_trigMatch_DecorKey.emplace(HHHBBBBTT::MTT_low, mu_MTT_low);
    
    for (const auto& [channel, key] : m_mu_trigMatch_DecorKey){
      ATH_CHECK(m_mu_trigMatch_DecorKey.at(channel).initialize());
    }

    SG::WriteDecorHandleKey<xAOD::ElectronContainer> ele_SLT;
    ele_SLT = m_elesKey.key() + ".trigMatch_SLT";
    m_ele_trigMatch_DecorKey.emplace(HHHBBBBTT::SLT, ele_SLT);
    SG::WriteDecorHandleKey<xAOD::ElectronContainer> ele_LTT;
    ele_LTT = m_elesKey.key() + ".trigMatch_LTT";
    m_ele_trigMatch_DecorKey.emplace(HHHBBBBTT::LTT, ele_LTT);
    SG::WriteDecorHandleKey<xAOD::ElectronContainer> ele_ETT;
    ele_ETT = m_elesKey.key() + ".trigMatch_ETT";
    m_ele_trigMatch_DecorKey.emplace(HHHBBBBTT::ETT, ele_ETT);
    SG::WriteDecorHandleKey<xAOD::ElectronContainer> ele_ETT_4J12;
    ele_ETT_4J12 = m_elesKey.key() + ".trigMatch_ETT_4J12";
    m_ele_trigMatch_DecorKey.emplace(HHHBBBBTT::ETT_4J12, ele_ETT_4J12);

    for (const auto& [channel, key] : m_ele_trigMatch_DecorKey){
      ATH_CHECK(m_ele_trigMatch_DecorKey.at(channel).initialize());
    }

    for (const auto& [channel, name] : m_triggerChannels){
      if(channel==HHHBBBBTT::SLT) continue;
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
    
    checkDiBJetTriggers(year(*eventInfo), eventInfo.cptr(),
		       runBoolDecos, triggerdecos, pass_decos);
		
    checkBjetTauTriggers(year(*eventInfo), eventInfo.cptr(),
		       runBoolDecos, triggerdecos, pass_decos);

    return StatusCode::SUCCESS;
  }

  void TriggerDecoratorAlg::checkSingleMuTriggers
  (int year, const xAOD::EventInfo* eventInfo,
   const runBoolReadDecoMap& runBoolDecos, const trigReadDecoMap& triggerdecos,
   passWriteDecoMap& pass_decos,
   const xAOD::MuonContainer* muons,
   muTrigMatchWriteDecoMap& mu_trigMatchDecos) const{

    std::vector<std::string> single_mu_paths;
    getSingleMuTriggers(year, eventInfo, runBoolDecos, single_mu_paths);

    bool trigPassed_SMT = false;

    for(const auto& trig : single_mu_paths){
      bool pass = triggerdecos.at("trigPassed_"+trig)(*eventInfo);
      trigPassed_SMT |= pass;
      if(pass){
	for(const xAOD::Muon* mu : *muons){
	  bool match = m_matchingTool->match(*mu, trig);
	  mu_trigMatchDecos.at(HHHBBBBTT::SLT)(*mu) |= match;
	}
      }
    }

    pass_decos.at(HHHBBBBTT::SLT)(*eventInfo) |= trigPassed_SMT;

  }

  void TriggerDecoratorAlg::checkSingleEleTriggers
  (int year, const xAOD::EventInfo* eventInfo,
   const runBoolReadDecoMap& runBoolDecos, const trigReadDecoMap& triggerdecos,
   passWriteDecoMap& pass_decos,
   const xAOD::ElectronContainer* electrons,
   eleTrigMatchWriteDecoMap& ele_trigMatchDecos) const{

    std::vector<std::string> single_ele_paths;
    getSingleEleTriggers(year, eventInfo, runBoolDecos, single_ele_paths);

    bool trigPassed_SET = false;
    
    for(const auto& trig : single_ele_paths){
      bool pass = triggerdecos.at("trigPassed_"+trig)(*eventInfo);
      trigPassed_SET |= pass;
      if(pass){
	for(const xAOD::Electron* ele : *electrons){
	  bool match = m_matchingTool->match(*ele, trig);
	  ele_trigMatchDecos.at(HHHBBBBTT::SLT)(*ele) |= match;
	}
      }
    }

    pass_decos.at(HHHBBBBTT::SLT)(*eventInfo) |= trigPassed_SET;
    
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
    getMuTauTriggers(year, eventInfo, runBoolDecos,
		     mu_tau_paths_2016, mu_tau_paths_low, mu_tau_paths_high);

    std::unordered_map<HHHBBBBTT::TriggerChannel, std::vector<std::string>> mapPaths;
    mapPaths.emplace(HHHBBBBTT::MTT_2016, mu_tau_paths_2016);
    mapPaths.emplace(HHHBBBBTT::MTT_high, mu_tau_paths_high);
    mapPaths.emplace(HHHBBBBTT::MTT_low, mu_tau_paths_low);

    std::unordered_map<HHHBBBBTT::TriggerChannel, bool> mapDecisions;
    mapDecisions.emplace(HHHBBBBTT::MTT_2016, false);
    mapDecisions.emplace(HHHBBBBTT::MTT_high, false);
    mapDecisions.emplace(HHHBBBBTT::MTT_low, false);

    for(const auto& [channel, paths] : mapPaths){
      for(const auto& trig: paths){
	bool pass = triggerdecos.at("trigPassed_"+trig)(*eventInfo);
	mapDecisions.at(channel) |= pass;
	if(pass){
	  for(const xAOD::Muon* mu : *muons){
	    bool match = m_matchingTool->match(*mu, trig);
	    mu_trigMatchDecos.at(channel)(*mu) |= match;
	    mu_trigMatchDecos.at(HHHBBBBTT::LTT)(*mu) |= match;
	  }
	  for(const xAOD::TauJet* tau : *taus){
	    bool match = m_matchingTool->match(*tau, trig, 0.2);
	    tau_trigMatchDecos.at(channel)(*tau) |= match;
	    tau_trigMatchDecos.at(HHHBBBBTT::LTT)(*tau) |= match;
	  }
	}
      }

      pass_decos.at(channel)(*eventInfo) |= mapDecisions.at(channel);
      pass_decos.at(HHHBBBBTT::LTT)(*eventInfo) |= mapDecisions.at(channel);
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
    getEleTauTriggers(year, eventInfo, runBoolDecos,
		      ele_tau_paths, ele_tau_paths_4J12);

    std::unordered_map<HHHBBBBTT::TriggerChannel, std::vector<std::string>> mapPaths;
    mapPaths.emplace(HHHBBBBTT::ETT, ele_tau_paths);
    mapPaths.emplace(HHHBBBBTT::ETT_4J12, ele_tau_paths_4J12);

    std::unordered_map<HHHBBBBTT::TriggerChannel, bool> mapDecisions;
    mapDecisions.emplace(HHHBBBBTT::ETT, false);
    mapDecisions.emplace(HHHBBBBTT::ETT_4J12, false);

    for(const auto& [channel, paths] : mapPaths){
      for(const auto& trig : paths){
	bool pass = triggerdecos.at("trigPassed_"+trig)(*eventInfo);
	mapDecisions.at(channel) |= pass;
	if(pass){
	  for(const xAOD::Electron* ele : *electrons){
	    bool match = m_matchingTool->match(*ele, trig);
	    ele_trigMatchDecos.at(channel)(*ele) |= match;
	    ele_trigMatchDecos.at(HHHBBBBTT::LTT)(*ele) |= match;
	  }
	  for(const xAOD::TauJet* tau : *taus){
	    bool match = m_matchingTool->match(*tau, trig, 0.2);
	    tau_trigMatchDecos.at(channel)(*tau) |= match;
	    tau_trigMatchDecos.at(HHHBBBBTT::LTT)(*tau) |= match;
	  }
	}
      }

      pass_decos.at(channel)(*eventInfo) |= mapDecisions.at(channel);
      pass_decos.at(HHHBBBBTT::LTT)(*eventInfo) |= mapDecisions.at(channel);
    }

  }


  void TriggerDecoratorAlg::checkSingleTauTriggers
  (int year, const xAOD::EventInfo* eventInfo,
   const runBoolReadDecoMap& runBoolDecos, const trigReadDecoMap& triggerdecos,
   passWriteDecoMap& pass_decos,
   const xAOD::TauJetContainer* taus,
   tauTrigMatchWriteDecoMap& tau_trigMatchDecos) const {

    std::vector<std::string> single_tau_paths;
    getSingleTauTriggers(year, eventInfo, runBoolDecos, single_tau_paths);

    bool trigPassed_STT = false;
    
    for(const auto& trig : single_tau_paths){
      bool pass = triggerdecos.at("trigPassed_"+trig)(*eventInfo);
      trigPassed_STT |= pass;
      if(pass){
	for(const xAOD::TauJet* tau : *taus){
	  bool match = m_matchingTool->match(*tau, trig, 0.2);
	  if(!m_useDiTauTrigMatch) match = true;
	  tau_trigMatchDecos.at(HHHBBBBTT::STT)(*tau) |= match;
	}
      }
    }

    pass_decos.at(HHHBBBBTT::STT)(*eventInfo) |= trigPassed_STT;
     
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
    std::vector<std::string> ditau_paths_L1Topo_delayed;
    std::vector<std::string> ditau_paths_4J12_delayed;
    std::vector<std::string> tau35_match_paths;
    std::vector<std::string> tau25_match_paths;

    getDiTauTriggers(year, eventInfo, runBoolDecos,
		     ditau_paths_2016, ditau_paths_L1Topo, ditau_paths_4J12,
		     ditau_paths_L1Topo_delayed, ditau_paths_4J12_delayed,
		     tau35_match_paths, tau25_match_paths);

    std::unordered_map<HHHBBBBTT::TriggerChannel, std::vector<std::string>> mapPaths;
    mapPaths.emplace(HHHBBBBTT::DTT_2016, ditau_paths_2016);
    mapPaths.emplace(HHHBBBBTT::DTT_4J12, ditau_paths_4J12);
    mapPaths.emplace(HHHBBBBTT::DTT_L1Topo, ditau_paths_L1Topo);
    mapPaths.emplace(HHHBBBBTT::DTT_4J12_delayed, ditau_paths_4J12_delayed);
    mapPaths.emplace(HHHBBBBTT::DTT_L1Topo_delayed, ditau_paths_L1Topo_delayed);

    std::unordered_map<HHHBBBBTT::TriggerChannel, bool> mapDecisions;
    mapDecisions.emplace(HHHBBBBTT::DTT_2016, false);
    mapDecisions.emplace(HHHBBBBTT::DTT_4J12, false);
    mapDecisions.emplace(HHHBBBBTT::DTT_L1Topo, false);
    mapDecisions.emplace(HHHBBBBTT::DTT_4J12_delayed, false);
    mapDecisions.emplace(HHHBBBBTT::DTT_L1Topo_delayed, false);

    for(const auto& [channel, paths] : mapPaths){
      for(const auto& trig : paths){
	bool pass = triggerdecos.at("trigPassed_"+trig)(*eventInfo);
	mapDecisions.at(channel) |= pass;
	if(pass){
	  // Naming altered for matching
	  // Run2 4J12 trigger named with '4J12.0ETA' -> need altering, Run3 4J12 already named with "4J12p0ETA" -> no altering
	  std::string trig2 = trig;
	  if(channel==HHHBBBBTT::DTT_4J12)
	    trig2 = std::regex_replace(trig, std::regex("4J12p0ETA23"),
				      "4J12_0ETA23");
	  else if(channel==HHHBBBBTT::DTT_L1Topo || channel==HHHBBBBTT::DTT_L1Topo_delayed) {
	    if (year >= 2024) {
	      trig2 = std::regex_replace(trig,
				         std::regex("DR_eTAU30eTAU20_jJ55"),
				         "DR-eTAU30eTAU20-jJ55");
	    }
	    else {
	      trig2 = std::regex_replace(trig,
				         std::regex("L1DR_TAU20ITAU12I_J25"),
				         "L1DR-TAU20ITAU12I-J25");
	    }
	  }
	  for(const xAOD::TauJet* tau : *taus){
	    bool match = m_matchingTool->match(*tau, trig2, 0.2);
	    tau_trigMatchDecos.at(channel)(*tau) |= match;
	    tau_trigMatchDecos.at(HHHBBBBTT::DTT)(*tau) |= match;
	  }
	}
      }

      pass_decos.at(channel)(*eventInfo) |= mapDecisions.at(channel);
      pass_decos.at(HHHBBBBTT::DTT)(*eventInfo) |= mapDecisions.at(channel);
    }

    // Save specifically tau35 + tau25 independent trigger matching
    std::unordered_map<HHHBBBBTT::TriggerChannel, std::vector<std::string>> mapMatchPaths;
    mapMatchPaths.emplace(HHHBBBBTT::trigMatch_Tau35, tau35_match_paths);
    mapMatchPaths.emplace(HHHBBBBTT::trigMatch_Tau25, tau25_match_paths);

    for(const auto& [channel, paths] : mapMatchPaths){
      for(const auto& trig : paths){
        bool pass = triggerdecos.at("trigPassed_"+trig)(*eventInfo);
        if(!pass) continue;
        for(const xAOD::TauJet* tau : *taus){
          bool match = m_matchingTool->match(*tau, trig, 0.2);
          tau_trigMatchDecos.at(channel)(*tau) |= match;
        }
      }
    }

  }


void TriggerDecoratorAlg::checkDiBJetTriggers
  (int year, const xAOD::EventInfo* eventInfo,
   const runBoolReadDecoMap& runBoolDecos, const trigReadDecoMap& triggerdecos,
   passWriteDecoMap& pass_decos) const {

    std::vector<std::string> dib_paths;
    getDiBJetTriggers(year, eventInfo, runBoolDecos, dib_paths);

    std::unordered_map<HHHBBBBTT::TriggerChannel, std::vector<std::string>> mapPaths;
    mapPaths.emplace(HHHBBBBTT::DBT, dib_paths);

    std::unordered_map<HHHBBBBTT::TriggerChannel, bool> mapDecisions;
    mapDecisions.emplace(HHHBBBBTT::DBT, false);

    for(const auto& [channel, paths] : mapPaths){
      for(const auto& trig : paths){
	bool pass = triggerdecos.at("trigPassed_"+trig)(*eventInfo);
	mapDecisions.at(channel) |= pass;
	if(pass){
    //TO DO: implement b-jet matching
	}
      }

      pass_decos.at(channel)(*eventInfo) |= mapDecisions.at(channel);
      pass_decos.at(HHHBBBBTT::DBT)(*eventInfo) |= mapDecisions.at(channel);
    }
  }

void TriggerDecoratorAlg::checkBjetTauTriggers
  (int year, const xAOD::EventInfo* eventInfo,
   const runBoolReadDecoMap& runBoolDecos, const trigReadDecoMap& triggerdecos,
   passWriteDecoMap& pass_decos) const {

    std::vector<std::string> btau_paths;
    getBjetTauTriggers(year, eventInfo, runBoolDecos, btau_paths);

    std::unordered_map<HHHBBBBTT::TriggerChannel, std::vector<std::string>> mapPaths;
    mapPaths.emplace(HHHBBBBTT::BTT, btau_paths);

    std::unordered_map<HHHBBBBTT::TriggerChannel, bool> mapDecisions;
    mapDecisions.emplace(HHHBBBBTT::BTT, false);

    for(const auto& [channel, paths] : mapPaths){
      for(const auto& trig : paths){
	bool pass = triggerdecos.at("trigPassed_"+trig)(*eventInfo);
	mapDecisions.at(channel) |= pass;
	if(pass){
	//TO DO: implement trigger matching
	}
      }
      pass_decos.at(channel)(*eventInfo) |= mapDecisions.at(channel);
      pass_decos.at(HHHBBBBTT::BTT)(*eventInfo) |= mapDecisions.at(channel);
    }
  }

}


