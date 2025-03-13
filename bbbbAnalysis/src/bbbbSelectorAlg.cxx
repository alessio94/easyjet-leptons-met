/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "bbbbSelectorAlg.h"
#include <AthenaKernel/Units.h>

#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>

namespace HH4B
{
  bbbbSelectorAlg::bbbbSelectorAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
  }


  StatusCode bbbbSelectorAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("         bbbbSelectorAlg         \n");
    ATH_MSG_INFO("*********************************\n");

    ATH_CHECK (m_filterParams.initialize(m_systematicsList));

    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    if (!m_lRjetHandle.empty()) {
      ATH_CHECK (m_lRjetHandle.initialize(m_systematicsList));
    }
    if (!m_jetHandle.empty()) {
      ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    }
    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }
    
    if (m_saveCutFlow) ATH_CHECK (m_generatorWeight.initialize(m_systematicsList, m_eventHandle));

    //Initialize trigger decorations
    for (const std::string &trig : m_Triggers)
    {
      std::string triggerDecorName = "trigPassed_"+trig;
      SG::ReadDecorHandleKey< xAOD::EventInfo > triggerDecorKey("EventInfo." + triggerDecorName);
      m_triggerDecorKeys.emplace(trig,triggerDecorKey);
      ATH_CHECK(m_triggerDecorKeys.at(trig).initialize());
    }
    
    for (const std::string &string_var: m_inputCutList) {
      CP::SysWriteDecorHandle<bool> var {string_var+"_%SYS%", this};
      m_Bbranches.emplace(string_var, var);
      ATH_CHECK (m_Bbranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    if(m_saveCutFlow) ATH_CHECK (initialiseCutflow());
    return StatusCode::SUCCESS;
  }


  StatusCode bbbbSelectorAlg::execute()
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

      // reset all cut flags to default=false
      for (CutEntry& cut : m_bbbbCuts) {
        cut.passed = false;
        m_Bbranches.at(cut.name).set(*event, cut.passed, sys);
      }
    
      evaluateTriggerCuts(*event, m_Triggers, m_bbbbCuts);
      
      // Get number of jets
      int n_boosted_jets = 0;
      int n_resolved_jets = 0;
      int n_b_jets = 0;
      if (!m_lRjetHandle.empty()) {
        const xAOD::JetContainer *lRjets = nullptr;
        ANA_CHECK (m_lRjetHandle.retrieve (lRjets, sys));
        n_boosted_jets = lRjets->size();
      }
      if (!m_jetHandle.empty()) {
        const xAOD::JetContainer *jets = nullptr;
        ANA_CHECK (m_jetHandle.retrieve (jets, sys));
        n_resolved_jets = jets->size();
        // Count b-tagged jets
        if (!m_isBtag.empty()) {
          for (const xAOD::Jet *jet : *jets) {
            if (m_isBtag.get(*jet, sys)) n_b_jets++;
          }
        }
      }

      evaluateJetCuts(n_boosted_jets, n_resolved_jets, n_b_jets, m_bbbbCuts);

      bool passedall = true;
      for (CutEntry& cut : m_bbbbCuts) {
        passedall = passedall && cut.passed;
        m_Bbranches.at(cut.name).set(*event, cut.passed, sys);
      }

      // Global event filter true if any syst passes and controls
      // if event is passed to output writing or not
      if (m_bypass || passedall) filter.setPassed(true);

      // Only store cutflow for NOSYS
      if (sys.name()=="" && m_saveCutFlow){

        // Compute total_events
        m_total_events+=1;
        if (m_isMC) m_total_mcEventWeight+= m_generatorWeight.get(*event, sys);

        // Count how many cuts the event passed and increase the relative counter
        for (const auto &cut : m_inputCutList) {
          if(m_bbbbCuts.exists(cut)) {
            if (m_bbbbCuts(cut).passed) {
              m_bbbbCuts(cut).counter += 1;
              if (m_isMC) m_bbbbCuts(cut).w_counter += m_generatorWeight.get(*event, sys);
            }
          }
        }

        // Check how many consecutive cuts are passed by the event.
        unsigned int consecutive_cuts = 0;
        for (size_t i = 0; i < m_bbbbCuts.size(); ++i) {
          if (m_bbbbCuts[i].passed) consecutive_cuts++;
          else break;
        }

        // Here we basically increment the  N_events(pass_i  AND pass_i-1  AND ... AND pass_0) for the i-cut.
        // I think this is an elegant way to do it :) . Considering the difficulties a configurable cut list imposes. 
        for (unsigned int i=0; i<consecutive_cuts; i++) {
          m_bbbbCuts[i].relativeCounter += 1;
          if (m_isMC) m_bbbbCuts[i].w_relativeCounter += m_generatorWeight.get(*event, sys);
        }
      }

    }

    return StatusCode::SUCCESS;
  }

  StatusCode bbbbSelectorAlg::finalize()
  {
    //adapt the following for each syst TODO
    ATH_MSG_INFO("Total events = " << m_total_events <<std::endl);
    m_bbbbCuts.CheckCutResults(); // Print CheckCutResults

    if(m_saveCutFlow) {
      m_bbbbCuts.DoAbsoluteEfficiency(m_total_events, efficiency("AbsoluteEfficiency"));
      m_bbbbCuts.DoRelativeEfficiency(m_total_events, efficiency("RelativeEfficiency"));
      m_bbbbCuts.DoStandardCutFlow(m_total_events, efficiency("StandardCutFlow"));
      if (m_isMC) {
        m_bbbbCuts.DoWeightedAbsoluteEfficiency(m_total_mcEventWeight, efficiency("WeightedAbsoluteEfficiency"));
        m_bbbbCuts.DoWeightedRelativeEfficiency(m_total_mcEventWeight, efficiency("WeightedRelativeEfficiency"));
        m_bbbbCuts.DoWeightedStandardCutFlow(m_total_mcEventWeight, efficiency("WeightedStandardCutFlow"));
      }
      m_bbbbCuts.DoCutflowLabeling(m_total_events, hist("EventsPassed_BinLabeling"));
    }

    return StatusCode::SUCCESS;

  }

  void bbbbSelectorAlg::evaluateTriggerCuts(const xAOD::EventInfo& event, const std::vector<std::string> &Triggers, 
                                            CutManager& bbbbCuts) {
    if (!bbbbCuts.exists("PASS_TRIGGER"))
        return;
    
    bool pass_trigger = false;

    for (const std::string &trigger : Triggers)
    {
      SG::ReadDecorHandleKey<xAOD::EventInfo>& triggerDecorKey = m_triggerDecorKeys.at(trigger);
      SG::ReadDecorHandle<xAOD::EventInfo, bool> m_triggerDecorHandle(triggerDecorKey);
    // OR between triggers
      pass_trigger |= m_triggerDecorHandle(event);     
    }
    bbbbCuts("PASS_TRIGGER").passed = pass_trigger;
  }

  void bbbbSelectorAlg::evaluateJetCuts(const int nBoostedJets, const int nResolvedJets, const int nBJets, CutManager& bbbbCuts)
  {
    if (nBoostedJets >= 2 && bbbbCuts.exists("AT_LEAST_TWO_LRJETS"))
      bbbbCuts("AT_LEAST_TWO_LRJETS").passed = true;

    if (nResolvedJets >= 4 && bbbbCuts.exists("AT_LEAST_FOUR_JETS"))
      bbbbCuts("AT_LEAST_FOUR_JETS").passed = true;

    if ((nBoostedJets >= 2 || nResolvedJets >= 4) && bbbbCuts.exists("AT_LEAST_FOUR_JETS_OR_TWO_LRJETS"))
      bbbbCuts("AT_LEAST_FOUR_JETS_OR_TWO_LRJETS").passed = true;

    if (nBJets >= 2 && bbbbCuts.exists("AT_LEAST_TWO_BJETS"))
      bbbbCuts("AT_LEAST_TWO_BJETS").passed = true;

    if ((nBoostedJets >= 2 || (nResolvedJets >= 4 && nBJets >= 2)) && bbbbCuts.exists("AT_LEAST_FOUR_JETS_TWO_BJETS_OR_TWO_LRJETS"))
      bbbbCuts("AT_LEAST_FOUR_JETS_TWO_BJETS_OR_TWO_LRJETS").passed = true;
  }

  StatusCode bbbbSelectorAlg::initialiseCutflow(){

    m_bbbbCuts.CheckInputCutList(m_inputCutList,m_STANDARD_CUTS);

    for (const std::string &cut : m_inputCutList)  {
      // Initialize a vector of CutEntry structs based on the input Cut List
      m_bbbbCuts.add(cut);
    }

    //After filling the CutManager, book your histograms.
    const unsigned int nbins = m_bbbbCuts.size() + 1; //  need an extra bin for the total num of events.
    ANA_CHECK (book (TEfficiency("AbsoluteEfficiency","Absolute Efficiency of HH-> cuts.Needs rescaling to total events.;Cuts;#epsilon",
				 nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("RelativeEfficiency","Relative Efficiency of HH-> cuts.Needs rescaling to total events.;Cuts;#epsilon",
				 nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("StandardCutFlow","StandardCutFlow of HH-> cuts.Needs rescaling to total events.;Cuts;#epsilon",
				 nbins, 0.5, nbins + 0.5)));
    if (m_isMC) {
      ANA_CHECK (book (TEfficiency("WeightedAbsoluteEfficiency","Weighted Absolute Efficiency of HH-> cuts.Needs rescaling to sumOfWeights.;Cuts;#epsilon",
				   nbins, 0.5, nbins + 0.5)));
      ANA_CHECK (book (TEfficiency("WeightedRelativeEfficiency","Weighted Relative Efficiency of HH-> cuts.Needs rescaling to sumOfWeights.;Cuts;#epsilon",
				   nbins, 0.5, nbins + 0.5)));
      ANA_CHECK (book (TEfficiency("WeightedStandardCutFlow","Weighted StandardCutFlow of HH-> cuts.Needs rescaling to sumOfWeights.;Cuts;#epsilon",
				   nbins, 0.5, nbins + 0.5)));
    }
    ANA_CHECK (book (TH1F("EventsPassed_BinLabeling", "Events passed by each cut / Bin labeling", nbins, 0.5, nbins + 0.5)));

    return StatusCode::SUCCESS;
  }

}
