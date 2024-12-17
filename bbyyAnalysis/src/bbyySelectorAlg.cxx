/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "bbyySelectorAlg.h"
#include <AthenaKernel/Units.h>

#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>


namespace HHBBYY
{

  bbyySelectorAlg::bbyySelectorAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("cutList", m_inputCutList);
    declareProperty("saveCutFlow",m_saveCutFlow);
    declareProperty("photonTriggers",m_photonTriggers);
  }


  StatusCode bbyySelectorAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("      bbyySelectorAlg      \n");
    ATH_MSG_INFO("*********************************\n");

    ATH_CHECK (m_filterParams.initialize(m_systematicsList));

    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }
    ATH_CHECK (m_runNumber.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_rdmRunNumber.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK (m_photonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    
    ATH_CHECK (m_generatorWeight.initialize(m_systematicsList, m_eventHandle));

    m_photonWPDecorHandle = CP::SysReadDecorHandle<char>
      ("baselineSelection_"+m_photonWPName+"_%SYS%", this);
    m_photonTNIWPDecorHandle = CP::SysReadDecorHandle<char>
      ("baselineSelection_"+m_photon_TightID_NonIso_WPName+"_%SYS%", this);
    m_photonLIWPDecorHandle = CP::SysReadDecorHandle<char>
      ("baselineSelection_"+m_photon_LooseID_Iso_WPName+"_%SYS%", this);

    ATH_CHECK(m_photonWPDecorHandle.initialize(m_systematicsList, m_photonHandle));
    ATH_CHECK(m_photonTNIWPDecorHandle.initialize(m_systematicsList, m_photonHandle));
    ATH_CHECK(m_photonLIWPDecorHandle.initialize(m_systematicsList, m_photonHandle));

    ATH_CHECK(m_selected_ph.initialize(m_systematicsList, m_photonHandle));
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
    // Intialise booleans with value false.
    for (auto& [key, value] : m_boolnames) {
      m_bools.emplace(key, false);
      CP::SysWriteDecorHandle<bool> var {value+"_%SYS%", this};
      m_Bbranches.emplace(value, var);
      ATH_CHECK(m_Bbranches.at(value).initialize(m_systematicsList, m_eventHandle));
    };
  
    // special flag for all cuts
    ATH_CHECK (m_passallcuts.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize()); 
    
    m_bbyyCuts.CheckInputCutList(m_inputCutList,m_STANDARD_CUTS);

    for (const std::string &cut : m_inputCutList)  { 
      // Initialize a vector of CutEntry structs based on the input Cut List
      m_bbyyCuts.add(cut);
    }

    // Intialise booleans with value false.
    for (auto& [key, value] : m_boolnames) {
      m_bools.emplace(key, false);
    };
    //After filling the CutManager, book your histograms.
    const unsigned int nbins = m_bbyyCuts.size() + 1; //  need an extra bin for the total num of events.
    ANA_CHECK (book (TEfficiency("AbsoluteEfficiency","Absolute Efficiency of HH->bbyy cuts.Needs rescaling to total events.;Cuts;#epsilon", 
                                  nbins, 0.5, nbins + 0.5))); 
    ANA_CHECK (book (TEfficiency("RelativeEfficiency","Relative Efficiency of HH->bbyy cuts.Needs rescaling to total events.;Cuts;#epsilon", 
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("StandardCutFlow","StandardCutFlow of HH->bbyy cuts.Needs rescaling to total events.;Cuts;#epsilon", 
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("WeightedAbsoluteEfficiency","Weighted Absolute Efficiency of HH->bbyy cuts.Needs rescaling to sumOfWeights.;Cuts;#epsilon", 
                                  nbins, 0.5, nbins + 0.5))); 
    ANA_CHECK (book (TEfficiency("WeightedRelativeEfficiency","Weighted Relative Efficiency of HH->bbyy cuts.Needs rescaling to sumOfWeights.;Cuts;#epsilon", 
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("WeightedStandardCutFlow","Weighted StandardCutFlow of HH->bbyy cuts.Needs rescaling to sumOfWeights.;Cuts;#epsilon", 
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TH1F("EventsPassed_BinLabeling", "Events passed by each cut / Bin labeling", nbins, 0.5, nbins + 0.5)));  

    return StatusCode::SUCCESS;
  }


  StatusCode bbyySelectorAlg::execute()
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
     
      // Set run number dependent quantities for nominal systematics
      if(sys.name()==""){
        unsigned int rdmNumber = m_isMC ? m_rdmRunNumber.get(*event, sys) : m_runNumber.get(*event,sys);
        setRunNumberQuantities(rdmNumber);
      }

      const xAOD::PhotonContainer *photons = nullptr;
      ANA_CHECK (m_photonHandle.retrieve (photons, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      for(const xAOD::Jet* jet : *jets) {
        if (WPgiven) {
          if (m_isBtag.get(*jet, sys) && std::abs(jet->eta())<2.5) bjets->push_back(jet);
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
        evaluateTriggerMatchingCuts(m_photonTriggers, photons, m_bbyyCuts);
      }
      m_Bbranches.at("pass_trigger_single_photon").set(*event, m_bools.at(HHBBYY::pass_trigger_single_photon), sys);
      m_Bbranches.at("pass_trigger_diphoton").set(*event, m_bools.at(HHBBYY::pass_trigger_diphoton), sys);

      m_Bbranches.at("pass_matching_trigger_single_photon").set(*event, m_bools.at(HHBBYY::pass_matching_trigger_single_photon), sys);
      m_Bbranches.at("pass_matching_trigger_diphoton").set(*event, m_bools.at(HHBBYY::pass_matching_trigger_diphoton), sys);

      if (m_bbyyCuts.exists("TWO_LOOSE_PHOTONS"))
      m_bbyyCuts("TWO_LOOSE_PHOTONS").passed = (photons->size() >= 2);

      const xAOD::Photon* photon1 = photons->size()>0 ? photons->at(0) : nullptr;
      const xAOD::Photon* photon2 = photons->size()>1 ? photons->at(1) : nullptr;
      std::vector<const xAOD::Photon*> sel_photons = {photon1, photon2};

      // count the number of TightID Iso photons
      int n_photons = 0;
      int n_TightID_NonIso_photons = 0;
      int n_LooseID_Iso_photons = 0;

      // Ensure a decoration is added for each photon in the collection
      // This is to ensure all photons are accessible during baseline variables 
      // definitions, not just the first two.
      for (const xAOD::Photon* photon : *photons){
        m_selected_ph.set(*photon, false, sys);
      }

      for (const xAOD::Photon* photon : sel_photons){
        if(!photon) break;
        bool passPhotonWP = m_photonWPDecorHandle.get(*photon, sys);
        bool passPhoton_TightID_NonIsoWP = m_photonTNIWPDecorHandle.get(*photon, sys);
        bool passPhoton_LooseID_IsoWP = m_photonLIWPDecorHandle.get(*photon, sys);

        m_selected_ph.set(*photon, passPhoton_TightID_NonIsoWP || passPhoton_LooseID_IsoWP, sys);

        if (passPhotonWP) n_photons++;
        if (passPhoton_TightID_NonIsoWP) n_TightID_NonIso_photons++;
        if (passPhoton_LooseID_IsoWP) n_LooseID_Iso_photons++;
      }



      if (n_TightID_NonIso_photons >=2 || n_LooseID_Iso_photons >=2 || n_photons >=2){
        evaluatePhotonCuts(sel_photons, n_TightID_NonIso_photons, n_LooseID_Iso_photons, n_photons, m_bbyyCuts);
      }
      evaluateLeptonCuts(*electrons, *muons, m_bbyyCuts);
      evaluateJetCuts(*bjets, *jets, m_bbyyCuts);

      bool passedall = true;
      for (CutEntry& cut : m_bbyyCuts) {
        passedall = passedall && cut.passed;
        if (not m_enableSinglePhotonTrigger and cut.name == "PASS_TRIGGER")
          passedall = passedall && m_bools.at(HHBBYY::pass_trigger_diphoton);
        else if (not m_enableSinglePhotonTrigger and cut.name == "PASS_TRIGGER_MATCHING")
          passedall = passedall && m_bools.at(HHBBYY::pass_matching_trigger_diphoton);
        m_Bbranches.at(cut.name).set(*event, cut.passed, sys);
      }
      m_passallcuts.set(*event, passedall, sys);

      // Global event filter true if any syst passes and controls
      // if event is passed to output writing or not
      if (m_bypass or passedall) filter.setPassed(true);

      // do the CUTFLOW only with sys="" -> NOSYS
      if (sys.name()!=m_specialSysWeight) continue;

      // Compute total_events
      m_total_events+=1; 
      if (m_isMC) m_total_mcEventWeight+=  m_generatorWeight.get(*event, sys);

      // Count how many cuts the event passed and increase the relative counter
      for (const auto &cut : m_inputCutList) {
        if(m_bbyyCuts.exists(cut)) {
          if (m_bbyyCuts(cut).passed) {
            bool pass = true;
            if (not m_enableSinglePhotonTrigger and cut == "PASS_TRIGGER") 
              pass = m_bools.at(HHBBYY::pass_trigger_diphoton);
            else if (not m_enableSinglePhotonTrigger and cut == "PASS_TRIGGER_MATCHING")
              pass = m_bools.at(HHBBYY::pass_matching_trigger_diphoton);
            m_bbyyCuts(cut).counter += pass;
            if (m_isMC) m_bbyyCuts(cut).w_counter += m_generatorWeight.get(*event, sys) * pass;
          }
        }
      }

      // Check how many consecutive cuts are passed by the event.
      unsigned int consecutive_cuts = 0;
      for (size_t i = 0; i < m_bbyyCuts.size(); ++i) {
        if (m_bbyyCuts[i].passed)
        {
          if (not m_enableSinglePhotonTrigger and m_inputCutList.at(i) == "PASS_TRIGGER"){
            if (not m_bools.at(HHBBYY::pass_trigger_diphoton)) break;
            consecutive_cuts += m_bools.at(HHBBYY::pass_trigger_diphoton);
          }else if (not m_enableSinglePhotonTrigger and m_inputCutList.at(i) == "PASS_TRIGGER_MATCHING"){
            if (not m_bools.at(HHBBYY::pass_matching_trigger_diphoton)) break;
            consecutive_cuts += m_bools.at(HHBBYY::pass_matching_trigger_diphoton);
          }else
            consecutive_cuts++;
        }
        else
          break;
      }

      // Here we basically increment the  N_events(pass_i  AND pass_i-1  AND ... AND pass_0) for the i-cut.
      // I think this is an elegant way to do it :) . Considering the difficulties a configurable cut list imposes. 
      for (unsigned int i=0; i<consecutive_cuts; i++) {
        std::string cut = m_inputCutList.at(i);
        bool pass = true;
        if (not m_enableSinglePhotonTrigger and cut == "PASS_TRIGGER")
          pass = m_bools.at(HHBBYY::pass_trigger_diphoton);
        else if (not m_enableSinglePhotonTrigger and cut == "PASS_TRIGGER_MATCHING")
          pass = m_bools.at(HHBBYY::pass_matching_trigger_diphoton);
        m_bbyyCuts[i].relativeCounter += pass;
        if (m_isMC) m_bbyyCuts(cut).w_relativeCounter += m_generatorWeight.get(*event, sys) * pass;
      }

    }

    return StatusCode::SUCCESS;
  }

  StatusCode bbyySelectorAlg::finalize()
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

  void bbyySelectorAlg::evaluateTriggerCuts(const xAOD::EventInfo& event, const std::vector<std::string> &photonTriggers, 
                                                  CutManager& bbyyCuts) {

    if (!bbyyCuts.exists("PASS_TRIGGER"))
        return;

    bool pass_trigger_single_photon = false;
    bool pass_trigger_diphoton = false;

    for (const std::string &trigger : photonTriggers)
    {
      
      SG::ReadDecorHandleKey<xAOD::EventInfo>& triggerDecorKey = m_triggerDecorKeys.at(trigger);
      SG::ReadDecorHandle<xAOD::EventInfo, bool> m_triggerDecorHandle(triggerDecorKey);

      if (m_triggerMap.at(trigger) == "single_photon")
      {
        if (!m_bools.at(HHBBYY::is15) && trigger == "HLT_g120_loose")
        {
          continue;
        }
        else
          pass_trigger_single_photon = m_triggerDecorHandle(event);
      }
      else
      {
        pass_trigger_diphoton = m_triggerDecorHandle(event);
      }
     
    }
    // OR between single and double photon triggers
    bbyyCuts("PASS_TRIGGER").passed = pass_trigger_diphoton || pass_trigger_single_photon;
    m_bools.at(HHBBYY::pass_trigger_diphoton) = pass_trigger_diphoton;
    m_bools.at(HHBBYY::pass_trigger_single_photon) = pass_trigger_single_photon;
  }


  void bbyySelectorAlg::evaluateTriggerMatchingCuts(const std::vector<std::string> &photonTriggers, 
                                                  const xAOD::PhotonContainer* photons, CutManager& bbyyCuts) {

    if (!bbyyCuts.exists("PASS_TRIGGER_MATCHING"))
        return;

    bool pass_matching_trigger_single_photon = false;
    bool pass_matching_trigger_diphoton = false;
    if (photons->size() >= 2){

      for (const std::string &trigger : photonTriggers)
      {
      

        if (m_triggerMap.at(trigger) == "single_photon")
        {
          if (!m_bools.at(HHBBYY::is15) && trigger == "HLT_g120_loose") {
            continue;
          }
          else
            pass_matching_trigger_single_photon = m_matchingTool->match(*photons->at(0), trigger) || m_matchingTool->match(*photons->at(1), trigger);
        }
        else
        {
          pass_matching_trigger_diphoton = m_matchingTool->match({photons->at(0), photons->at(1)}, trigger);
        }
     
      }
    
    }
    //OR between single and di-photon trigger for matching studies
    bbyyCuts("PASS_TRIGGER_MATCHING").passed = pass_matching_trigger_single_photon || pass_matching_trigger_diphoton;
    m_bools.at(HHBBYY::pass_matching_trigger_single_photon) = pass_matching_trigger_single_photon;
    m_bools.at(HHBBYY::pass_matching_trigger_diphoton) = pass_matching_trigger_diphoton;

  }

  void bbyySelectorAlg::evaluatePhotonCuts
  (const std::vector<const xAOD::Photon*>& photons, int n_TightID_NonIso_photons, int n_Loose_Iso_photons, int n_TightID_Iso_photons, CutManager& bbyyCuts)
  {
    // photon isolation and selection pT/myy
   
    if (bbyyCuts.exists("TWO_TIGHTID_PHOTONS"))
    bbyyCuts("TWO_TIGHTID_PHOTONS").passed = (n_TightID_NonIso_photons >= 2);

    if (bbyyCuts.exists("TWO_ISO_PHOTONS"))
    bbyyCuts("TWO_ISO_PHOTONS").passed = (n_Loose_Iso_photons >= 2);

    if (bbyyCuts.exists("TWO_TIGHTID_ISO_PHOTONS"))
    bbyyCuts("TWO_TIGHTID_ISO_PHOTONS").passed = (n_TightID_Iso_photons >= 2);

    double myy = (photons.at(0)->p4() + photons.at(1)->p4()).M();
    std::vector<float> ptOverMasses;

    for (const xAOD::Photon* photon : {photons.at(0), photons.at(1)})
    {
      ptOverMasses.push_back(photon->pt() / myy);
    }

    if (ptOverMasses[0] > 0.35 && ptOverMasses[1] > 0.25 && bbyyCuts.exists("PASS_RELPT"))
      bbyyCuts("PASS_RELPT").passed = true;
    if (myy >= 105. * Athena::Units::GeV && myy < 160. * Athena::Units::GeV && bbyyCuts.exists("DIPHOTON_MASS"))
      bbyyCuts("DIPHOTON_MASS").passed = true;
  }


  void bbyySelectorAlg::evaluateLeptonCuts
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

  void bbyySelectorAlg::evaluateJetCuts(const ConstDataVector<xAOD::JetContainer>& bjets,
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
    if ((bjets.size()==1 || bjets.size()==2) && bbyyCuts.exists("EXACTLY_ONE_OR_TWO_B_JETS")) 
      bbyyCuts("EXACTLY_ONE_OR_TWO_B_JETS").passed = true;
  }
  void bbyySelectorAlg::setRunNumberQuantities(unsigned int rdmNumber){
    // References:
    // https://atlas-tagservices.cern.ch/tagservices/RunBrowser/runBrowserReport/rBR_Period_Report.php
    // https://twiki.cern.ch/twiki/bin/view/Atlas/LowestUnprescaled

    m_bools.at(HHBBYY::is15) = 266904 <= rdmNumber && rdmNumber <= 284484;
    m_bools.at(HHBBYY::is16) = 296939 <= rdmNumber && rdmNumber <= 311481;
  }

}
