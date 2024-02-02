/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "SelectionFlagsbbyyAlg.h"

namespace HHBBYY
{

  SelectionFlagsbbyyAlg::SelectionFlagsbbyyAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("cutList", m_inputCutList);
    declareProperty("saveCutFlow",m_saveCutFlow);
    declareProperty("photonTriggers",m_photonTriggers);
  }


  StatusCode SelectionFlagsbbyyAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("      SelectionFlagsbbyyAlg      \n");
    ATH_MSG_INFO("*********************************\n");

    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    ATH_CHECK (m_photonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK (m_mcEventWeightsKey.initialize());

    //Initialize trigger decorations
    for (const std::string &trig : m_photonTriggers)
    {
      std::string triggerDecorName = "trigPassed_"+trig;
      SG::ReadDecorHandleKey< xAOD::EventInfo > triggerDecorKey = m_eventHandle.getNamePattern() + "." + triggerDecorName;
      m_triggerDecorKeys.emplace(trig,triggerDecorKey);
      ATH_CHECK(m_triggerDecorKeys.at(trig).initialize());
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

    m_bbyyCuts.CheckInputCutList(m_inputCutList,m_STANDARD_CUTS);

    for (const std::string &cut : m_inputCutList)  { 
      // Initialize a vector of CutEntry structs based on the input Cut List
      m_bbyyCuts.add(cut);
    }

    //After filling the CutManager, book your histograms.
    const unsigned int nbins = m_bbyyCuts.size() + 1; //  need an extra bin for the total num of events.
    ANA_CHECK (book (TEfficiency("AbsoluteEfficiency","Absolute Efficiency of HH->bbyy cuts;Cuts;#epsilon", 
                                  nbins, 0.5, nbins + 0.5))); 
    ANA_CHECK (book (TEfficiency("RelativeEfficiency","Relative Efficiency of HH->bbyy cuts;Cuts;#epsilon", 
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("StandardCutFlow","StandardCutFlow of HH->bbyy cuts;Cuts;#epsilon", 
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("WeightedAbsoluteEfficiency","Weighted Absolute Efficiency of HH->bbyy cuts;Cuts;#epsilon", 
                                  nbins, 0.5, nbins + 0.5))); 
    ANA_CHECK (book (TEfficiency("WeightedRelativeEfficiency","Weighted Relative Efficiency of HH->bbyy cuts;Cuts;#epsilon", 
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("WeightedStandardCutFlow","Weighted StandardCutFlow of HH->bbyy cuts;Cuts;#epsilon", 
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TH1F("EventsPassed_BinLabeling", "Events passed by each cut / Bin labeling", nbins, 0.5, nbins + 0.5)));  

    return StatusCode::SUCCESS;
  }


  StatusCode SelectionFlagsbbyyAlg::execute()
  {

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      // Retrive inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::PhotonContainer *photons = nullptr;
      ANA_CHECK (m_photonHandle.retrieve (photons, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      if(m_isMC) {
        SG::ReadDecorHandle<xAOD::EventInfo, std::vector<float>> mcEventWeightsHandle(m_mcEventWeightsKey);
        eventWeights = mcEventWeightsHandle(*event);
      }

      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      for(const xAOD::Jet* jet : *jets) {
        if (WPgiven) {
          if (m_isBtag.get(*jet, sys)) bjets->push_back(jet);
        }
      }

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));
      
      // reset all cut flags to default=false
      for (CutEntry& cut : m_bbyyCuts) {
        cut.passed = false;
        m_Bbranches.at(cut.name).set(*event, cut.passed, sys);
      }
    
      if (!m_photonTriggers.empty()) {
        evaluateTriggerCuts(*event, m_photonTriggers, m_bbyyCuts);
      }

      evaluatePhotonCuts(*photons, m_bbyyCuts);
      evaluateLeptonCuts(*electrons, *muons, m_bbyyCuts);
      evaluateJetCuts(*bjets, *jets, m_bbyyCuts);

      bool passedall = true;
      for (CutEntry& cut : m_bbyyCuts) {
        passedall = passedall && cut.passed;
        m_Bbranches.at(cut.name).set(*event, cut.passed, sys);
      }
      m_passallcuts.set(*event, passedall, sys);

      // do the CUTFLOW only with sys="" -> NOSYS
      if (sys.name()!="") continue;

      // Compute total_events
      m_total_events+=1; 
      if(m_isMC) m_total_mcEventWeight+= eventWeights.at(0);


      // Count how many cuts the event passed and increase the relative counter
      for (const auto &cut : m_inputCutList) {
        if(m_bbyyCuts.exists(cut)) {
          if (m_bbyyCuts(cut).passed) {
            m_bbyyCuts(cut).counter+=1;
            if(m_isMC) m_bbyyCuts(cut).w_counter += eventWeights.at(0);
          }
        }
      }

      // Check how many consecutive cuts are passed by the event.
      unsigned int consecutive_cuts = 0;
      for (size_t i = 0; i < m_bbyyCuts.size(); ++i) {
        if (m_bbyyCuts[i].passed)
          consecutive_cuts++;
        else
          break;
      }

      // Here we basically increment the  N_events(pass_i  AND pass_i-1  AND ... AND pass_0) for the i-cut.
      // I think this is an elegant way to do it :) . Considering the difficulties a configurable cut list imposes. 
      for (unsigned int i=0; i<consecutive_cuts; i++) {
        m_bbyyCuts[i].relativeCounter+=1;
        if(m_isMC) m_bbyyCuts[i].w_relativeCounter += eventWeights.at(0);
      }

    }

    return StatusCode::SUCCESS;
  }

  StatusCode SelectionFlagsbbyyAlg::finalize()
  {

    //adapt the following for each syst TODO
    ATH_MSG_INFO("Total events = " << m_total_events <<std::endl);
    m_bbyyCuts.CheckCutResults(); // Print CheckCutResults

    if(m_saveCutFlow) {
      m_bbyyCuts.DoAbsoluteEfficiency(m_total_events, efficiency("AbsoluteEfficiency"));
      m_bbyyCuts.DoRelativeEfficiency(m_total_events, efficiency("RelativeEfficiency"));
      m_bbyyCuts.DoStandardCutFlow(m_total_events, efficiency("StandardCutFlow"));
      m_bbyyCuts.DoWeightedAbsoluteEfficiency(m_total_mcEventWeight, efficiency("WeightedAbsoluteEfficiency"));
      m_bbyyCuts.DoWeightedRelativeEfficiency(m_total_mcEventWeight, efficiency("WeightedRelativeEfficiency"));
      m_bbyyCuts.DoWeightedStandardCutFlow(m_total_mcEventWeight, efficiency("WeightedStandardCutFlow"));
      m_bbyyCuts.DoCutflowLabeling(m_total_events, hist("EventsPassed_BinLabeling"));
    }
    else {
      delete efficiency("AbsoluteEfficiency");
      delete efficiency("RelativeEfficiency");
      delete efficiency("StandardCutFlow");
      delete efficiency("WeightedAbsoluteEfficiency");
      delete efficiency("WeightedRelativeEfficiency");
      delete efficiency("WeightedStandardCutFlow");
      delete hist("EventsPassed_BinLabeling");
    }


     return StatusCode::SUCCESS;

  }

  void SelectionFlagsbbyyAlg::evaluateTriggerCuts(const xAOD::EventInfo& event, const std::vector<std::string> &photonTriggers, 
                                                  CutManager& bbyyCuts) {

    if (!bbyyCuts.exists("PASS_TRIGGER"))
        return;

    for (const std::string &trigger : photonTriggers)
    {
      SG::ReadDecorHandleKey<xAOD::EventInfo>& triggerDecorKey = m_triggerDecorKeys.at(trigger);
      SG::ReadDecorHandle<xAOD::EventInfo, bool> m_triggerDecorHandle(triggerDecorKey);
      //If the event passes any of the available (single or di-) photon triggers, set the overall trigger cut to true.
      if (m_triggerDecorHandle(event)) {
        bbyyCuts("PASS_TRIGGER").passed = true;
        break;
      }
    }

  }

  void SelectionFlagsbbyyAlg::evaluatePhotonCuts
  (const xAOD::PhotonContainer& photons, CutManager& bbyyCuts)
  {
    if (bbyyCuts.exists("TWO_TIGHTID_ISO_PHOTONS"))
      bbyyCuts("TWO_TIGHTID_ISO_PHOTONS").passed = (photons.size() == 2);

    // photon isolation and selection pT/myy
    if (photons.size() >= 2)
    {
      double myy = (photons.at(0)->p4() + photons.at(1)->p4()).M();
      std::vector<float> ptOverMasses;

      for (const xAOD::Photon* photon : {photons.at(0), photons.at(1)})
      {
        ptOverMasses.push_back(photon->pt() / myy);
      }

      if (ptOverMasses[0] > 0.35 && ptOverMasses[1] > 0.25 && bbyyCuts.exists("PASS_RELPT"))
        bbyyCuts("PASS_RELPT").passed = true;
      if (myy >= 105000. && myy < 160000. && bbyyCuts.exists("DIPHOTON_MASS"))
        bbyyCuts("DIPHOTON_MASS").passed = true;
    }
  }


  void SelectionFlagsbbyyAlg::evaluateLeptonCuts
  (const xAOD::ElectronContainer& electrons, const xAOD::MuonContainer& muons,
   CutManager& bbyyCuts)
  {

    if (!bbyyCuts.exists("EXACTLY_ZERO_LEPTONS"))
      return;

    // No medium+isolated electrons and muons.
    int n_leptons = electrons.size() + muons.size();
    if (n_leptons==0)
      bbyyCuts("EXACTLY_ZERO_LEPTONS").passed = true;

  }

  void SelectionFlagsbbyyAlg::evaluateJetCuts(const ConstDataVector<xAOD::JetContainer>& bjets,
                            const xAOD::JetContainer& jets, CutManager& bbyyCuts)
  {
    int CentralJets=0;

    ///All jets in the containers should have pT>25GeV. Check minPt of your JetSelectorAlg in the bbyy_config file.
    for (const xAOD::Jet *jet : jets)
    {// Jets here can be every type of jet (No Working point selected)
      // check if jet is central
      if(std::abs(jet->eta())<2.5)
        CentralJets+=1;
    }

    if (CentralJets<6 && bbyyCuts.exists("LESS_THAN_SIX_CENTRAL_JETS"))
      bbyyCuts("LESS_THAN_SIX_CENTRAL_JETS").passed = true;

    // If Forward Jets + Central jets >=2 --> The event passes.
    if (jets.size() >= 2 && bbyyCuts.exists("AT_LEAST_TWO_JETS"))
      bbyyCuts("AT_LEAST_TWO_JETS").passed = true;

    if (bjets.size()>=1 && bbyyCuts.exists("AT_LEAST_ONE_B_JET"))
      bbyyCuts("AT_LEAST_ONE_B_JET").passed = true;
    if (bjets.size()==1 && bbyyCuts.exists("EXACTLY_ONE_B_JET"))
      bbyyCuts("EXACTLY_ONE_B_JET").passed = true;
    if (bjets.size()>=2 && bbyyCuts.exists("AT_LEAST_TWO_B_JETS"))
      bbyyCuts("AT_LEAST_TWO_B_JETS").passed = true;
    if (bjets.size()==2 && bbyyCuts.exists("EXACTLY_TWO_B_JETS")) 
      bbyyCuts("EXACTLY_TWO_B_JETS").passed = true;

  }

}
