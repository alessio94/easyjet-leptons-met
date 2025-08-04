/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MonojetSelectorAlg.h"
#include <AthenaKernel/Units.h>

#include <AsgDataHandles/ReadDecorHandle.h>
#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>
#include <AthContainers/ConstDataVector.h>


namespace MONOJET
{

  MonojetSelectorAlg::MonojetSelectorAlg(const std::string& name,
    ISvcLocator* pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
  }


  StatusCode MonojetSelectorAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("     MonojetSelectorAlg      \n");
    ATH_MSG_INFO("*********************************\n");

    // Initialise global event filter
    ATH_CHECK(m_filterParams.initialize(m_systematicsList));

    ATH_CHECK(m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_largejetHandle.initialize(m_systematicsList));
    if (!m_isBtag.empty()) {
      ATH_CHECK(m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    ATH_CHECK(m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK(m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK(m_tauHandle.initialize(m_systematicsList));
    ATH_CHECK(m_metHandle.initialize(m_systematicsList));
    ATH_CHECK(m_eventHandle.initialize(m_systematicsList));

    if (m_saveCutFlow) ATH_CHECK (m_generatorWeight.initialize(m_systematicsList, m_eventHandle));

    for (const std::string &string_var: m_inputCutList) {
      CP::SysWriteDecorHandle<bool> var {string_var+"_%SYS%", this};
      m_Bbranches.emplace(string_var, var);
      ATH_CHECK (m_Bbranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }

    // make trigger decorators
    for (auto trig : m_triggers) {
      std::string triggerDecorName = "trigPassed_"+trig;
      CP::SysReadDecorHandle<bool> deco{ this, "trig" + triggerDecorName, triggerDecorName, "Name of trigger" };
      m_triggerdecos.emplace(triggerDecorName, deco);
      ATH_CHECK(m_triggerdecos.at(triggerDecorName).initialize(m_systematicsList, m_eventHandle));
    }

    // special flag for all cuts
    ATH_CHECK(m_passallcuts.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK(m_systematicsList.initialize());


    if(m_saveCutFlow) ATH_CHECK (initialiseCutflow());
    return StatusCode::SUCCESS;
  }


  StatusCode MonojetSelectorAlg::execute()
  {
    // Global filter originally false
    CP::SysFilterReporterCombiner filterCombiner(m_filterParams, false);

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      CP::SysFilterReporter filter(filterCombiner, sys);

      // Retrive inputs
      const xAOD::EventInfo* event = nullptr;
      ANA_CHECK(m_eventHandle.retrieve(event, sys));

      const xAOD::JetContainer* jets = nullptr;
      ANA_CHECK(m_jetHandle.retrieve(jets, sys));

      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>>(SG::VIEW_ELEMENTS);
      for (const xAOD::Jet* jet : *jets) {
        if (WPgiven) {
          if (m_isBtag.get(*jet, sys) && std::abs(jet->eta()) < 2.5) bjets->push_back(jet);
        }
      }

      const xAOD::MissingETContainer *metCont = nullptr;
      ANA_CHECK(m_metHandle.retrieve(metCont, sys));
      const xAOD::MissingET* met = (*metCont)["Final"]; // To check
      if (!met) {
        ATH_MSG_ERROR("Could not retrieve MET");
        return StatusCode::FAILURE;
      }

      const xAOD::JetContainer *largeJets = nullptr;
      ANA_CHECK (m_largejetHandle.retrieve (largeJets, sys));

      const xAOD::MuonContainer* muons = nullptr;
      ANA_CHECK(m_muonHandle.retrieve(muons, sys));

      const xAOD::ElectronContainer* electrons = nullptr;
      ANA_CHECK(m_electronHandle.retrieve(electrons, sys));

      const xAOD::TauJetContainer* taus = nullptr;
      ANA_CHECK(m_tauHandle.retrieve(taus, sys));

      // reset all cut flags to default=false
      for (CutEntry& cut : m_MonojetCuts) {
        cut.passed = false;
        m_Bbranches.at(cut.name).set(*event, cut.passed, sys);
      }

      evaluateMETTriggerCuts(event,m_triggers, m_MonojetCuts, sys);
      evaluateJETCuts(*jets, m_MonojetCuts);
      evaluateLARGEJETCuts(*largeJets, m_MonojetCuts);
      evaluateMETCuts(met, m_MonojetCuts);
      evaluateLeptonVeto(*electrons, *muons, *taus, m_MonojetCuts);
      evaluateElectronVeto(*electrons, m_MonojetCuts);
      evaluateMuonVeto(*muons, m_MonojetCuts);
      evaluateTauVeto(*taus, m_MonojetCuts);
      evaluateALLcuts(m_MonojetCuts);

      bool passedall = true;
      for (CutEntry& cut : m_MonojetCuts) {
        passedall &= cut.passed;
        m_Bbranches.at(cut.name).set(*event, cut.passed, sys);
      }

      // Global event filter true if any syst passes and controls
      // if event is passed to output writing or not
      if (m_bypass || passedall) filter.setPassed(true);

      // do the CUTFLOW only with sys="" -> NOSYS
      if (sys.name() == "" && m_saveCutFlow) {
        // Compute total_events
        m_total_events += 1;
        if (m_isMC) m_total_mcEventWeight+= m_generatorWeight.get(*event, sys);

        // Count how many cuts the event passed and increase the relative counter
        for (const auto& cut : m_inputCutList) {
          if (m_MonojetCuts.exists(cut)) {
            if(m_MonojetCuts(cut).passed) {
              m_MonojetCuts(cut).counter += 1;
              if (m_isMC) m_MonojetCuts(cut).w_counter += m_generatorWeight.get(*event,sys);
            }
          }
        }

        // Check how many consecutive cuts are passed by the event.
        unsigned int consecutive_cuts = 0;
        for (size_t i = 0; i < m_MonojetCuts.size(); ++i) {
          if (m_MonojetCuts[i].passed) consecutive_cuts++;
          else break;
        }

        // Here we basically increment the  N_events(pass_i  AND pass_i-1  AND ... AND pass_0) for the i-cut.
        // I think this is an elegant way to do it :) . Considering the difficulties a configurable cut list imposes.
        for (unsigned int i = 0; i < consecutive_cuts; i++) {
          m_MonojetCuts[i].relativeCounter += 1;
          if (m_isMC) m_MonojetCuts[i].w_relativeCounter += m_generatorWeight.get(*event,sys);

        }
      }
    }
    return StatusCode::SUCCESS;
  }

  StatusCode MonojetSelectorAlg::finalize()
  {
    //adapt the following for each syst TODO
    ATH_MSG_INFO("Total events = " << m_total_events << std::endl);
    ANA_CHECK(m_filterParams.finalize());
    m_MonojetCuts.CheckCutResults(); // Print CheckCutResults

    if (m_saveCutFlow) {
      m_MonojetCuts.DoAbsoluteEfficiency(m_total_events, efficiency("AbsoluteEfficiency"));
      m_MonojetCuts.DoRelativeEfficiency(m_total_events, efficiency("RelativeEfficiency"));
      m_MonojetCuts.DoStandardCutFlow(m_total_events, efficiency("StandardCutFlow"));
      m_MonojetCuts.DoCutflowLabeling(m_total_events, hist("EventsPassed_BinLabeling"));

      std::cout<<"TOTAL MC EVENT "<<m_total_mcEventWeight<<std::endl;

      for (size_t i = 0; i < m_MonojetCuts.size(); ++i) {

          if ( (m_MonojetCuts[i].w_counter > m_total_mcEventWeight) || (m_MonojetCuts[i].w_relativeCounter > m_total_mcEventWeight)  ){
          std::cout<<"CUT "<< m_MonojetCuts[i].name<<std::endl;
          std::cout<<"w_counter "<< m_MonojetCuts[i].w_counter<<std::endl;
          std::cout<<"w_relativeCounter "<< m_MonojetCuts[i].w_relativeCounter<<std::endl;
        }
      }

      if (m_isMC) {
        m_MonojetCuts.DoWeightedAbsoluteEfficiency(m_total_mcEventWeight, efficiency("WeightedAbsoluteEfficiency"));
        m_MonojetCuts.DoWeightedRelativeEfficiency(m_total_mcEventWeight, efficiency("WeightedRelativeEfficiency"));
        m_MonojetCuts.DoWeightedStandardCutFlow(m_total_mcEventWeight, efficiency("WeightedStandardCutFlow"));
      }

    }

    return StatusCode::SUCCESS;
  }

  void MonojetSelectorAlg::evaluateALLcuts
  (CutManager& MonojetCuts) {
    if (!MonojetCuts.exists("ALL_CUTS")) {
      return;
    }

    bool passed = true;

    // check all other cuts
    for (CutEntry& cut : m_MonojetCuts) {

      if (cut.name == "ALL_CUTS") continue; 

      passed &= cut.passed;
    }

    MonojetCuts("ALL_CUTS").passed = passed;

  }

  // MET trigger
  void MonojetSelectorAlg::evaluateMETTriggerCuts
  (const xAOD::EventInfo* event, 
  const std::vector<std::string> &Triggers, CutManager& MonojetCuts, const CP::SystematicSet& sys) {

    if (!MonojetCuts.exists("PASS_TRIGGER")) {
      return;
    }

    bool pass_trigger = false;

    for (const auto& trig : Triggers) {
      bool pass = m_triggerdecos.at("trigPassed_" + trig).get(*event, sys);
      pass_trigger |= pass;
    }
    
    MonojetCuts("PASS_TRIGGER").passed = pass_trigger;

  }



  void MonojetSelectorAlg::evaluateJETCuts
  (const xAOD::JetContainer& jets, CutManager& MonojetCuts)
  {
    if (MonojetCuts.exists("JETCUT") && (jets.size() <= 4)) {
      MonojetCuts("JETCUT").passed = true;
    }
  }

  void MonojetSelectorAlg::evaluateLARGEJETCuts
  (const xAOD::JetContainer& largeJets, CutManager& MonojetCuts)
  {
    if (MonojetCuts.exists("LARGEJETCUT") && (largeJets.size() >= 1)){
      MonojetCuts("LARGEJETCUT").passed = true;
    }
  }

  void MonojetSelectorAlg::evaluateMETCuts
  (const xAOD::MissingET* met, CutManager& MonojetCuts)
  {
    if (MonojetCuts.exists("METCUT") && (m_met_cut > 0) ) {
      if (met->met() > m_met_cut * Athena::Units::GeV){
        MonojetCuts("METCUT").passed = true;
      }
    }
  }

  void MonojetSelectorAlg::evaluateLeptonVeto(const xAOD::ElectronContainer& electrons,
    const xAOD::MuonContainer& muons,
    const xAOD::TauJetContainer& taus,
    CutManager& MonojetCuts)
  {
    if (!MonojetCuts.exists("LEPTON_VETO")) {
      return;
    }
    int n_leptons = electrons.size() + muons.size() + taus.size();
    if (n_leptons == 0) MonojetCuts("LEPTON_VETO").passed = true;
  }

  void MonojetSelectorAlg::evaluateElectronVeto(const xAOD::ElectronContainer& electrons,
    CutManager& MonojetCuts)
  {
    if (!MonojetCuts.exists("ELECTRON_VETO")) {
      return;
    }
    if (electrons.size() == 0) MonojetCuts("ELECTRON_VETO").passed = true;
  }

    void MonojetSelectorAlg::evaluateMuonVeto(const xAOD::MuonContainer& muons,
    CutManager& MonojetCuts)
  {
    if (!MonojetCuts.exists("MUON_VETO")) {
      return;
    }
    if (muons.size() == 0) MonojetCuts("MUON_VETO").passed = true;
  }

  void MonojetSelectorAlg::evaluateTauVeto(
    const xAOD::TauJetContainer& taus,
    CutManager& MonojetCuts)
    {
    if (!MonojetCuts.exists("TAU_VETO")) {
      return;
    }
    if (taus.size() == 0) MonojetCuts("TAU_VETO").passed = true;
  }

  StatusCode MonojetSelectorAlg::initialiseCutflow(){
    m_MonojetCuts.CheckInputCutList(m_inputCutList,m_STANDARD_CUTS);

    for (const std::string &cut : m_inputCutList)  {
      // Initialize a vector of CutEntry structs based on the input Cut List
      m_MonojetCuts.add(cut);
    }
    //After filling the CutManager, book your histograms.
    const unsigned int nbins = m_MonojetCuts.size() + 1; //  need an extra bin for the total num of events.
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