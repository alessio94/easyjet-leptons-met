/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "yymlSelectorAlg.h"
#include <AthenaKernel/Units.h>
#include <AthContainers/ConstDataVector.h>
#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>

namespace HHYYML
{
  yymlSelectorAlg::yymlSelectorAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : EL::AnaAlgorithm(name, pSvcLocator)
  {
  }


  StatusCode yymlSelectorAlg::initialize() {

    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("     yymlSelectorAlg             \n");
    ATH_MSG_INFO("*********************************\n");

    // Initialise global event filter
    ATH_CHECK (m_filterParams.initialize(m_systematicsList));

    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK (m_photonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_tauHandle.initialize(m_systematicsList));
    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    if (m_saveCutFlow) ATH_CHECK (m_generatorWeight.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK(m_year.initialize(m_systematicsList, m_eventHandle));

    // // make trigger decorators
    // for (auto trig : m_triggers){
    //   CP::SysReadDecorHandle<bool> deco {this, "trig"+trig, trig, "Name of trigger"};
    //   m_triggerdecos.emplace(trig, deco);
    //   ATH_CHECK(m_triggerdecos.at(trig).initialize(m_systematicsList, m_eventHandle));
    // }

    //Initialize trigger decorations
    for (const std::string &trig : m_photonTriggers)
    {
      SG::ReadDecorHandleKey< xAOD::EventInfo > triggerDecorKey(m_eventHandle.getNamePattern() + ".trigPassed_"+trig);
      m_triggerDecorKeys.emplace(trig,triggerDecorKey);
      ATH_CHECK(m_triggerDecorKeys.at(trig).initialize());
    }

    ATH_CHECK (m_matchingTool.retrieve());

    // Intialise booleans with value false. Also initialise syst-aware output decorators
    for (auto& [key, value] : m_boolnames) {
      m_bools.emplace(key, false);
      CP::SysWriteDecorHandle<bool> whandle{value+"_%SYS%", this};
      m_Bbranches.emplace(key, whandle);
      ATH_CHECK(m_Bbranches.at(key).initialize(m_systematicsList, m_eventHandle));
    };

    // Initialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    if (m_saveCutFlow) ATH_CHECK (initialiseCutflow());

    return StatusCode::SUCCESS;
  }


  StatusCode yymlSelectorAlg::execute() {

    // Global filter originally false
    CP::SysFilterReporterCombiner filterCombiner (m_filterParams, false);

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector()) {

      CP::SysFilterReporter filter (filterCombiner, sys);

      // Retrieve inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::PhotonContainer *photons = nullptr;
      ANA_CHECK (m_photonHandle.retrieve (photons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      const xAOD::TauJetContainer *taus = nullptr;
      ANA_CHECK (m_tauHandle.retrieve (taus, sys));

      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      auto nonbjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      for(const xAOD::Jet* jet : *jets) {
        if (WPgiven) {
          if (m_isBtag.get(*jet, sys) && std::abs(jet->eta())<2.5) bjets->push_back(jet); // TODO: is this eta cut needed/appropriate?
          else nonbjets->push_back(jet);
        }
      }

      for (const auto& [key, value] : m_bools) {
        m_bools.at(key) = false;
      }

      // Reset syst-aware output decorators to default=false
      for (auto& [key, var] : m_bools) {
        m_Bbranches.at(key).set(*event, false, sys);
      };

      // reset all cut flags to default=false
      for (CutEntry& cut : m_yymlCuts) {
        cut.passed = false;
      }

      // apply baseline selection for objects

      if (!m_photonTriggers.empty()) {
        evaluateTriggerCuts(*event, m_photonTriggers, m_yymlCuts);
        evaluateTriggerMatchingCuts(m_photonTriggers, photons, m_yymlCuts);
      }

      applyChannelSelection(*electrons, *muons, *taus, m_yymlCuts);

      bool pass_baseline=false;
      // TODO: only if PASS_TRIGGER is also in listed cuts?
      if(m_bools.at(HHYYML::PASS_TRIGGER)) pass_baseline=true;

      bool pass_selection = false;
      for (const auto& [key, value] : m_bools) {
        if (key == HHYYML::PASS_TRIGGER ||
            key == HHYYML::pass_trigger_diphoton ||
            key == HHYYML::pass_matching_trigger_diphoton)
          continue;
        // pass selection if any of the sub-channel selections pass
        pass_selection |= value;
      }

      pass_baseline &= pass_selection;


      //****************
      // Cutflow
      //****************

      // do the CUTFLOW only with sys="" -> NOSYS
      if (sys.name()=="" && m_saveCutFlow) {
        // Compute total_events
        m_total_events+=1;
        if(m_isMC) m_total_mcEventWeight+= m_generatorWeight.get(*event, sys);

        for (const auto &cut : m_inputCutKeys) {
          auto cutname = m_boolnames.at(cut);
          if (m_yymlCuts.exists(cutname)) {
            m_yymlCuts(cutname).passed = m_bools.at(cut);
            if (m_yymlCuts(cutname).passed) {
              m_yymlCuts(cutname).counter += 1;
              if(m_isMC) m_yymlCuts(cutname).w_counter += m_generatorWeight.get(*event, sys);
            }
          }
        }

        // Check how many consecutive cuts are passed by the event.
        unsigned int consecutive_cuts = 0;
        for (size_t i = 0; i < m_yymlCuts.size(); ++i) {
          if (m_yymlCuts[i].passed)
            consecutive_cuts++;
          else
            break;
        }

        // Here we basically increment the  N_events(pass_i  AND pass_i-1  AND ... AND pass_0) for the i-cut.
        for (unsigned int i=0; i<consecutive_cuts; i++) {
          m_yymlCuts[i].relativeCounter += 1;
          if(m_isMC) m_yymlCuts[i].w_relativeCounter += m_generatorWeight.get(*event, sys);
        }
      }

      // Fill syst-aware output decorators
      for (auto& [key, var] : m_bools) {
        m_Bbranches.at(key).set(*event, var, sys);
      };

      // if not in bypass mode and baseline not passed, don't write out (at least not due to this systematic)
      if (!m_bypass && !pass_baseline) continue;

      // Global event filter true if any syst passes and controls
      // if event is passed to output writing or not
      filter.setPassed(true);
    } // End loop over all systs
    return StatusCode::SUCCESS;
  }

  StatusCode yymlSelectorAlg::finalize() {
    //adapt the following for each syst TODO
    ATH_MSG_INFO("Total events = " << m_total_events <<std::endl);
    ANA_CHECK (m_filterParams.finalize ());
    m_yymlCuts.CheckCutResults(); // Print CheckCutResults

    if(m_saveCutFlow) {
      m_yymlCuts.DoAbsoluteEfficiency(m_total_events, efficiency("AbsoluteEfficiency"));
      m_yymlCuts.DoRelativeEfficiency(m_total_events, efficiency("RelativeEfficiency"));
      m_yymlCuts.DoStandardCutFlow(m_total_events, efficiency("StandardCutFlow"));
      if(m_isMC) {
        m_yymlCuts.DoWeightedAbsoluteEfficiency(m_total_mcEventWeight, efficiency("WeightedAbsoluteEfficiency"));
        m_yymlCuts.DoWeightedRelativeEfficiency(m_total_mcEventWeight, efficiency("WeightedRelativeEfficiency"));
        m_yymlCuts.DoWeightedStandardCutFlow(m_total_mcEventWeight, efficiency("WeightedStandardCutFlow"));
      }
      m_yymlCuts.DoCutflowLabeling(m_total_events, hist("EventsPassed_BinLabeling"));
    }

    return StatusCode::SUCCESS;
  }

  void yymlSelectorAlg::evaluateTriggerCuts(const xAOD::EventInfo& event,
    const std::vector<std::string> &photonTriggers, const CutManager& yymlCuts) {

    // If not requested as a cut, will default to false without determining decision explicitly (but still decorate it)
    if (!yymlCuts.exists("PASS_TRIGGER"))
        return;

    bool pass_trigger_diphoton = false;

    for (const std::string &trigger : photonTriggers) {

      SG::ReadDecorHandleKey<xAOD::EventInfo>& triggerDecorKey = m_triggerDecorKeys.at(trigger);
      SG::ReadDecorHandle<xAOD::EventInfo, bool> triggerDecorHandle(triggerDecorKey);

      pass_trigger_diphoton = triggerDecorHandle(event);

    }
    // Currently only diphoton trigger (no single photon trigger)
    m_bools.at(HHYYML::PASS_TRIGGER) = pass_trigger_diphoton;
    m_bools.at(HHYYML::pass_trigger_diphoton) = pass_trigger_diphoton;
  }

  void yymlSelectorAlg::evaluateTriggerMatchingCuts(const std::vector<std::string> &photonTriggers, 
    const xAOD::PhotonContainer* photons, const CutManager& yymlCuts) {

    // If not requested as a cut, will default to false without determining decision explicitly (but still decorate it)
    if (!yymlCuts.exists("pass_matching_trigger_diphoton"))
    return;

    bool pass_matching_trigger_diphoton = false;
    if (photons->size() >= 2 ) {

      for (const std::string &trigger : photonTriggers) {
        pass_matching_trigger_diphoton = m_matchingTool->match({photons->at(0), photons->at(1)}, trigger);
      }

    }
    m_bools.at(HHYYML::pass_matching_trigger_diphoton) = pass_matching_trigger_diphoton;

  }

  StatusCode yymlSelectorAlg::initialiseCutflow() {

    std::vector<std::string> boolnameslist;
    for (const auto& [key, value] : m_boolnames) {
      boolnameslist.push_back(value);
    }
    m_yymlCuts.CheckInputCutList(m_inputCutList, boolnameslist);

    // Initialize an array containing the enum values needed for the cutlist
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
    // Check that every element of m_inputCutList has a corresponding enum in m_inputCutEnum
    for (unsigned int index = 0; index < inputWasFound.size(); index++) {
      if(inputWasFound.at(index)) continue;
      ATH_MSG_ERROR("Doubled or falsely spelled cuts in CutList (see config file)." + m_inputCutList[index]);
    }
    // Initialize a vector of CutEntry structs based on the input Cut List
    for (const auto &cut : m_inputCutKeys) {
      m_yymlCuts.add(m_boolnames[cut]);
    }

    //After filling the CutManager, book your histograms.
    const unsigned int nbins = m_yymlCuts.size() + 1; //  need an extra bin for the total num of events.
    ANA_CHECK (book (TEfficiency("AbsoluteEfficiency","Absolute Efficiency of HH->yyml cuts;Cuts;#epsilon",
				 nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("RelativeEfficiency","Relative Efficiency of HH->yyml cuts;Cuts;#epsilon",
				 nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("StandardCutFlow","StandardCutFlow of HH->yyml cuts;Cuts;#epsilon",
				 nbins, 0.5, nbins + 0.5)));
    if(m_isMC) {
      ANA_CHECK (book (TEfficiency("WeightedAbsoluteEfficiency","Weighted Absolute Efficiency of HH->multilepton cuts;Cuts;#epsilon",
        nbins, 0.5, nbins + 0.5)));
      ANA_CHECK (book (TEfficiency("WeightedRelativeEfficiency","Weighted Relative Efficiency of HH->multilepton cuts;Cuts;#epsilon",
        nbins, 0.5, nbins + 0.5)));
      ANA_CHECK (book (TEfficiency("WeightedStandardCutFlow","Weighted StandardCutFlow of HH->multilepton cuts;Cuts;#epsilon",
        nbins, 0.5, nbins + 0.5)));
    }
    ANA_CHECK (book (TH1F("EventsPassed_BinLabeling", "Events passed by each cut / Bin labeling", nbins, 0.5, nbins + 0.5)));

    return StatusCode::SUCCESS;
  }

  bool yymlSelectorAlg::evaluate1l0tauSelection(
    const SubChannelClassify &classify,
    const CutManager& yymlCuts) {

    auto sub_channel_id = classify.getSubChannelId();
    if (!yymlCuts.exists("pass_1l0tau") || sub_channel_id != CH_ID::hh1l0tau) return false;
    bool pass_selection = true;

    // TODO: apply additional object-level selections

    return pass_selection;
  }

  bool yymlSelectorAlg::evaluate0l1tauSelection(
    const SubChannelClassify &classify,
    const CutManager& yymlCuts) {

    auto sub_channel_id = classify.getSubChannelId();
    if (!yymlCuts.exists("pass_0l1tau") || sub_channel_id != CH_ID::hh0l1tau) return false;
    bool pass_selection = true;

    // TODO: apply additional object-level selections

    return pass_selection;
  }

  bool yymlSelectorAlg::evaluate2l0tauSelection(
    const SubChannelClassify &classify,
    const CutManager& yymlCuts) {

    auto sub_channel_id = classify.getSubChannelId();
    if (!yymlCuts.exists("pass_2l0tau") || sub_channel_id != CH_ID::hh2l0tau) return false;
    bool pass_selection = true;

    // TODO: apply additional object-level selections

    return pass_selection;
  }

  bool yymlSelectorAlg::evaluate1l1tauSelection(
    const SubChannelClassify &classify,
    const CutManager& yymlCuts) {

    auto sub_channel_id = classify.getSubChannelId();
    if (!yymlCuts.exists("pass_1l1tau") || sub_channel_id != CH_ID::hh1l1tau) return false;
    bool pass_selection = true;

    // TODO: apply additional object-level selections

    return pass_selection;
  }

  bool yymlSelectorAlg::evaluate0l2tauSelection(
    const SubChannelClassify &classify,
    const CutManager& yymlCuts) {

    auto sub_channel_id = classify.getSubChannelId();
    if (!yymlCuts.exists("pass_0l2tau") || sub_channel_id != CH_ID::hh0l2tau) return false;
    bool pass_selection = true;

    // TODO: apply additional object-level selections

    return pass_selection;
  }

  void yymlSelectorAlg::applyChannelSelection(
    const xAOD::ElectronContainer& electrons,
    const xAOD::MuonContainer& muons,
    const xAOD::TauJetContainer& taus,
    CutManager& yymlCuts) {

    auto classifier = SubChannelClassify(&electrons, &muons, &taus);

    if (yymlCuts.exists("pass_1l0tau")) {
      m_bools.at(HHYYML::pass_1l0tau) = evaluate1l0tauSelection(classifier, yymlCuts);
    }
    if (yymlCuts.exists("pass_0l1tau")) {
      m_bools.at(HHYYML::pass_0l1tau) = evaluate0l1tauSelection(classifier, yymlCuts);
    }
    if (yymlCuts.exists("pass_2l0tau")) {
      m_bools.at(HHYYML::pass_2l0tau) = evaluate2l0tauSelection(classifier, yymlCuts);
    }
    if (yymlCuts.exists("pass_1l1tau")) {
      m_bools.at(HHYYML::pass_1l1tau) = evaluate1l1tauSelection(classifier, yymlCuts);
    }
    if (yymlCuts.exists("pass_0l2tau")) {
      m_bools.at(HHYYML::pass_0l2tau) = evaluate0l2tauSelection(classifier, yymlCuts);
    }

  }

} // namespace HHYYML

