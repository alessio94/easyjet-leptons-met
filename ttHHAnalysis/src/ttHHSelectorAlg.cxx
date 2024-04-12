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
    ATH_CHECK(m_passTriggerBjet.initialize(m_systematicsList, m_eventHandle));

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
        if (!m_passTriggerBjet.empty() and m_passTriggerBjet.get(*event, sys)) {
          m_ttHHCuts("PASS_TRIGGER").passed = true;
        } else if (!m_passTriggerSinglep.empty() and m_passTriggerSinglep.get(*event, sys)) {
          m_ttHHCuts("PASS_TRIGGER").passed = true;
        } else if (!m_passTriggerDilep.empty() and m_passTriggerDilep.get(*event, sys)) {
          m_ttHHCuts("PASS_TRIGGER").passed = true;
        }
      }

      evaluateJetCuts(*bjets, *jets, m_ttHHCuts);

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

      if (!m_bypass and !m_ttHHCuts("PASS_BASELINE").passed) continue;

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

  void ttHHSelectorAlg::evaluateJetCuts(const xAOD::JetContainer& bjets,
                            const xAOD::JetContainer& jets, CutManager& ttHHCuts)
  {

    if (jets.size() >= 4 && ttHHCuts.exists("NJETS"))
        ttHHCuts("NJETS").passed = true;

    if (bjets.size() >= 4 && ttHHCuts.exists("NBJETS"))
        ttHHCuts("NBJETS").passed = true;

    if (ttHHCuts("NBJETS").passed && ttHHCuts("NJETS").passed)
        ttHHCuts("PASS_BASELINE").passed = true;

  }

}
