/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TriggerDecoratorAlg.h"
#include "TriggerUtils.h"


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
    ATH_CHECK (m_jetsKey.initialize());

    ATH_CHECK (m_matchingTool.retrieve());

    // make trigger decorators
    for (const auto& trig : m_triggers){
      // convert trigger name to a valid branch name
      std::string modifiedTrigName = trig;
      std::string to_remove = "trigPassed_";
      if (modifiedTrigName.find(to_remove) != std::string::npos) {
          modifiedTrigName.erase(modifiedTrigName.find(to_remove), to_remove.length());
      }
      SG::ReadDecorHandleKey<xAOD::EventInfo> deco;
      deco = "EventInfo." + trig;
      m_triggerdecoKeys.emplace(trig, deco);
      ATH_CHECK(m_triggerdecoKeys.at(trig).initialize());
      // initialize the read handle to read a list of matched thresholds
      m_L1ThresholdsDecorKey.emplace(
            trig, m_jetsKey.key() + ".match" + modifiedTrigName + "_L1thresholds");
      m_HLTThresholdsDecorKey.emplace(
            trig, m_jetsKey.key() + ".match" + modifiedTrigName + "_HLTthresholds");
      m_L1ETDecorKey.emplace(
            trig, m_jetsKey.key() + ".match" + modifiedTrigName + "_L1et");
      m_HLTPTDecorKey.emplace(
            trig, m_jetsKey.key() + ".match" + modifiedTrigName + "_HLTpt");
      ATH_CHECK(m_L1ThresholdsDecorKey.at(trig).initialize());
      ATH_CHECK(m_HLTThresholdsDecorKey.at(trig).initialize());
      ATH_CHECK(m_L1ETDecorKey.at(trig).initialize());
      ATH_CHECK(m_HLTPTDecorKey.at(trig).initialize());
    }

    for (const auto& [channel, name] : m_triggerChannels){
      if(channel==HHBBTT::trigMatch_Tau35 || channel==HHBBTT::trigMatch_Tau25) continue;
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

    for (const auto& [channel, name] : m_triggerChannels){
      if(name.find("DTT") || name.find("DBT")){
        SG::WriteDecorHandleKey<xAOD::JetContainer> deco;
        deco = m_jetsKey.key() + ".trigMatch_"+name;
        m_jet_trigMatch_DecorKey.emplace(channel, deco);
        ATH_CHECK(m_jet_trigMatch_DecorKey.at(channel).initialize());
        SG::WriteDecorHandleKey<xAOD::JetContainer> deco_th;
        deco_th = m_jetsKey.key() + ".trigMatch_"+name+"_threshold";
        m_jet_trigMatch_ThresholdKey.emplace(channel, deco_th);
        ATH_CHECK(m_jet_trigMatch_ThresholdKey.at(channel).initialize());
        SG::WriteDecorHandleKey<xAOD::JetContainer> deco_pt;
        deco_pt = m_jetsKey.key() + ".trigMatch_"+name+"_onlinept";
        m_jet_trigMatch_OnlinePtKey.emplace(channel, deco_pt);
        ATH_CHECK(m_jet_trigMatch_OnlinePtKey.at(channel).initialize());
      }
    }

    return StatusCode::SUCCESS;
  }


  StatusCode TriggerDecoratorAlg::execute(const EventContext& ctx) const
  {
    jetReadTrigMatchThresholdMap jetL1Thresholds;
    jetReadTrigMatchThresholdMap jetHLTThresholds;
    jetReadTrigMatchptMap jetL1ET;
    jetReadTrigMatchptMap jetHLTPT;
    for (auto &trig : m_triggers)
    {
      jetL1Thresholds.emplace(trig, m_L1ThresholdsDecorKey.at(trig));
      jetHLTThresholds.emplace(trig, m_HLTThresholdsDecorKey.at(trig));
      jetL1ET.emplace(trig, m_L1ETDecorKey.at(trig));
      jetHLTPT.emplace(trig, m_HLTPTDecorKey.at(trig));
    }

    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey,ctx);
    ATH_CHECK (eventInfo.isValid());

    SG::ReadHandle<xAOD::MuonContainer> muons(m_muonsKey,ctx);
    ATH_CHECK (muons.isValid());

    SG::ReadHandle<xAOD::ElectronContainer> electrons(m_elesKey,ctx);
    ATH_CHECK (electrons.isValid());
    
    SG::ReadHandle<xAOD::TauJetContainer> taus(m_tausKey,ctx);
    ATH_CHECK (taus.isValid());

    SG::ReadHandle<xAOD::JetContainer> jets(m_jetsKey,ctx);
    ATH_CHECK (jets.isValid());

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

    jetTrigMatchWriteDecoMap jet_trigMatchDecos;
    for (const auto& [channel, key] : m_jet_trigMatch_DecorKey){
      jet_trigMatchDecos.emplace(channel, key);
      for(const xAOD::Jet* jet : *jets){
	jet_trigMatchDecos.at(channel)(*jet) = false;
      }
    }

    jetTrigMatchThresholdMap jet_trigMatchThresholds;
    for (const auto& [channel, key] : m_jet_trigMatch_ThresholdKey){
      jet_trigMatchThresholds.emplace(channel, key);
      for(const xAOD::Jet* jet : *jets){
	jet_trigMatchThresholds.at(channel)(*jet) = std::vector<int>{-99};
      }
    }

    jetTrigMatchOnlinePtMap jet_trigMatchOnlinePt;
    for (const auto& [channel, key] : m_jet_trigMatch_OnlinePtKey){
      jet_trigMatchOnlinePt.emplace(channel, key);
      for(const xAOD::Jet* jet : *jets){
	jet_trigMatchOnlinePt.at(channel)(*jet) = -99.;
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
		       taus.cptr(), tau_trigMatchDecos,
		       jets.cptr(), jet_trigMatchDecos,
		       jet_trigMatchThresholds, jet_trigMatchOnlinePt,
           jetL1Thresholds, jetL1ET);
    
    checkDiBJetTriggers(year(*eventInfo), eventInfo.cptr(),
		       runBoolDecos, triggerdecos, pass_decos,
		       jets.cptr(), jet_trigMatchDecos,
		       jet_trigMatchThresholds, jet_trigMatchOnlinePt,
           jetL1Thresholds, jetHLTThresholds,
           jetL1ET, jetHLTPT);

    checkLargeRJetsTriggers(
      year(*eventInfo), 
      eventInfo.cptr(),
      triggerdecos,
      pass_decos);

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
    getSingleEleTriggers(year, eventInfo, runBoolDecos, single_ele_paths);

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
    getMuTauTriggers(year, eventInfo, runBoolDecos,
		     mu_tau_paths_2016, mu_tau_paths_low, mu_tau_paths_high);

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
    getEleTauTriggers(year, eventInfo, runBoolDecos,
		      ele_tau_paths, ele_tau_paths_4J12);

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
    getSingleTauTriggers(year, eventInfo, runBoolDecos, single_tau_paths);

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
   tauTrigMatchWriteDecoMap& tau_trigMatchDecos,
   const xAOD::JetContainer* jets,
   jetTrigMatchWriteDecoMap& jet_trigMatchDecos,
   jetTrigMatchThresholdMap& jet_trigMatchThresholds,
   jetTrigMatchOnlinePtMap& jet_trigMatchOnlinePt,
   jetReadTrigMatchThresholdMap& jetL1Thresholds,
   jetReadTrigMatchptMap& jetL1ET) const {

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

    std::unordered_map<HHBBTT::TriggerChannel, std::vector<std::string>> mapPaths;
    mapPaths.emplace(HHBBTT::DTT_2016, ditau_paths_2016);
    mapPaths.emplace(HHBBTT::DTT_4J12, ditau_paths_4J12);
    mapPaths.emplace(HHBBTT::DTT_L1Topo, ditau_paths_L1Topo);
    mapPaths.emplace(HHBBTT::DTT_4J12_delayed, ditau_paths_4J12_delayed);
    mapPaths.emplace(HHBBTT::DTT_L1Topo_delayed, ditau_paths_L1Topo_delayed);

    std::unordered_map<HHBBTT::TriggerChannel, bool> mapDecisions;
    mapDecisions.emplace(HHBBTT::DTT_2016, false);
    mapDecisions.emplace(HHBBTT::DTT_4J12, false);
    mapDecisions.emplace(HHBBTT::DTT_L1Topo, false);
    mapDecisions.emplace(HHBBTT::DTT_4J12_delayed, false);
    mapDecisions.emplace(HHBBTT::DTT_L1Topo_delayed, false);

    for(const auto& [channel, paths] : mapPaths){
      for(const auto& trig : paths){
	bool pass = triggerdecos.at("trigPassed_"+trig)(*eventInfo);
	mapDecisions.at(channel) |= pass;
	if(pass){
	  // Naming altered for matching
	  // Run2 4J12 trigger named with '4J12.0ETA' -> need altering, Run3 4J12 already named with "4J12p0ETA" -> no altering
	  std::string trig2 = trig;
	  if(channel==HHBBTT::DTT_4J12)
	    trig2 = std::regex_replace(trig, std::regex("4J12p0ETA23"),
				      "4J12_0ETA23");
	  else if(channel==HHBBTT::DTT_L1Topo || channel==HHBBTT::DTT_L1Topo_delayed)
	    trig2 = std::regex_replace(trig,
				       std::regex("L1DR_TAU20ITAU12I_J25"),
				       "L1DR-TAU20ITAU12I-J25");
	  for(const xAOD::TauJet* tau : *taus){
	    bool match = m_matchingTool->match(*tau, trig2, 0.2);
	    tau_trigMatchDecos.at(channel)(*tau) |= match;
	    tau_trigMatchDecos.at(HHBBTT::DTT)(*tau) |= match;
	  }
	  for (const xAOD::Jet *jet : *jets){
	    bool match = !jetL1Thresholds.at("trigPassed_"+trig)(*jet).empty();
	    jet_trigMatchDecos.at(channel)(*jet) |= match;
	    jet_trigMatchDecos.at(HHBBTT::DTT)(*jet) |= match;
	    jet_trigMatchThresholds.at(channel)(*jet) = jetL1Thresholds.at("trigPassed_"+trig)(*jet);
	    jet_trigMatchOnlinePt.at(channel)(*jet) = jetL1ET.at("trigPassed_"+trig)(*jet);
	  }
	}
      }

      pass_decos.at(channel)(*eventInfo) |= mapDecisions.at(channel);
      pass_decos.at(HHBBTT::DTT)(*eventInfo) |= mapDecisions.at(channel);
    }

    // Save specifically tau35 + tau25 independent trigger matching
    std::unordered_map<HHBBTT::TriggerChannel, std::vector<std::string>> mapMatchPaths;
    mapMatchPaths.emplace(HHBBTT::trigMatch_Tau35, tau35_match_paths);
    mapMatchPaths.emplace(HHBBTT::trigMatch_Tau25, tau25_match_paths);

    for(const auto& [channel, paths] : mapMatchPaths){
      for(const auto& trig : paths){
        // Not doing trigPassed checking since it does not work for tau35 + tau25 triggers for all Run2 data
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
   passWriteDecoMap& pass_decos,
   const xAOD::JetContainer* jets,
   jetTrigMatchWriteDecoMap& jet_trigMatchDecos,
   jetTrigMatchThresholdMap& jet_trigMatchThresholds,
   jetTrigMatchOnlinePtMap& jet_trigMatchOnlinePt,
   jetReadTrigMatchThresholdMap& jetL1Thresholds,
   jetReadTrigMatchThresholdMap& jetHLTThresholds,
   jetReadTrigMatchptMap& jetL1ET,
   jetReadTrigMatchptMap& jetHLTPT) const {

    std::vector<std::string> dib_paths;
    getDiBJetTriggers(year, eventInfo, runBoolDecos, dib_paths);

    std::unordered_map<HHBBTT::TriggerChannel, std::vector<std::string>> mapPaths;
    mapPaths.emplace(HHBBTT::DBT, dib_paths);

    std::unordered_map<HHBBTT::TriggerChannel, bool> mapDecisions;
    mapDecisions.emplace(HHBBTT::DBT, false);

    for(const auto& [channel, paths] : mapPaths){
      for(const auto& trig : paths){
	bool pass = triggerdecos.at("trigPassed_"+trig)(*eventInfo);
	mapDecisions.at(channel) |= pass;
	if(pass){
	  for (const xAOD::Jet *jet : *jets){
	    bool matchL1 = !jetL1Thresholds.at("trigPassed_"+trig)(*jet).empty();
	    jet_trigMatchDecos.at(HHBBTT::DBT_L1)(*jet) |= matchL1;
	    jet_trigMatchThresholds.at(HHBBTT::DBT_L1)(*jet) = jetL1Thresholds.at("trigPassed_"+trig)(*jet);
	    jet_trigMatchOnlinePt.at(HHBBTT::DBT_L1)(*jet) = jetL1ET.at("trigPassed_"+trig)(*jet);
	    bool matchHLT = !jetHLTThresholds.at("trigPassed_"+trig)(*jet).empty();
	    jet_trigMatchDecos.at(HHBBTT::DBT_HLT)(*jet) |= matchHLT;
	    jet_trigMatchThresholds.at(HHBBTT::DBT_HLT)(*jet) = jetHLTThresholds.at("trigPassed_"+trig)(*jet);
	    jet_trigMatchOnlinePt.at(HHBBTT::DBT_HLT)(*jet) = jetHLTPT.at("trigPassed_"+trig)(*jet);
	    jet_trigMatchDecos.at(HHBBTT::DBT)(*jet) |= matchHLT;
	  }
	}
      }

      pass_decos.at(channel)(*eventInfo) |= mapDecisions.at(channel);
      pass_decos.at(HHBBTT::DBT)(*eventInfo) |= mapDecisions.at(channel);
    }


  }

  void TriggerDecoratorAlg::checkLargeRJetsTriggers(
   int year, 
   const xAOD::EventInfo* eventInfo,
   const trigReadDecoMap& triggerdecos,
   passWriteDecoMap& pass_decos
  ) const {
    
    // get triggers name
    std::vector<std::string> largeRjets_paths;
    getLargeRJetsTriggers(year, largeRjets_paths);

    // Loop on triggers
    bool trigPassed_largeRjets = false;
    for(const auto& trig : largeRjets_paths){
      bool pass = triggerdecos.at("trigPassed_"+trig)(*eventInfo);
      trigPassed_largeRjets |= pass;
      // TODO: __BOOSTED__ map also each jet ?
    }  
    pass_decos.at(HHBBTT::LARGE_R_JETS)(*eventInfo) |= trigPassed_largeRjets;
  }

}


