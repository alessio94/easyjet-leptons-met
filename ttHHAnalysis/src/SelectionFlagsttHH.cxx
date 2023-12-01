/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "SelectionFlagsttHH.h"

namespace ttHH
{

  SelectionFlagsttHHAlg::SelectionFlagsttHHAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("cutList", m_inputCutList);
    declareProperty("saveCutFlow", m_saveCutFlow);
    declareProperty("triggers", m_Triggers);
    declareProperty("nLeptons", m_nLeptons);
  }


  StatusCode SelectionFlagsttHHAlg::initialize()
  {

    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("      SelectionFlagsttHHAlg      \n");
    ATH_MSG_INFO("*********************************\n");

    ATH_CHECK (m_bjetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    //Initialize trigger decorations
    for (const std::string &trig : m_Triggers)
    {
      // CP alg should convert trigger names
      std::string modifiedTrigName = trig;
      std::replace(modifiedTrigName.begin(), modifiedTrigName.end(), '-', '_');
      std::replace(modifiedTrigName.begin(), modifiedTrigName.end(), '.', 'p');

      std::string triggerDecorName = "trigPassed_"+modifiedTrigName;
      SG::ReadDecorHandleKey< xAOD::EventInfo > triggerDecorKey = m_eventHandle.getNamePattern() + "." + triggerDecorName;

      m_triggerDecorKeys.emplace(modifiedTrigName,triggerDecorKey);
      ATH_CHECK(m_triggerDecorKeys.at(modifiedTrigName).initialize());
    }

    for (const std::string &string_var: m_inputCutList) {
      CP::SysWriteDecorHandle<bool> var {string_var+"_%SYS%", this};
      m_Bbranches.emplace(string_var, var);
      ATH_CHECK (m_Bbranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }
  
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


  StatusCode SelectionFlagsttHHAlg::execute()
  {

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
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
     
      // reset all cut flags to default=false
      for (CutEntry& cut : m_ttHHCuts) {
        cut.passed = false;
        m_Bbranches.at(cut.name).set(*event, cut.passed, sys);
      }

      if (!m_Triggers.empty()) {
        evaluateTriggerCuts(*event, m_Triggers, m_ttHHCuts);
      }

      evaluateLeptonCuts(*electrons, *muons, m_ttHHCuts);
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

    }

    return StatusCode::SUCCESS;
  }

  StatusCode SelectionFlagsttHHAlg::finalize()
  {

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

  void SelectionFlagsttHHAlg::evaluateTriggerCuts(const xAOD::EventInfo& event, const std::vector<std::string> &Triggers, 
                                                  CutManager& ttHHCuts) {

    if (!ttHHCuts.exists("PASS_TRIGGER"))
        return;

    for (const std::string &trigger : Triggers)
    {
      // CP alg should convert trigger names
      std::string modifiedTrigName = trigger;
      std::replace(modifiedTrigName.begin(), modifiedTrigName.end(), '-', '_');
      std::replace(modifiedTrigName.begin(), modifiedTrigName.end(), '.', 'p');

      SG::ReadDecorHandleKey<xAOD::EventInfo>& triggerDecorKey = m_triggerDecorKeys.at(modifiedTrigName);
      SG::ReadDecorHandle<xAOD::EventInfo, bool> m_triggerDecorHandle(triggerDecorKey);

      //If the event passes any of the available triggers, set the overall trigger cut to true.
      if (m_triggerDecorHandle(event)) {
        ttHHCuts("PASS_TRIGGER").passed = true;
        break;
      }
    }

  }

  void SelectionFlagsttHHAlg::evaluateLeptonCuts(const xAOD::ElectronContainer& electrons,
                                const xAOD::MuonContainer& muons, CutManager& ttHHCuts)
  {

    if (!ttHHCuts.exists("NLEPTONS"))
      return;

    // No medium+isolated electrons and muons.
    int n_leptons = electrons.size() + muons.size();
    if (n_leptons==m_nLeptons)
      ttHHCuts("NLEPTONS").passed = true;

  }

  void SelectionFlagsttHHAlg::evaluateJetCuts(const xAOD::JetContainer& bjets,
                            const xAOD::JetContainer& jets, CutManager& ttHHCuts)
  {

    if (jets.size() >= 4 && ttHHCuts.exists("NJETS"))
        ttHHCuts("NJETS").passed = true;

    if (bjets.size() >= 4 && ttHHCuts.exists("NBJETS"))
        ttHHCuts("NBJETS").passed = true;

  }

}
