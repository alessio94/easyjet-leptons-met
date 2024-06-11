/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ttHHSelectorAlg.h"
#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>

namespace ttHH
{

  ttHHSelectorAlg::ttHHSelectorAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("cutList", m_inputCutList);
    declareProperty("saveCutFlow", m_saveCutFlow);
    declareProperty("triggerLists", m_leptonTriggers);
  }


  StatusCode ttHHSelectorAlg::initialize()
  {

    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("      ttHHSelectorAlg      \n");
    ATH_MSG_INFO("*********************************\n");

    // Initialise global event filter
    ATH_CHECK (m_filterParams.initialize(m_systematicsList));

    ATH_CHECK(m_passTriggerDilep.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_passTriggerSinglep.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK (m_bjetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    ATH_CHECK(m_selected_el.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK(m_selected_mu.initialize(m_systematicsList, m_muonHandle));

    for (const std::string &string_var: m_inputCutList) {
      CP::SysWriteDecorHandle<bool> var {string_var+"_%SYS%", this};
      m_Bbranches.emplace(string_var, var);
      ATH_CHECK (m_Bbranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }

    //Initialise booleans with a false value
    for (auto& [key, value] : m_triggermatchingnames) {
      m_triggers_matchs.emplace(key, false);
      CP::SysWriteDecorHandle<bool> var {value+"_%SYS%", this};
      m_Bbranches.emplace(value, var);
      ATH_CHECK(m_Bbranches.at(value).initialize(m_systematicsList, m_eventHandle));
    };

    m_eleWPDecorHandle = CP::SysReadDecorHandle<char>
      ("baselineSelection_" + m_eleWPName+"_%SYS%", this);
    m_muonWPDecorHandle = CP::SysReadDecorHandle<char>
      ("baselineSelection_"+m_muonWPName+"_%SYS%", this);

    ATH_CHECK(m_eleWPDecorHandle.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK(m_muonWPDecorHandle.initialize(m_systematicsList, m_muonHandle));

    // special flag for all cuts
    ATH_CHECK (m_passallcuts.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize()); 

    m_ttHHCuts.CheckInputCutList(m_inputCutList,m_STANDARD_CUTS);

    for (const std::string &cut : m_inputCutList)  { 
      // Initialize a vector of CutEntry structs based on the input Cut List
      m_ttHHCuts.add(cut);
    }

    //After filling the CutManager, book your histograms.
    const unsigned int nbins = m_ttHHCuts.size() + 1; //  need an extra bin for the total num of events.
    ANA_CHECK (book (TEfficiency("AbsoluteEfficiency","Absolute Efficiency of ttHH(4b) cuts;Cuts;#epsilon", 
                                  nbins, 0.5, nbins + 0.5))); 
    ANA_CHECK (book (TEfficiency("RelativeEfficiency","Relative Efficiency of ttHH(4b) cuts;Cuts;#epsilon", 
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("StandardCutFlow","StandardCutFlow of ttHH(4b) cuts;Cuts;#epsilon", 
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TH1F("EventsPassed_BinLabeling", "Events passed by each cut / Bin labeling", nbins, 0.5, nbins + 0.5)));    

    return StatusCode::SUCCESS;
  }


  StatusCode ttHHSelectorAlg::execute()
  {

    // Global filter originally false
    CP::SysFilterReporterCombiner filterCombiner (m_filterParams, false);

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      CP::SysFilterReporter filter (filterCombiner, sys);

      // Retrive inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      const xAOD::JetContainer *bjets = nullptr;
      ANA_CHECK (m_bjetHandle.retrieve (bjets, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      // lepton WP
      for (const xAOD::Electron *electron : *electrons)
      {
        bool passElectronWP = m_eleWPDecorHandle.get(*electron, sys);
        m_selected_el.set(*electron, passElectronWP, sys);
      }
      for (const xAOD::Muon *muon : *muons)
      {
        bool passMuonWP = m_muonWPDecorHandle.get(*muon, sys);
        m_selected_mu.set(*muon, passMuonWP, sys);
      }

      // reset all cut flags to default=false
      for (CutEntry& cut : m_ttHHCuts) {
        cut.passed = false;
        m_Bbranches.at(cut.name).set(*event, cut.passed, sys);
      }

      if (m_ttHHCuts.exists("PASS_TRIGGER")) {
        if (!m_passTriggerSinglep.empty() and m_passTriggerSinglep.get(*event, sys)) {
          m_ttHHCuts("PASS_TRIGGER").passed = true;
        } else if (!m_passTriggerDilep.empty() and m_passTriggerDilep.get(*event, sys)) {
          m_ttHHCuts("PASS_TRIGGER").passed = true;
        }
      }

      evaluateCuts(*bjets, *muons, *electrons, m_ttHHCuts);
      if (!m_leptonTriggers.empty())
        evaluateTriggerMatchingCuts(m_leptonTriggers, muons, electrons,m_ttHHCuts);
      
      m_Bbranches.at("pass_matching_trigger_singlep").set(*event, m_triggers_matchs.at(ttHH::pass_matching_trigger_singlep), sys);
      m_Bbranches.at("pass_matching_trigger_dilep").set(*event, m_triggers_matchs.at(ttHH::pass_matching_trigger_dilep), sys);
      
      bool passedall = true;
      for (CutEntry& cut : m_ttHHCuts) {
        passedall = passedall && cut.passed;
        m_Bbranches.at(cut.name).set(*event, cut.passed, sys);
      }
      m_passallcuts.set(*event, passedall, sys);

      // do the CUTFLOW only with sys="" -> NOSYS
      if (sys.name()!="") continue;

      // Compute total_events
      m_total_events+=1; 

      // Count how many cuts the event passed and increase the relative counter
      for (const auto &cut : m_inputCutList) {
        if(m_ttHHCuts.exists(cut)) {
          if (m_ttHHCuts(cut).passed)
            m_ttHHCuts(cut).counter+=1;
        }
      }

      // Check how many consecutive cuts are passed by the event.
      unsigned int consecutive_cuts = 0;
      for (const auto& cut : m_ttHHCuts) {
        if (cut.passed) {
          consecutive_cuts++;
	}
        else {
          break;
        }
      }

      // Here we basically increment the  N_events(pass_i  AND pass_i-1  AND ... AND pass_0) for the i-cut.
      // I think this is an elegant way to do it :) . Considering the difficulties a configurable cut list imposes. 
      for (unsigned int i=0; i<consecutive_cuts; i++) {
        m_ttHHCuts[i].relativeCounter+=1;
      }

      if (!m_bypass and (!m_ttHHCuts("PASS_BASELINE").passed or !m_ttHHCuts("PASS_TRIGGER").passed or !m_ttHHCuts("PASS_TRIGGER_MATCHING").passed)) continue;

      // Global event filter true if any syst passes and controls
      // if event is passed to output writing or not
      filter.setPassed(true);

    }

    return StatusCode::SUCCESS;
  }

  StatusCode ttHHSelectorAlg::finalize()
  {
    ANA_CHECK (m_filterParams.finalize());

    //adapt the following for each syst TODO
    ATH_MSG_INFO("Total events = " << m_total_events <<std::endl);
    m_ttHHCuts.CheckCutResults(); // Print CheckCutResults

    if(m_saveCutFlow) {
      m_ttHHCuts.DoAbsoluteEfficiency(m_total_events, efficiency("AbsoluteEfficiency"));
      m_ttHHCuts.DoRelativeEfficiency(m_total_events, efficiency("RelativeEfficiency"));
      m_ttHHCuts.DoStandardCutFlow(m_total_events, efficiency("StandardCutFlow"));
      m_ttHHCuts.DoCutflowLabeling(m_total_events, hist("EventsPassed_BinLabeling"));
    }
    else {
      delete efficiency("AbsoluteEfficiency");
      delete efficiency("RelativeEfficiency");
      delete efficiency("StandardCutFlow");
      delete hist("EventsPassed_BinLabeling");
    }


     return StatusCode::SUCCESS;

  }

  void ttHHSelectorAlg::evaluateCuts(const xAOD::JetContainer& bjets,
                            const xAOD::MuonContainer& muons, 
			    const xAOD::ElectronContainer& electrons,
			    CutManager& ttHHCuts)
  {

    int nLeptons = muons.size() + electrons.size();
    int nBJets = bjets.size();

    if ((((nLeptons==1) && (nBJets>=4)) || ((nLeptons>=2) && (nBJets>=2))) && ttHHCuts.exists("PASS_BASELINE"))
        ttHHCuts("PASS_BASELINE").passed = true;

  }

  void ttHHSelectorAlg::evaluateTriggerMatchingCuts(const std::vector<std::string> &m_leptonTriggers, 
                                                  const xAOD::MuonContainer* muons,  const xAOD::ElectronContainer* electrons, CutManager& ttHHCuts) {

    if (!ttHHCuts.exists("PASS_TRIGGER_MATCHING"))
      return;

    int nLeptons = muons->size() + electrons->size();
    bool pass_matching_trigger_singlep = false;
    bool pass_matching_trigger_dilep = false;
    if (nLeptons>=1){
      for (const std::string &trigger : m_leptonTriggers){
        if (muons->size()==1 && electrons->size()==0){
          if (trigger.find("_mu") != std::string::npos && trigger.find("_e") == std::string::npos)
            pass_matching_trigger_singlep |= m_matchingTool->match(*muons->at(0), trigger);
        } else if (muons->size()==0 && electrons->size()==1){
          if (trigger.find("_e") != std::string::npos && trigger.find("_mu") == std::string::npos)
            pass_matching_trigger_singlep |= m_matchingTool->match(*electrons->at(0), trigger);
        } else if (muons->size()>=1 && electrons->size()>=1){
          if (trigger.find("_e") != std::string::npos && trigger.find("_mu") != std::string::npos)
            pass_matching_trigger_dilep |= m_matchingTool->match({muons->at(0), electrons->at(0)}, trigger);
        } else if (muons->size()>=2 && electrons->size()==0){
          if (trigger.find("_2mu") != std::string::npos)
            pass_matching_trigger_dilep |= m_matchingTool->match({muons->at(0), muons->at(1)}, trigger);
        } else if (muons->size()==0 && electrons->size()>=2){
          if (trigger.find("_2e") != std::string::npos)
            pass_matching_trigger_dilep |= m_matchingTool->match({electrons->at(0), electrons->at(1)}, trigger);
        }
      }
    }

    ttHHCuts("PASS_TRIGGER_MATCHING").passed = pass_matching_trigger_singlep || pass_matching_trigger_dilep;
    m_triggers_matchs.at(ttHH::pass_matching_trigger_singlep) = pass_matching_trigger_singlep;
    m_triggers_matchs.at(ttHH::pass_matching_trigger_dilep) = pass_matching_trigger_dilep;
  }

}
