/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "jjjjSelectorAlg.h"
#include <AthenaKernel/Units.h>

#include <AsgDataHandles/ReadDecorHandle.h>
#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>
#include <AthContainers/ConstDataVector.h>


namespace jjjj
{

  jjjjSelectorAlg::jjjjSelectorAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : EL::AnaAlgorithm(name, pSvcLocator)
  {
  }


  StatusCode jjjjSelectorAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("     jjjjSelectorAlg      \n");
    ATH_MSG_INFO("*********************************\n");

    // Initialise global event filter
    ATH_CHECK (m_filterParams.initialize(m_systematicsList));
    
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList)); 
    
    ATH_CHECK(m_year.initialize(m_systematicsList, m_eventHandle));
    
    ATH_CHECK (m_matchingTool.retrieve());

    for (auto& [key, value] : m_boolnames) {
      m_bools.emplace(key, false);
      CP::SysWriteDecorHandle<bool> whandle{value+"_%SYS%", this};
      m_Bbranches.emplace(key, whandle);
      ATH_CHECK(m_Bbranches.at(key).initialize(m_systematicsList, m_eventHandle));
    }

    // make trigger decorators
    for (auto trig : m_triggers){
      CP::SysReadDecorHandle<bool> deco {this, "trig"+trig, trig, "Name of trigger"};
      m_triggerdecos.emplace(trig, deco);
      ATH_CHECK(m_triggerdecos.at(trig).initialize(m_systematicsList, m_eventHandle));
    }

    // special flag for all cuts
    ATH_CHECK (m_pass_cuts.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    std::vector<std::string> boolnameslist;
    for (const auto& [key, value] : m_boolnames) {
      boolnameslist.push_back(value);
    }
    m_jjjjCuts.CheckInputCutList(m_inputCutList, boolnameslist);

    m_inputCutKeys.resize(m_inputCutList.size());
    std::vector<bool> inputWasFound (m_inputCutList.size(), false);
    for (const auto& [key, value]: m_boolnames) {
      auto it = std::find(m_inputCutList.begin(), m_inputCutList.end(), value);
      if (it != m_inputCutList.end()) {
        auto index = it - m_inputCutList.begin();
        m_inputCutKeys.at(index) = key;
        inputWasFound.at(index) = true;
      }
    }

    for (unsigned int index = 0; index < inputWasFound.size(); index++) {
      if(inputWasFound.at(index)) continue;
      ATH_MSG_ERROR("Doubled or falsely spelled cuts in CutList (see config file)." + m_inputCutList[index]);
    }

    for (const auto &cut : m_inputCutKeys) {
      m_jjjjCuts.add(m_boolnames[cut]);
    }

    return StatusCode::SUCCESS;
  }


  StatusCode jjjjSelectorAlg::execute()
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
      m_bools.at(jjjj::pass_trigger_SJT) = false;
      m_bools.at(jjjj::PASS_TRIGGER) = false;
      setThresholds(event, sys);
      const xAOD::Jet* j1 = nullptr;
      if (jets->size() >= 1) {
        j1 = jets->at(0);
      }
      evaluateTriggerCuts(event, j1, m_jjjjCuts, sys);
      bool pass_baseline=true;
      for (const auto& [key, value] : m_boolnames) {
        auto it = std::find(m_BASELINE_CUTS.begin(), m_BASELINE_CUTS.end(), value);
        if (it != m_BASELINE_CUTS.end()) {
          pass_baseline &= m_bools.at(key);
        }
      }
      m_pass_cuts.set(*event, pass_baseline, sys);

      // do the CUTFLOW only with sys="" -> NOSYS
      if (sys.name()=="") {

        // Compute total_events
        m_total_events+=1;

        for (const auto &cut : m_inputCutKeys) {
          if (m_jjjjCuts.exists(m_boolnames.at(cut))) {
            m_jjjjCuts(m_boolnames.at(cut)).passed = m_bools.at(cut);
            if (m_jjjjCuts(m_boolnames.at(cut)).passed) {
              m_jjjjCuts(m_boolnames.at(cut)).counter += 1;
            }
          }
        }
      }

      // Check how many consecutive cuts are passed by the event.
      unsigned int consecutive_cuts = 0;
      for (size_t i = 0; i < m_jjjjCuts.size(); ++i) {
        if (m_jjjjCuts[i].passed)
          consecutive_cuts++;
        else
          break;
      }

      for (unsigned int i=0; i<consecutive_cuts; i++) {
        m_jjjjCuts[i].relativeCounter+=1;
      }

      for (auto& [key, var] : m_bools) {
        m_Bbranches.at(key).set(*event, var, sys);
      }

      if (!m_bypass && !pass_baseline) continue;
      filter.setPassed(true);
    }

    return StatusCode::SUCCESS;
  }

  StatusCode jjjjSelectorAlg::finalize()
  {
    //adapt the following for each syst TODO
    ATH_MSG_INFO("Total events = " << m_total_events <<std::endl);
    ANA_CHECK (m_filterParams.finalize ());
    m_jjjjCuts.CheckCutResults(); // Print CheckCutResults
    return StatusCode::SUCCESS;
  }

  void jjjjSelectorAlg::evaluateTriggerCuts
  (const xAOD::EventInfo *event, const xAOD::Jet* j1,
   CutManager& jjjjCuts, const CP::SystematicSet& sys) {
    if (!jjjjCuts.exists("PASS_TRIGGER"))
        return;
    if (j1) evaluateSingleJetTrigger(event, j1, sys);

    if (m_bools.at(jjjj::pass_trigger_SJT)) m_bools.at(jjjj::PASS_TRIGGER) = true;
  }

  void jjjjSelectorAlg::evaluateSingleJetTrigger
  (const xAOD::EventInfo *event, const xAOD::Jet* j1,
   const CP::SystematicSet& sys)
  {
    // Check single jet triggers
    std::vector<std::string> single_jet_paths;

    int year = m_year.get(*event, sys);
    if(year==2015 || year == 2016){
      single_jet_paths = {
        "HLT_j380"
      };
    }
    else if(year==2017 || year==2018){
      single_jet_paths = {
        "HLT_j420"
      };
    }
    else if(year==2022 || year==2023){
      single_jet_paths = {
        "HLT_j420_pf_ftf_preselj225_L1J100"
      };
    }
    bool trigPassed_SJT = false;
    if(j1){
      for(const auto& trig : single_jet_paths){
        bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
        trigPassed_SJT |= pass;
      }
      trigPassed_SJT &= j1->pt() > m_pt_threshold[jjjj::SJT][jjjj::j1];
    }
    m_bools.at(jjjj::pass_trigger_SJT) |= trigPassed_SJT;
  }

  // leading jet pT thresholds for trigger turn on.
  void jjjjSelectorAlg::setThresholds(const xAOD::EventInfo* event,
					const CP::SystematicSet& sys) {
    
    int year = m_year.get(*event, sys);

    // Single-jet triggers
    if(year==2015 || year == 2016)
      m_pt_threshold[jjjj::SJT][jjjj::j1] = 480. * Athena::Units::GeV;
    else if (year==2017 || year==2018)
      m_pt_threshold[jjjj::SJT][jjjj::j1] = 480. * Athena::Units::GeV;
  }

}

