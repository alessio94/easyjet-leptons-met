/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MultileptonSelectorAlg.h"
#include <AthenaKernel/Units.h>
#include <AthContainers/ConstDataVector.h>
#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>

namespace MULTILEPTON
{
  MultileptonSelectorAlg::MultileptonSelectorAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : EL::AnaAlgorithm(name, pSvcLocator)
  {
  }


  StatusCode MultileptonSelectorAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("     MultileptonSelectorAlg      \n");
    ATH_MSG_INFO("*********************************\n");

    // Initialise global event filter
    ATH_CHECK (m_filterParams.initialize(m_systematicsList));
    
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_tauHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));    

    if (m_saveCutFlow) ATH_CHECK (m_generatorWeight.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK(m_year.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK (m_matchingTool.retrieve());

    // Intialise booleans with value false. Also initialise syst-aware output decorators
    for (auto& [key, value] : m_boolnames) {
      m_bools.emplace(key, false);
      CP::SysWriteDecorHandle<bool> whandle{value+"_%SYS%", this};
      m_Bbranches.emplace(key, whandle);
      ATH_CHECK(m_Bbranches.at(key).initialize(m_systematicsList, m_eventHandle));
    };

    // make trigger decorators
    for (auto trig : m_triggers){
      CP::SysReadDecorHandle<bool> deco {this, "trig"+trig, trig, "Name of trigger"};
      m_triggerdecos.emplace(trig, deco);
      ATH_CHECK(m_triggerdecos.at(trig).initialize(m_systematicsList, m_eventHandle));
    }

    // Initialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    if (m_saveCutFlow) ATH_CHECK (initialiseCutflow());

    return StatusCode::SUCCESS;
  }


  StatusCode MultileptonSelectorAlg::execute()
  {
    // Global filter originally false
    CP::SysFilterReporterCombiner filterCombiner (m_filterParams, false);

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector()) {

      CP::SysFilterReporter filter (filterCombiner, sys);

      // Retrieve inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      auto nonbjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      for(const xAOD::Jet* jet : *jets) {
        if (WPgiven) {
          if (m_isBtag.get(*jet, sys) && std::abs(jet->eta())<2.5) bjets->push_back(jet);
          else nonbjets->push_back(jet);
        }
      }

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      const xAOD::TauJetContainer *taus = nullptr;
      ANA_CHECK (m_tauHandle.retrieve (taus, sys));


      for (const auto& [key, value] : m_bools) {
        m_bools.at(key) = false;
      }

      setThresholds(event, sys);

      // TODO: apply baseline selection for objects

      evaluateTriggerCuts(event, electrons, muons, taus, m_hhmlCuts, sys);
      applyChannelSelection(*electrons, *muons, *taus, *bjets, m_hhmlCuts);

      bool pass_baseline=false;
      if(m_bools.at(MULTILEPTON::PASS_TRIGGER)) pass_baseline=true;

      bool pass_selection = false;
      for (const auto& [key, value] : m_bools) {
        if (key == MULTILEPTON::PASS_TRIGGER ||
            key == pass_trigger_SLT ||
            key == pass_trigger_DLT ||
            key == pass_baseline_tau_trigger)
          continue;
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
          if (m_hhmlCuts.exists(cutname)) {
            m_hhmlCuts(cutname).passed = m_bools.at(cut);
            if (m_hhmlCuts(cutname).passed) {
              m_hhmlCuts(cutname).counter += 1;
              if(m_isMC) m_hhmlCuts(cutname).w_counter += m_generatorWeight.get(*event, sys);
            }
          }
        }

        // Check how many consecutive cuts are passed by the event.
        unsigned int consecutive_cuts = 0;
        for (size_t i = 0; i < m_hhmlCuts.size(); ++i) {
          if (m_hhmlCuts[i].passed)
            consecutive_cuts++;
          else
            break;
        }

        // Here we basically increment the  N_events(pass_i  AND pass_i-1  AND ... AND pass_0) for the i-cut.
        for (unsigned int i=0; i<consecutive_cuts; i++) {
          m_hhmlCuts[i].relativeCounter += 1;
          if(m_isMC) m_hhmlCuts[i].w_relativeCounter += m_generatorWeight.get(*event, sys);
        }
      }

      // Fill syst-aware output decorators
      for (auto& [key, var] : m_bools) {
        m_Bbranches.at(key).set(*event, var, sys);
      };

      if (!m_bypass && !pass_baseline) continue;

      // Global event filter true if any syst passes and controls
      // if event is passed to output writing or not
      filter.setPassed(true);
    }
    return StatusCode::SUCCESS;
  }

  StatusCode MultileptonSelectorAlg::finalize()
  {
    //adapt the following for each syst TODO
    ATH_MSG_INFO("Total events = " << m_total_events <<std::endl);
    ANA_CHECK (m_filterParams.finalize ());

    m_hhmlCuts.CheckCutResults(); // Print CheckCutResults

    if(m_saveCutFlow) {
      m_hhmlCuts.DoAbsoluteEfficiency(m_total_events, efficiency("AbsoluteEfficiency"));
      m_hhmlCuts.DoRelativeEfficiency(m_total_events, efficiency("RelativeEfficiency"));
      m_hhmlCuts.DoStandardCutFlow(m_total_events, efficiency("StandardCutFlow"));
      if(m_isMC) {
        m_hhmlCuts.DoWeightedAbsoluteEfficiency(m_total_mcEventWeight, efficiency("WeightedAbsoluteEfficiency"));
        m_hhmlCuts.DoWeightedRelativeEfficiency(m_total_mcEventWeight, efficiency("WeightedRelativeEfficiency"));
        m_hhmlCuts.DoWeightedStandardCutFlow(m_total_mcEventWeight, efficiency("WeightedStandardCutFlow"));
      }
      m_hhmlCuts.DoCutflowLabeling(m_total_events, hist("EventsPassed_BinLabeling"));
    }

    return StatusCode::SUCCESS;
  }


  StatusCode MultileptonSelectorAlg::initialiseCutflow()
  {
    // CutFlow
    std::vector<std::string> boolnamelist;
    for (const auto& [key, value]: m_boolnames) {
      boolnamelist.push_back(value);
    }
    m_hhmlCuts.CheckInputCutList(m_inputCutList,boolnamelist);

    // Initialize an array containing the enum values needed for the cutlist
    m_inputCutKeys.resize(m_inputCutList.size());
    std::vector<bool> inputWasFound (m_inputCutList.size(), false);
    for (const auto& [key, value]: m_boolnames) {
      auto it = std::find(m_inputCutList.begin(), m_inputCutList.end(), value);
      if(it != m_inputCutList.end()) {
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
      m_hhmlCuts.add(m_boolnames[cut]);
    }

    //After filling the CutManager, book your histograms.
    const unsigned int nbins = m_hhmlCuts.size() + 1; //  need an extra bin for the total num of events.
    ANA_CHECK (book (TEfficiency("AbsoluteEfficiency","Absolute Efficiency of HH->multilepton cuts;Cuts;#epsilon",
			nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("RelativeEfficiency","Relative Efficiency of HH->multilepton cuts;Cuts;#epsilon",
			nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("StandardCutFlow","StandardCutFlow of HH->multilepton cuts;Cuts;#epsilon",
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

  void MultileptonSelectorAlg::evaluateTriggerCuts(
    const xAOD::EventInfo* event,
    const xAOD::ElectronContainer* electrons, const xAOD::MuonContainer *muons,
    const xAOD::TauJetContainer* taus,
    CutManager& hhmlCuts, const CP::SystematicSet& sys){

    if (!hhmlCuts.exists("PASS_TRIGGER"))
        return;

    if (electrons || muons){ 
      evaluateSingleLeptonTrigger(event, electrons, muons, sys);
      evaluateDiLeptonTrigger(event, electrons, muons, sys);
    }
    if (taus) evaluateBaselineTauTrigger(event, electrons, muons, taus, sys);

    if (m_bools.at(MULTILEPTON::pass_trigger_SLT) || m_bools.at(MULTILEPTON::pass_trigger_DLT) || m_bools.at(MULTILEPTON::pass_baseline_tau_trigger)) m_bools.at(MULTILEPTON::PASS_TRIGGER) = true;
  }

  void MultileptonSelectorAlg::evaluateSingleLeptonTrigger(
    const xAOD::EventInfo* event,
    const xAOD::ElectronContainer* electrons, const xAOD::MuonContainer *muons,
    const CP::SystematicSet& sys){

    // Check single electron triggers
    std::vector<std::string> single_ele_paths;

    int year = m_year.get(*event, sys);
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
    // TODO: Add 2022 and 2023 triggers
    // else if(m_is22_75bunches.get(*event, sys)){
    //   single_ele_paths = {
    //     "HLT_e17_lhvloose_L1EM15VHI", "HLT_e20_lhvloose_L1EM15VH",
    //     "HLT_e250_etcut_L1EM22VHI"
    //   };
    // }
    // else if(year==2022){
    //   single_ele_paths = {
    //     "HLT_e26_lhtight_ivarloose_L1EM22VHI", "HLT_e60_lhmedium_L1EM22VHI",
    //     "HLT_e140_lhloose_L1EM22VHI", "HLT_e300_etcut_L1EM22VHI"
    //   };
    // }
    // else if(m_is23_75bunches.get(*event, sys)){
    //   single_ele_paths = {
    //     "HLT_e26_lhtight_ivarloose_L1EM22VHI", "HLT_e60_lhmedium_L1EM22VHI",
    //     "HLT_e140_lhloose_L1EM22VHI", "HLT_e140_lhloose_noringer_L1EM22VHI",
    //     "HLT_e300_etcut_L1EM22VHI"
    //   };
    // }
    // else if(year==2023){
    //   single_ele_paths = {
    //     "HLT_e26_lhtight_ivarloose_L1eEM26M", "HLT_e60_lhmedium_L1eEM26M",
    //     "HLT_e140_lhloose_L1eEM26M", "HLT_e140_lhloose_noringer_L1eEM26M",
    //     "HLT_e300_etcut_L1eEM26M"
    //   };
    // }

    bool trigPassed_SET = false;
    if (electrons){
      for (const auto& trig : single_ele_paths){
        bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
        if (pass){
          for (const auto& ele : *electrons){
            bool match = m_matchingTool->match(*ele, trig);
            trigPassed_SET |= match && ele->pt() > m_pt_threshold[MULTILEPTON::SLT][MULTILEPTON::ele];
          }
        }
      }
    }

    // Check single muon triggers
    std::vector<std::string> single_mu_paths;

    if(year==2015){
      single_mu_paths = {"HLT_mu20_iloose_L1MU15", "HLT_mu50"};
    }
    else if(2016<=year && year<=2018){
      single_mu_paths = {"HLT_mu26_ivarmedium", "HLT_mu50"};
    }
    // TODO: Add 2022 and 2023 triggers
    // else if(2022<=year && year<=2023 &&
	  //   !m_is22_75bunches.get(*event, sys) &&
	  //   !m_is23_75bunches.get(*event, sys) &&
	  //   !m_is23_400bunches.get(*event, sys)){
    //   single_mu_paths = {
    //     "HLT_mu24_ivarmedium_L1MU14FCH", "HLT_mu50_L1MU14FCH",
    //     "HLT_mu60_0eta105_msonly_L1MU14FCH", "HLT_mu60_L1MU14FCH",
    //     "HLT_mu80_msonly_3layersEC_L1MU14FCH"
    //   };
    // }

    bool trigPassed_SMT = false;
    if (muons){
      for(const auto& trig : single_mu_paths){
        bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
        if (pass){
          for (const auto& mu : *muons){
            bool match = m_matchingTool->match(*mu, trig);
            trigPassed_SMT |= match && mu->pt() > m_pt_threshold[MULTILEPTON::SLT][MULTILEPTON::mu];
          }
        }
      }
    }

    m_bools.at(MULTILEPTON::pass_trigger_SLT) |= (trigPassed_SET || trigPassed_SMT);
  }

  void MultileptonSelectorAlg::evaluateDiLeptonTrigger(
    const xAOD::EventInfo* event,
    const xAOD::ElectronContainer* electrons, const xAOD::MuonContainer *muons,
    const CP::SystematicSet& sys){

    std::vector<std::string> di_ele_paths;

    int year = m_year.get(*event, sys);
    if(year==2015){
      di_ele_paths = {"HLT_2e12_lhloose_L12EM10VH"};
    }
    else if(year==2016){
      di_ele_paths = {"HLT_2e17_lhvloose_nod0"};
    }
    else if(2017<=year && year<=2018){
      di_ele_paths = {
        "HLT_2e24_lhvloose_nod0"
      };
    }
    // TODO: Add 2022 and 2023 triggers
    // else if(year==2022){
    //   di_ele_paths = {
    //     "HLT_2e17_lhvloose_L12EM15VHI", "HLT_2e24_lhvloose_L12EM20VH"
    //   };
    // }
    // else if(year==2023){
    //   di_ele_paths = {
    //     "HLT_2e17_lhvloose_L12eEM18M", "HLT_2e24_lhvloose_L12eEM24L"
    //   };
    // }

    bool trigPassed_DET = false;
    if (electrons && electrons->size() >= 2){
      for (const auto &trig : di_ele_paths){
        bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
        if (pass){
          for (const auto &ele0: *electrons){
            for (const auto &ele1: *electrons){
              if (ele0 == ele1) continue;
              bool match = m_matchingTool->match({ele0, ele1}, trig);
              bool pass_cut = (
                (ele0->pt() > m_pt_threshold[MULTILEPTON::DLT][MULTILEPTON::leadingele] &&
                 ele1->pt() > m_pt_threshold[MULTILEPTON::DLT][MULTILEPTON::subleadingele]) || 
                (ele1->pt() > m_pt_threshold[MULTILEPTON::DLT][MULTILEPTON::leadingele] && 
                 ele0->pt() > m_pt_threshold[MULTILEPTON::DLT][MULTILEPTON::subleadingele])
              );
              trigPassed_DET |= match && pass_cut;
            } // ele1 loop end
          } // ele0 loop end
        } // pass check end
      } // trigger loop end
    }

    // Check di-muon triggers
    std::vector<std::string> di_mu_paths;

    if(year==2015){
      di_mu_paths = {"HLT_mu18_mu8noL1"};
    }
    else if(2016<=year && year<=2018){
      di_mu_paths = {"HLT_mu22_mu8noL1"};
    }
    // TODO: Add 2022 and 2023 triggers
    // else if(2022<=year && year<=2023){
    //   di_mu_paths = {"HLT_mu22_mu8noL1_L1MU14FCH", "HLT_2mu14_L12MU8F"};
    // }

    bool trigPassed_DMT = false;
    if (muons && muons->size() >= 2){
      for (const auto &trig : di_mu_paths){
        bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
        if (pass){
          for (const auto &mu0: *muons){
            for (const auto &mu1: *muons){
              if (mu0 == mu1) continue;
              bool match = m_matchingTool->match({mu0, mu1}, trig);
              bool pass_cut = (
                (mu0->pt() > m_pt_threshold[MULTILEPTON::DLT][MULTILEPTON::leadingmu] &&
                 mu1->pt() > m_pt_threshold[MULTILEPTON::DLT][MULTILEPTON::subleadingmu]) || 
                (mu1->pt() > m_pt_threshold[MULTILEPTON::DLT][MULTILEPTON::leadingmu] && 
                 mu0->pt() > m_pt_threshold[MULTILEPTON::DLT][MULTILEPTON::subleadingmu])
              );
              trigPassed_DMT |= match && pass_cut;
            } // mu1 loop end
          } // mu0 loop end
        } // pass check end
      } // trigger loop end
    }

    // Check electron-muon triggers
    std::vector<std::string> emu_paths;
    if(year==2015){
      emu_paths = {"HLT_e17_lhloose_mu14"};
    }
    else if(2016<=year && year<=2018){
      emu_paths = {"HLT_e17_lhloose_nod0_mu14"};
    }
    // TODO: Add 2022 and 2023 triggers
    // else if(2022<=year && year<=2023){
    //   emu_paths = {"HLT_e17_lhloose_mu14_L1EM15VH_MU8F"};
    // }
    bool trigPassed_EMT = false;
    if (electrons && muons){
      for(const auto& trig : emu_paths){
        bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
        if (pass){
          for(const auto& ele : *electrons){
            for(const auto& mu : *muons){
              bool match = m_matchingTool->match(*ele, trig) && m_matchingTool->match(*mu, trig);
              bool pass_cut = (
                  ele->pt() > m_pt_threshold[MULTILEPTON::ASLT][MULTILEPTON::leadingele] &&
                  mu->pt() > m_pt_threshold[MULTILEPTON::ASLT][MULTILEPTON::leadingmu]);
              trigPassed_EMT |= match && pass_cut;
            }
          }
        }
      }
    }

    m_bools.at(MULTILEPTON::pass_trigger_DLT) = (trigPassed_DET || trigPassed_DMT || trigPassed_EMT);
  }

  void MultileptonSelectorAlg::evaluateBaselineTauTrigger(
    const xAOD::EventInfo* event,
    const xAOD::ElectronContainer* electrons, const xAOD::MuonContainer *muons,
    const xAOD::TauJetContainer* taus,
    const CP::SystematicSet& sys){

    int year = m_year.get(*event, sys);

    if (!taus) return;

    // Single tau trigger
    std::vector<std::string> single_tau_paths;
    if (year==2015){
      single_tau_paths = {
        "HLT_tau80_medium1_tracktwo_L1TAU60",
      };
    }
    else if(year==2016){
      single_tau_paths = {
        "HLT_tau80_medium1_tracktwo_L1TAU60",
        "HLT_tau125_medium1_tracktwo",
        "HLT_tau160_medium1_tracktwo",
      };
    }
    else if(year==2017){
      single_tau_paths = {
        "HLT_tau160_medium1_tracktwo",
        "HLT_tau160_medium1_tracktwo_L1TAU100",
      };
    }
    else if(year==2018){
      single_tau_paths = {
        "HLT_tau160_medium1_tracktwoEF_L1TAU100",
        "HLT_tau160_mediumRNN_tracktwoMVA_L1TAU100",
      };
    }

    bool trigPassed_STT = false;
    for(const auto& trig : single_tau_paths){
      bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
      if (pass){
        for(const auto& tau : *taus){
          bool match = m_matchingTool->match(*tau, trig, 0.2);
          trigPassed_STT |= match;
        }
      }
    }

    // Di-tau trigger
    std::vector<std::string> di_tau_paths;
    if (year==2015){
      di_tau_paths = {
        "HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1TAU20IM_2TAU12IM",
      };
    }
    else if(year==2016){
      di_tau_paths = {
        "HLT_tau35_loose1_tracktwo_tau25_loose1_tracktwo",
        "HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo",
        "HLT_tau80_medium1_TAU60_tau50_medium1_L1TAU12",
      };
    }
    else if(year==2017){
      di_tau_paths = {
        // can we use "HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo"? 
        // Cause it is described as "di-tau+L1 jets" in https://twiki.cern.ch/twiki/bin/viewauth/Atlas/LowestUnprescaled?sortcol=2;table=77;up=0#sorted_table
        "HLT_tau80_medium1_tracktwo_L1TAU60_tau50_medium1_tracktwo_L1TAU12",
        "HLT_tau80_medium1_tracktwo_L1TAU60_tau60_medium1_tracktwo_L1TAU40",
      };
    }
    else if(year==2018){
      di_tau_paths = {
        "HLT_tau80_medium1_tracktwoEF_L1TAU60_tau60_medium1_tracktwoEF_L1TAU40",
        "HLT_tau80_mediumRNN_tracktwoMVA_L1TAU60_tau60_mediumRNN_tracktwoMVA_L1TAU40",
      };
    }
    bool trigPassed_DTT = false;
    if (taus->size() >= 2){
      for(const auto& trig : di_tau_paths){
        bool pass = false;
        if (m_triggerdecos.contains("trigPassed_"+trig)){
          pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
        } else {
          ATH_MSG_WARNING("Trigger " << trig << " not found. Skipping.");
        }

        if (pass){
          for (const auto& tau0 : *taus){
            for (const auto& tau1 : *taus){
              if (tau0 == tau1) continue;
              bool match = m_matchingTool->match({tau0,tau1}, trig, 0.2);
              trigPassed_DTT |= match;
            }
          }
        }
      }
    }

    // electron-tau trigger
    std::vector<std::string> ele_tau_paths;
    if (year==2015){
      ele_tau_paths = {
        "HLT_e17_lhmedium_nod0_tau25_medium1_tracktwo",
      };
    }
    else if(year==2016){
      ele_tau_paths = {
        "HLT_e17_lhmedium_nod0_tau25_medium1_tracktwo",
        "HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwo",
      };
    }
    else if(year==2017){
      ele_tau_paths = {
        "HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwo",
        "HLT_e24_lhmedium_nod0_ivarloose_tau35_medium1_tracktwo",
      };
    }
    else if(year==2018){
      ele_tau_paths = {
        "HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwoEF",
        "HLT_e17_lhmedium_nod0_ivarloose_tau25_mediumRNN_tracktwoMVA",
      };
    }

    bool trigPassed_ETT = false;
    if (electrons){
      for(const auto& trig : ele_tau_paths){
        bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
        if (pass){
          for (const auto& ele : *electrons){
            for (const auto& tau : *taus){
              bool match = m_matchingTool->match(*ele, trig) && m_matchingTool->match(*tau, trig, 0.2);
              bool pass_cut = (
                  ele->pt() > m_pt_threshold[MULTILEPTON::ETT][MULTILEPTON::ele] &&
                  tau->pt() > m_pt_threshold[MULTILEPTON::ETT][MULTILEPTON::tau]);
              trigPassed_ETT |= match && pass_cut;
            }
          }
        }
      }
    }

    // muon-tau trigger
    std::vector<std::string> mu_tau_paths;
    if (year==2015){
      mu_tau_paths = {
        "HLT_mu14_tau25_medium1_tracktwo",
      };
    }
    else if(year==2016){
      mu_tau_paths = {
        "HLT_mu14_tau25_medium1_tracktwo",
        "HLT_mu14_ivarloose_tau25_medium1_tracktwo",
      };
    }
    else if(year==2017){
      mu_tau_paths = {
        "HLT_mu14_ivarloose_tau25_medium1_tracktwo",
        "HLT_mu14_ivarloose_tau35_medium1_tracktwo",
      };
    }
    else if(year==2018){
      mu_tau_paths = {
        "HLT_mu14_ivarloose_tau35_medium1_tracktwoEF",
        "HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA",
      };
    }
    bool trigPassed_MTT = false;
    if (muons){
      for(const auto& trig : mu_tau_paths){
        bool pass = m_triggerdecos.at("trigPassed_"+trig).get(*event, sys);
        if (pass){
          for (const auto& mu : *muons){
            for (const auto& tau : *taus){
              bool match = m_matchingTool->match(*mu, trig) && m_matchingTool->match(*tau, trig, 0.2);
              bool pass_cut = (
                  mu->pt() > m_pt_threshold[MULTILEPTON::MTT][MULTILEPTON::mu] &&
                  tau->pt() > m_pt_threshold[MULTILEPTON::MTT][MULTILEPTON::tau]);
              trigPassed_MTT |= match && pass_cut;
            }
          }
        }
      }
    }
    m_bools.at(MULTILEPTON::pass_baseline_tau_trigger) = (trigPassed_STT || trigPassed_DTT || trigPassed_ETT || trigPassed_MTT);
  }


  bool MultileptonSelectorAlg::evaluate2lscSelection(
      const SubChannelClassify &classify,
        CutManager& hhmlCuts){
    auto sub_channel_id = classify.getSubChannelId();
    if (!hhmlCuts.exists("pass_2lsc") || sub_channel_id != CH_ID::hh2lsc) return false;
    bool pass_selection = true;

    // TODO: ID & prompt lepton isolation WP cut

    // Low mass veto
    pass_selection &= classify.check_low_mass(12. * Athena::Units::GeV);

    return pass_selection;
  }

  bool MultileptonSelectorAlg::evaluate3lSelection(
      const SubChannelClassify &classify,
        CutManager& hhmlCuts){

    auto sub_channel_id = classify.getSubChannelId();
    if (!hhmlCuts.exists("pass_3l") || sub_channel_id != CH_ID::hh3l) return false;
    bool pass_selection = true;

    // Low mass veto for all pairs
    pass_selection &= classify.check_low_mass(12. * Athena::Units::GeV);

    return pass_selection;
  }
        

  bool MultileptonSelectorAlg::evaluatebb4lSelection(
      const SubChannelClassify &classify,
      CutManager& hhmlCuts){

    auto sub_channel_id = classify.getSubChannelId();
    if (!hhmlCuts.exists("pass_bb4l") || sub_channel_id != CH_ID::hhbb4l) return false;

    bool pass_selection = true;

    return pass_selection;
  }

  bool MultileptonSelectorAlg::evaluate1l2tauSelection(
      const SubChannelClassify &classify,
      CutManager& hhmlCuts){

    auto sub_channel_id = classify.getSubChannelId();
    if (!hhmlCuts.exists("pass_1l2tau") || sub_channel_id != CH_ID::hh1l2tau) return false;
    bool pass_selection = true;

    // TODO: ID and PLV cut

    // TODO: tau object selection

    return pass_selection;
  }

    
  bool MultileptonSelectorAlg::evaluate2l2tauSelection(
      const SubChannelClassify &classify,
      CutManager& hhmlCuts){

    auto sub_channel_id = classify.getSubChannelId();
    if (!hhmlCuts.exists("pass_2l2tau") || sub_channel_id != CH_ID::hh2l2tau)
      return false;

    bool pass_selection = true;

    // TODO: ID and PLV cut

    // Low mass veto
    pass_selection &= classify.check_low_mass(12. * Athena::Units::GeV);

    // TODO: tau object selection

    return pass_selection;
  }

  bool MultileptonSelectorAlg::evaluate2lsc1tauSelection(
      const SubChannelClassify &classify, CutManager &hhmlCuts)
  {
    auto sub_channel_id = classify.getSubChannelId();
    if (!hhmlCuts.exists("pass_2lsc1tau") || sub_channel_id != CH_ID::hh2lsc1tau) return false;
    bool pass_selection = true;

    // TODO: ID and PLV cut

    // TODO: tau object selection

    return pass_selection;
  }

  bool MultileptonSelectorAlg::evaluate1l3tauSelection(
      const SubChannelClassify &classify, CutManager &hhmlCuts)
  {
    auto sub_channel_id = classify.getSubChannelId();
    if (!hhmlCuts.exists("pass_1l3tau") || sub_channel_id != CH_ID::hh1l3tau) return false;
    bool pass_selection = true;

    // TODO: ID and PLV cut

    // TODO: tau object selection

    return pass_selection;
  }

  bool MultileptonSelectorAlg::evaluate3l1tauSelection(
      const SubChannelClassify &classify, CutManager &hhmlCuts)
  {
    auto sub_channel_id = classify.getSubChannelId();
    if (!hhmlCuts.exists("pass_3l1tau") || sub_channel_id != CH_ID::hh3l1tau) return false;
    bool pass_selection = true;

    // TODO: ID and PLV cut

    // TODO: tau object selection

    return pass_selection;
  }

  void MultileptonSelectorAlg::applyChannelSelection(
        const xAOD::ElectronContainer& electrons,
        const xAOD::MuonContainer& muons,
        const xAOD::TauJetContainer& taus,
        const ConstDataVector<xAOD::JetContainer>& bjets,
        CutManager& hhmlCuts){

    auto classifier = SubChannelClassify(&muons, &electrons, &taus, &bjets);

    if (hhmlCuts.exists("pass_2lsc")){
      m_bools.at(MULTILEPTON::pass_2lsc) =
          evaluate2lscSelection(classifier, hhmlCuts);
    }
    if (hhmlCuts.exists("pass_3l")){
      m_bools.at(MULTILEPTON::pass_3l) = evaluate3lSelection(classifier, hhmlCuts);
    }
    if (hhmlCuts.exists("pass_bb4l")){
      m_bools.at(MULTILEPTON::pass_bb4l) = evaluatebb4lSelection(classifier, hhmlCuts);
    }
    if (hhmlCuts.exists("pass_1l2tau")){
      m_bools.at(MULTILEPTON::pass_1l2tau) =
          evaluate1l2tauSelection(classifier, hhmlCuts);
    }
    if (hhmlCuts.exists("pass_2l2tau")){
      m_bools.at(MULTILEPTON::pass_2l2tau) =
          evaluate2l2tauSelection(classifier, hhmlCuts);
    }
    if (hhmlCuts.exists("pass_2lsc1tau")){
      m_bools.at(MULTILEPTON::pass_2lsc1tau) = evaluate2lsc1tauSelection(classifier, hhmlCuts);
    }
    if (hhmlCuts.exists("pass_1l3tau")){
      m_bools.at(MULTILEPTON::pass_1l3tau) =
          evaluate1l3tauSelection(classifier, hhmlCuts);
    }
    if (hhmlCuts.exists("pass_3l1tau")){
      m_bools.at(MULTILEPTON::pass_3l1tau) =
          evaluate3l1tauSelection(classifier, hhmlCuts);
    }
  }


  void MultileptonSelectorAlg::setThresholds(const xAOD::EventInfo* event,
					const CP::SystematicSet& sys) {
    
    int year = m_year.get(*event, sys);

    // Single-lepton triggers
    // electron
    if(year==2015)
      m_pt_threshold[MULTILEPTON::SLT][MULTILEPTON::ele] = 25. * Athena::Units::GeV;
    // TODO: Add 2022 and 2023 triggers
    // 2022 75 bunches
    // else if(m_is22_75bunches.get(*event, sys))
    //   m_pt_threshold[MULTILEPTON::SLT][MULTILEPTON::ele] = 18. * Athena::Units::GeV;
    else
      m_pt_threshold[MULTILEPTON::SLT][MULTILEPTON::ele] = 27. * Athena::Units::GeV;

    // muon
    if(year==2015)
      m_pt_threshold[MULTILEPTON::SLT][MULTILEPTON::mu] = 21. * Athena::Units::GeV;
    else if(year>=2016 && year<=2018)
      m_pt_threshold[MULTILEPTON::SLT][MULTILEPTON::mu] = 27. * Athena::Units::GeV;
    // TODO: Add 2022 and 2023 triggers
    // else
    //   m_pt_threshold[MULTILEPTON::SLT][MULTILEPTON::mu] = 25. * Athena::Units::GeV;


    //Di-lepton triggers
    //ee
    if(year==2015) {
      m_pt_threshold[MULTILEPTON::DLT][MULTILEPTON::leadingele] = 13. * Athena::Units::GeV;
      m_pt_threshold[MULTILEPTON::DLT][MULTILEPTON::subleadingele] = 13. * Athena::Units::GeV;
    }
    else if(year==2016) {
      m_pt_threshold[MULTILEPTON::DLT][MULTILEPTON::leadingele] = 18. * Athena::Units::GeV;
      m_pt_threshold[MULTILEPTON::DLT][MULTILEPTON::subleadingele] = 18. * Athena::Units::GeV;
    } else {
      m_pt_threshold[MULTILEPTON::DLT][MULTILEPTON::leadingele] = 25. * Athena::Units::GeV;
      m_pt_threshold[MULTILEPTON::DLT][MULTILEPTON::subleadingele] = 25. * Athena::Units::GeV;
    }

    //mm
    if(year==2015) {
      m_pt_threshold[MULTILEPTON::DLT][MULTILEPTON::leadingmu] = 19. * Athena::Units::GeV;
      m_pt_threshold[MULTILEPTON::DLT][MULTILEPTON::subleadingmu] = 9. * Athena::Units::GeV;
    }
    else if(year>=2016 && year<=2018) {
      // TODO: why bbll set cut on 24 & 10 GeV?
      m_pt_threshold[MULTILEPTON::DLT][MULTILEPTON::leadingmu] = 23. * Athena::Units::GeV;
      m_pt_threshold[MULTILEPTON::DLT][MULTILEPTON::subleadingmu] = 9. * Athena::Units::GeV;
    } 
    // TODO: Add 2022 and 2023 triggers
    // else {
    //   m_pt_threshold[MULTILEPTON::DLT][MULTILEPTON::leadingmu] = 15. * Athena::Units::GeV;
    //   m_pt_threshold[MULTILEPTON::DLT][MULTILEPTON::subleadingmu] = 15. * Athena::Units::GeV;
    // }

    //Asymmetric Lepton triggers
    m_pt_threshold[MULTILEPTON::ASLT][MULTILEPTON::leadingele] = 18. * Athena::Units::GeV;
    m_pt_threshold[MULTILEPTON::ASLT][MULTILEPTON::leadingmu] = 15. * Athena::Units::GeV;


    // Lepton tau triggers
    // electron-tau
    m_pt_threshold[MULTILEPTON::ETT][MULTILEPTON::ele] = 18. * Athena::Units::GeV;
    m_pt_threshold[MULTILEPTON::ETT][MULTILEPTON::tau] = 26. * Athena::Units::GeV;

    // muon-tau
    m_pt_threshold[MULTILEPTON::MTT][MULTILEPTON::mu] = 15. * Athena::Units::GeV;
    if (year>=2015 && year<=2017)
      m_pt_threshold[MULTILEPTON::MTT][MULTILEPTON::tau] = 26. * Athena::Units::GeV;
    else if (year==2018)
      m_pt_threshold[MULTILEPTON::MTT][MULTILEPTON::tau] = 36. * Athena::Units::GeV;
  }


}

