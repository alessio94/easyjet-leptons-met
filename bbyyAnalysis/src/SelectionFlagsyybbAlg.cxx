/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "SelectionFlagsyybbAlg.h"

namespace HHBBYY
{

  SelectionFlagsyybbAlg::SelectionFlagsyybbAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("cutList", m_inputCutList);
    declareProperty("saveCutFlow",m_saveCutFlow);
    declareProperty("photonTriggers",m_photonTriggers);
  }


  StatusCode SelectionFlagsyybbAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("      SelectionFlagsyybbAlg      \n");
    ATH_MSG_INFO("*********************************\n");

    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    ATH_CHECK (m_photonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    for (const std::string &string_var: m_inputCutList) {
      CP::SysWriteDecorHandle<bool> var {string_var+"_%SYS%", this};
      m_Bbranches.emplace(string_var, var);
      ATH_CHECK (m_Bbranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }
  
    // special flag for all cuts
    ATH_CHECK (m_passallcuts.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize()); 

    m_yybbCuts.CheckInputCutList(m_inputCutList,m_STANDARD_CUTS);

    for (const std::string &cut : m_inputCutList)  { 
      // Initialize a vector of CutEntry structs based on the input Cut List
      m_yybbCuts.add(cut);
    }

    //After filling the CutManager, book your histograms.
    const unsigned int nbins = m_yybbCuts.size() + 1; //  need an extra bin for the total num of events.
    ANA_CHECK (book (TEfficiency("AbsoluteEfficiency","Absolute Efficiency of HH->yybb cuts;Cuts;#epsilon", 
                                  nbins, 0.5, nbins + 0.5))); 
    ANA_CHECK (book (TEfficiency("RelativeEfficiency","Relative Efficiency of HH->yybb cuts;Cuts;#epsilon", 
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TEfficiency("StandardCutFlow","StandardCutFlow of HH->yybb cuts;Cuts;#epsilon", 
                                  nbins, 0.5, nbins + 0.5)));
    ANA_CHECK (book (TH1F("EventsPassed_BinLabeling", "Events passed by each cut / Bin labeling", nbins, 0.5, nbins + 0.5)));    

    return StatusCode::SUCCESS;
  }


  StatusCode SelectionFlagsyybbAlg::execute()
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
      for (CutEntry& cut : m_yybbCuts) {
        cut.passed = false;
        m_Bbranches.at(cut.name).set(*event, cut.passed, sys);
      }

      if (!m_photonTriggers.empty()) {
        evaluateTriggerCuts(*event, m_photonTriggers, m_yybbCuts);
      }

      evaluatePhotonCuts(*photons, m_yybbCuts);
      evaluateLeptonCuts(*electrons, *muons, m_yybbCuts);
      evaluateJetCuts(*bjets, *jets, m_yybbCuts);

      bool passedall = true;
      for (CutEntry& cut : m_yybbCuts) {
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
        if(m_yybbCuts.exists(cut)) {
          if (m_yybbCuts(cut).passed)
            m_yybbCuts(cut).counter+=1;
        }
      }

      // Check how many consecutive cuts are passed by the event.
      unsigned int consecutive_cuts = 0;
      for (size_t i = 0; i < m_yybbCuts.size(); ++i) {
        if (m_yybbCuts[i].passed)
          consecutive_cuts++;
        else
          break;
      }

      // Here we basically increment the  N_events(pass_i  AND pass_i-1  AND ... AND pass_0) for the i-cut.
      // I think this is an elegant way to do it :) . Considering the difficulties a configurable cut list imposes. 
      for (unsigned int i=0; i<consecutive_cuts; i++) {
        m_yybbCuts[i].relativeCounter+=1;
      }

    }

    return StatusCode::SUCCESS;
  }

  StatusCode SelectionFlagsyybbAlg::finalize()
  {

    //adapt the following for each syst TODO
    ATH_MSG_INFO("Total events = " << m_total_events <<std::endl);
    m_yybbCuts.CheckCutResults(); // Print CheckCutResults

    if(m_saveCutFlow) {
      m_yybbCuts.DoAbsoluteEfficiency(m_total_events, efficiency("AbsoluteEfficiency"));
      m_yybbCuts.DoRelativeEfficiency(m_total_events, efficiency("RelativeEfficiency"));
      m_yybbCuts.DoStandardCutFlow(m_total_events, efficiency("StandardCutFlow"));
      m_yybbCuts.DoCutflowLabeling(m_total_events, hist("EventsPassed_BinLabeling"));
    }
    else {
      delete efficiency("AbsoluteEfficiency");
      delete efficiency("RelativeEfficiency");
      delete efficiency("StandardCutFlow");
      delete hist("EventsPassed_BinLabeling");
    }


     return StatusCode::SUCCESS;

  }

  void SelectionFlagsyybbAlg::evaluateTriggerCuts(const xAOD::EventInfo& event, const std::vector<std::string> &photonTriggers, 
                                                  CutManager& yybbCuts) {

    if (!yybbCuts.exists("PASS_TRIGGER"))
        return;

    for (const std::string &trigger : photonTriggers)
    {
      std::string trigAccessorName = "trigPassed_"+trigger;
      const SG::AuxElement::ConstAccessor<bool> TriggerAccessor(trigAccessorName);
      //If the event passes any of the available (single or di-) photon triggers, set the overall trigger cut to true.
      if (TriggerAccessor(event)) {
        yybbCuts("PASS_TRIGGER").passed = true;
        break;
      }
    }

  }

  void SelectionFlagsyybbAlg::evaluatePhotonCuts(const xAOD::PhotonContainer& photons, CutManager& yybbCuts)
  {

    static const SG::AuxElement::ConstAccessor<char>  DFCommonPhotonsIsEMTight ("DFCommonPhotonsIsEMTight");

    if (yybbCuts.exists("TWO_LOOSE_PHOTONS"))
      yybbCuts("TWO_LOOSE_PHOTONS").passed = (photons.size() == 2);

    double myy = -99;
    bool PassIso = 0;
    std::vector<float> PassTightIDs;
    std::vector<float> PassIsos;
    std::vector<float> ptOverMasses;

    // photon isolation and selection pT/myy
    if (photons.size() >= 2)
    {
      myy = (photons.at(0)->p4() + photons.at(1)->p4()).M();

      for (const xAOD::Photon* photon : {photons.at(0), photons.at(1)})
      {
        PassIso = (photon->isolation(xAOD::Iso::topoetcone20) / photon->pt()) < 0.065 &&
                (photon->isolation(xAOD::Iso::ptcone20) / photon->pt()) < 0.05;
        PassTightIDs.push_back(DFCommonPhotonsIsEMTight(*photon));
        PassIsos.push_back(PassIso);
        ptOverMasses.push_back(photon->pt() / myy);
      }

      if (PassTightIDs[0] == 1 && PassTightIDs[1] == 1 && yybbCuts.exists("TWO_TIGHTID_PHOTONS"))
        yybbCuts("TWO_TIGHTID_PHOTONS").passed = true;
      if (PassIsos[0] == 1 && PassIsos[1] == 1 && yybbCuts.exists("TWO_ISO_PHOTONS"))
        yybbCuts("TWO_ISO_PHOTONS").passed = true;
      if (ptOverMasses[0] > 0.35 && ptOverMasses[1] > 0.25 && yybbCuts.exists("PASS_RELPT"))
        yybbCuts("PASS_RELPT").passed = true;
      if (myy >= 105000. && myy < 160000. && yybbCuts.exists("DIPHOTON_MASS"))
        yybbCuts("DIPHOTON_MASS").passed = true;
    }
  }


  void SelectionFlagsyybbAlg::evaluateLeptonCuts(const xAOD::ElectronContainer& electrons,
                                const xAOD::MuonContainer& muons, CutManager& yybbCuts)
  {

    if (!yybbCuts.exists("EXACTLY_ZERO_LEPTONS"))
      return;

    int n_leptons=0;

    static const SG::AuxElement::ConstAccessor<char>  DFCommonElectronsLHMedium ("DFCommonElectronsLHMedium");
    static const SG::AuxElement::ConstAccessor<char>  DFCommonMuonPassIDCuts ("DFCommonMuonPassIDCuts");
    static const SG::AuxElement::ConstAccessor<char>  DFCommonMuonPassPreselection ("DFCommonMuonPassPreselection");

    //electron isolation
    for (const xAOD::Electron *electron : electrons)
    {
      bool PassElectronIso = 0;
      bool PassElectronMedium = 0;
      /*Loose_VarRad WP based on 
        https://twiki.cern.ch/twiki/bin/view/AtlasProtected/RecommendedIsolationWPsRel22#Electron_isolation_working_point*/
      PassElectronIso = electron->isolation(xAOD::Iso::topoetcone20)/electron->pt() < 0.20 &&  
                  electron->isolation(xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000)/electron->pt() < 0.15;
      PassElectronMedium = DFCommonElectronsLHMedium(*electron);
      if (PassElectronIso && PassElectronMedium)
        n_leptons+=1;
    }

    //muon isolation
    for (const xAOD::Muon *muon : muons)
    {
      bool PassMuonIso = 0;
      bool PassMuonMedium = 0;
      /*PflowLoose_VarRad WP based on
      https://twiki.cern.ch/twiki/bin/view/AtlasProtected/RecommendedIsolationWPsRel22#Muon_isolation_working_points*/
      PassMuonIso = 	( muon->isolation(xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500) 
                      + 0.4*muon->isolation(xAOD::Iso::neflowisol20) ) / muon->pt() < 0.16;
      PassMuonMedium = DFCommonMuonPassIDCuts(*muon) && DFCommonMuonPassPreselection(*muon);
      if (PassMuonIso && PassMuonMedium)
        n_leptons+=1;
    }

    // No medium+isolated electrons and muons.
    if (n_leptons==0)
      yybbCuts("EXACTLY_ZERO_LEPTONS").passed = true;

  }

  void SelectionFlagsyybbAlg::evaluateJetCuts(const ConstDataVector<xAOD::JetContainer>& bjets,
                            const xAOD::JetContainer& jets, CutManager& yybbCuts)
  {
    int CentralJets=0;

    ///All jets in the containers should have pT>25GeV. Check minPt of your JetSelectorAlg in the yybb_config file.
    for (const xAOD::Jet *jet : jets)
    {// Jets here can be every type of jet (No Working point selected)
      // check if jet is central
      if(std::abs(jet->eta())<2.5)
        CentralJets+=1;
    }

    if (CentralJets<6 && yybbCuts.exists("LESS_THAN_SIX_CENTRAL_JETS"))
      yybbCuts("LESS_THAN_SIX_CENTRAL_JETS").passed = true;

    // If Forward Jets + Central jets >=2 --> The event passes.
    if (jets.size() >= 2 && yybbCuts.exists("AT_LEAST_TWO_JETS"))
      yybbCuts("AT_LEAST_TWO_JETS").passed = true;

    if (bjets.size()>=1 && yybbCuts.exists("AT_LEAST_ONE_B_JET"))
      yybbCuts("AT_LEAST_ONE_B_JET").passed = true;
    if (bjets.size()==1 && yybbCuts.exists("EXACTLY_ONE_B_JET"))
      yybbCuts("EXACTLY_ONE_B_JET").passed = true;
    if (bjets.size()>=2 && yybbCuts.exists("AT_LEAST_TWO_B_JETS"))
      yybbCuts("AT_LEAST_TWO_B_JETS").passed = true;
    if (bjets.size()==2 && yybbCuts.exists("EXACTLY_TWO_B_JETS")) 
      yybbCuts("EXACTLY_TWO_B_JETS").passed = true;

  }

}