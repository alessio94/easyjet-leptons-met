/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "SelectionFlagsbbllAlg.h"
#include <AthenaKernel/Units.h>

namespace HHBBLL
{

  SelectionFlagsbbllAlg::SelectionFlagsbbllAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("cutList", m_inputCutList);
    declareProperty("saveCutFlow",m_saveCutFlow);
    declareProperty("passTriggers",m_passTriggers);
    declareProperty("categoryList", m_Bvarnames);
  }


  StatusCode SelectionFlagsbbllAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("      SelectionFlagsbbllAlg      \n");
    ATH_MSG_INFO("*********************************\n");

    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    //Initialize trigger decorations
    for (const std::string &trig : m_passTriggers)
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

    for (const std::string &string_var: m_Bvarnames) {
      CP::SysWriteDecorHandle<bool> var {string_var+"_%SYS%", this};
      m_Bbranches.emplace(string_var, var);
      ATH_CHECK (m_Bbranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }

    // special flag for all cuts
    ATH_CHECK (m_passallcuts.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    m_bbllCuts.CheckInputCutList(m_inputCutList,m_STANDARD_CUTS);

    for (const std::string &cut : m_inputCutList)  {
      // Initialize a vector of CutEntry structs based on the input Cut List
      m_bbllCuts.add(cut);
    }

    //After filling the CutManager, book your histograms.
    const unsigned int nbins = m_bbllCuts.size() + 1; //  need an extra bin for the total num of events.
    ANA_CHECK (book (TEfficiency("AbsoluteEfficiency","Absolute Efficiency of HH->bbll cuts;Cuts;#epsilon",
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("RelativeEfficiency","Relative Efficiency of HH->bbll cuts;Cuts;#epsilon",
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("StandardCutFlow","StandardCutFlow of HH->bbll cuts;Cuts;#epsilon",
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TH1F("EventsPassed_BinLabeling", "Events passed by each cut / Bin labeling", nbins, 0.5, nbins + 0.5)));

    return StatusCode::SUCCESS;
  }


  StatusCode SelectionFlagsbbllAlg::execute()
  {

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      // Retrive inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      auto nonbjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      for(const xAOD::Jet* jet : *jets) {
        if (WPgiven) {
          if (m_isBtag.get(*jet, sys)) bjets->push_back(jet);
	  else nonbjets->push_back(jet);
        }
      }

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      // reset all cut flags to default=false
      for (CutEntry& cut : m_bbllCuts) {
        cut.passed = false;
        m_Bbranches.at(cut.name).set(*event, cut.passed, sys);
      }

      if (!m_passTriggers.empty()) {
        evaluateTriggerCuts(*event, m_passTriggers, m_bbllCuts);
      }

      evaluateLeptonCuts(*electrons, *muons, m_bbllCuts);
      evaluateJetCuts(*bjets, *nonbjets, m_bbllCuts);
      evaluateBJetLeptonCuts(*event, *bjets, *electrons, *muons);

      bool passedall = true;
      for (CutEntry& cut : m_bbllCuts) {
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
        if(m_bbllCuts.exists(cut)) {
          if (m_bbllCuts(cut).passed)
            m_bbllCuts(cut).counter+=1;
        }
      }

      // Check how many consecutive cuts are passed by the event.
      unsigned int consecutive_cuts = 0;
      for (size_t i = 0; i < m_bbllCuts.size(); ++i) {
        if (m_bbllCuts[i].passed)
          consecutive_cuts++;
        else
          break;
      }

      // Here we basically increment the  N_events(pass_i  AND pass_i-1  AND ... AND pass_0) for the i-cut.
      // I think this is an elegant way to do it :) . Considering the difficulties a configurable cut list imposes.
      for (unsigned int i=0; i<consecutive_cuts; i++) {
        m_bbllCuts[i].relativeCounter+=1;
      }

    }

    return StatusCode::SUCCESS;
  }

  StatusCode SelectionFlagsbbllAlg::finalize()
  {

    //adapt the following for each syst TODO
    ATH_MSG_INFO("Total events = " << m_total_events <<std::endl);
    m_bbllCuts.CheckCutResults(); // Print CheckCutResults

    if(m_saveCutFlow) {
      m_bbllCuts.DoAbsoluteEfficiency(m_total_events, efficiency("AbsoluteEfficiency"));
      m_bbllCuts.DoRelativeEfficiency(m_total_events, efficiency("RelativeEfficiency"));
      m_bbllCuts.DoStandardCutFlow(m_total_events, efficiency("StandardCutFlow"));
      m_bbllCuts.DoCutflowLabeling(m_total_events, hist("EventsPassed_BinLabeling"));

    }
    else {
      delete efficiency("AbsoluteEfficiency");
      delete efficiency("RelativeEfficiency");
      delete efficiency("StandardCutFlow");
      delete hist("EventsPassed_BinLabeling");
    }


     return StatusCode::SUCCESS;

  }

  void SelectionFlagsbbllAlg::evaluateTriggerCuts(const xAOD::EventInfo& event, const std::vector<std::string> &passTriggers,
                                                  CutManager& bbllCuts) {

    if (!bbllCuts.exists("PASS_TRIGGER"))
        return;

    for (const std::string &trigger : passTriggers)
    {
      SG::ReadDecorHandleKey<xAOD::EventInfo>& triggerDecorKey = m_triggerDecorKeys.at(trigger);
      SG::ReadDecorHandle<xAOD::EventInfo, bool> m_triggerDecorHandle(triggerDecorKey);
      //If the event passes any of the available (single or di-) lepton triggers, set the overall trigger cut to true.
      if (m_triggerDecorHandle(event)) {
        bbllCuts("PASS_TRIGGER").passed = true;
        break;
      }
    }

  }

  void SelectionFlagsbbllAlg::evaluateLeptonCuts
  (const xAOD::ElectronContainer& electrons, const xAOD::MuonContainer& muons,
   CutManager& bbllCuts)
  {
    double mee = -99;
    double mmumu = -99;
    double memu = -99;
    bool Dilepton_Pass_DF = false;
    bool Two_Opposite_Sign_Electrons = false;
    bool Two_Opposite_Sign_Muons = false;
    bool Opposite_Sign_ElecMu = false;

    if ((electrons.size() + muons.size() == 2)
      && bbllCuts.exists("EXACTLY_TWO_LEPTONS"))
      bbllCuts("EXACTLY_TWO_LEPTONS").passed = true;

    if (electrons.size() >= 2)
    {
      mee = (electrons.at(0)->p4() + electrons.at(1)->p4()).M();
      Two_Opposite_Sign_Electrons = electrons.at(0)->charge()*electrons.at(1)->charge() == -1;

    }
    if (muons.size() >= 2)
    {
      mmumu = (muons.at(0)->p4() + muons.at(1)->p4()).M();
      Two_Opposite_Sign_Muons = muons.at(0)->charge()*muons.at(1)->charge() == -1;
    }
    if (electrons.size() == 1 && muons.size() == 1)
    {
      memu = (electrons.at(0)->p4() + muons.at(0)->p4()).M();
      Dilepton_Pass_DF = (memu > 15. * Athena::Units::GeV && memu < 110. * Athena::Units::GeV);
      Opposite_Sign_ElecMu = electrons.at(0)->charge()*muons.at(0)->charge() == -1;
    }
    bool Dilepton_Pass_SF_SR1 = ((electrons.size() >= 2  && mee > 15. * Athena::Units::GeV && mee < 75. * Athena::Units::GeV) ||
      (muons.size() >= 2 && mmumu > 15. * Athena::Units::GeV && mmumu < 75. * Athena::Units::GeV));
    bool Dilepton_Pass_SF_SR2 = ((electrons.size() >= 2  && mee > 75. * Athena::Units::GeV && mee < 110. * Athena::Units::GeV) ||
      (muons.size() >= 2 && mmumu > 75. * Athena::Units::GeV && mmumu < 110. * Athena::Units::GeV));

    if ((Two_Opposite_Sign_Electrons || Two_Opposite_Sign_Muons || Opposite_Sign_ElecMu)
      && bbllCuts.exists("TWO_OPPOSITE_CHARGE_LEPTONS"))
    {
      bbllCuts("TWO_OPPOSITE_CHARGE_LEPTONS").passed = true;
    }

    if(bbllCuts.exists("DILEPTON_MASS_SR1")) bbllCuts("DILEPTON_MASS_SR1").passed = (Dilepton_Pass_SF_SR1 || Dilepton_Pass_DF);
    if(bbllCuts.exists("DILEPTON_MASS_SR2")) bbllCuts("DILEPTON_MASS_SR2").passed = Dilepton_Pass_SF_SR2;

  }

  void SelectionFlagsbbllAlg::evaluateJetCuts(const ConstDataVector<xAOD::JetContainer>& bjets,
                            const ConstDataVector<xAOD::JetContainer>& nonbjets, CutManager& bbllCuts)
  {

    ///All jets in the containers should have pT>20GeV. Check minPt of your JetSelectorAlg in the bbll_config file.

    bool VBFVeto = false;
    double mbb = -99;
    float max_mjj = 0;
    float max_delta_eta_jj = 0;

    if (nonbjets.size() >= 2){
      bool jetsFound = false;

      for(unsigned int i=0;i<nonbjets.size()-1;i++){
        for(unsigned int j=i+1;j<nonbjets.size();j++){
	  const xAOD::Jet* nonbjet1 = nonbjets.at(i);
          const xAOD::Jet* nonbjet2 = nonbjets.at(j);

	  if (nonbjet1->pt() >= 30. * Athena::Units::GeV
            && nonbjet2->pt() >= 30. * Athena::Units::GeV) {
	    jetsFound = true;

            float mjj = (nonbjet1->p4() + nonbjet2->p4()).M();
	    float delta_eta_jj = nonbjet1->eta() - nonbjet2->eta();

	    if (mjj > max_mjj) max_mjj = mjj;
	    if (delta_eta_jj > max_delta_eta_jj)  max_delta_eta_jj = delta_eta_jj;
	  }
        }
      }

      if (jetsFound) {
        VBFVeto = !(max_delta_eta_jj > 4 && max_mjj > 600. * Athena::Units::GeV);
      }
    }
    if(bbllCuts.exists("VBFVETO_SR1")) bbllCuts("VBFVETO_SR1").passed = VBFVeto;

    if (bjets.size()==2 && bbllCuts.exists("EXACTLY_TWO_B_JETS"))
      bbllCuts("EXACTLY_TWO_B_JETS").passed = true;

    if (bjets.size() >= 2){
      mbb = (bjets.at(0)->p4() + bjets.at(1)->p4()).M();
    }

    if(bbllCuts.exists("DIBJET_MASS_SR2")) bbllCuts("DIBJET_MASS_SR2").passed = (mbb > 40. * Athena::Units::GeV && mbb < 210. * Athena::Units::GeV);

  }

  void SelectionFlagsbbllAlg::evaluateBJetLeptonCuts
  (const xAOD::EventInfo& event,
   const ConstDataVector<xAOD::JetContainer>& bjets,
   const xAOD::ElectronContainer& electrons, const xAOD::MuonContainer& muons)
  {
    bool TWO_ISO_ELECTRONS = (electrons.size() >= 2);
    bool TWO_ISO_MUONS = (muons.size() >= 2);
    bool TWO_ISO_ELECMUs = (electrons.size() == 1 && muons.size() == 1);
    bool EXACTLY_TWO_B_JETS = bjets.size()==2;

    bool IS_ee = electrons.size() == 2 && muons.size() == 0;
    bool IS_mm = electrons.size() == 0 && muons.size() == 2;
    bool IS_em = electrons.size() == 1 && muons.size() == 1;

    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      m_Bbranches.at("IS_ee").set(event, IS_ee, sys);
      m_Bbranches.at("IS_mm").set(event, IS_mm, sys);
      m_Bbranches.at("IS_em").set(event, IS_em, sys);
      m_Bbranches.at("IS_SF").set(event, (IS_ee || IS_mm), sys);
      m_Bbranches.at("Pass_ll").set(event, ((TWO_ISO_ELECTRONS || TWO_ISO_MUONS || TWO_ISO_ELECMUs) && EXACTLY_TWO_B_JETS), sys);
    }

  }

}

