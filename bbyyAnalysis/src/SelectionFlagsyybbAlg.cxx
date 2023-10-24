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
    ATH_CHECK(m_smallRJets_BTag_ContainerInKey.initialize());
    ATH_CHECK(m_smallRJets_ContainerInKey.initialize());
    ATH_CHECK(m_photonContainerInKey.initialize());
    ATH_CHECK(m_muonContainerInKey.initialize());
    ATH_CHECK(m_electronContainerInKey.initialize());
    ATH_CHECK(m_EventInfoKey.initialize());

    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("Your selected list of cuts :\n");
    for (const std::string &cut : m_inputCutList) {
      std::string deco_cut = cut; 
      SG::AuxElement::Decorator<float> deco(deco_cut);
      m_decos.emplace(deco_cut, deco);
      ATH_MSG_INFO(cut);
    };
    ATH_MSG_INFO("*********************************\n");

    SG::AuxElement::Decorator<float> decoPassAll("PassAllCuts");
    m_decos.emplace("PassAllCuts", decoPassAll);

    yybbCuts.CheckInputCutList(m_inputCutList,m_STANDARD_CUTS);

    for (const std::string &cut : m_inputCutList)  { 
      // Initialize a vector of CutEntry structs based on the input Cut List
      yybbCuts.add(cut);
    }

    //After filling the CutManager, book your histograms.
    const unsigned int nbins = yybbCuts.size() + 1; //  need an extra bin for the total num of events.
    ANA_CHECK (book (TH1F ("AbsoluteEfficiency", "Absolute Efficiency of HH->yybb cuts", nbins, 0.5, nbins + 0.5))); 
    ANA_CHECK (book (TH1F ("RelativeEfficiency", "Relative Efficiency of HH->yybb cuts", nbins, 0.5, nbins + 0.5))); 
    ANA_CHECK (book (TH1F ("StandardCutFlow",       "Standard Cutflow of HH->yybb cuts", nbins, 0.5, nbins + 0.5))); 

    return StatusCode::SUCCESS;
  }


  StatusCode SelectionFlagsyybbAlg::execute()
  {
    //Containers that we read in
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    SG::ReadHandle<ConstDataVector<xAOD::JetContainer> > smallRJets_btag(
        m_smallRJets_BTag_ContainerInKey);
    SG::ReadHandle<ConstDataVector<xAOD::JetContainer> > smallRJets(
        m_smallRJets_ContainerInKey);
    SG::ReadHandle<ConstDataVector<xAOD::PhotonContainer> > photons(
        m_photonContainerInKey);
    SG::ReadHandle<ConstDataVector<xAOD::MuonContainer> > muons(
        m_muonContainerInKey);
    SG::ReadHandle<ConstDataVector<xAOD::ElectronContainer> > electrons(
        m_electronContainerInKey);

    ATH_CHECK(eventInfo.isValid());
    ATH_CHECK(smallRJets_btag.isValid());
    ATH_CHECK(smallRJets.isValid());
    ATH_CHECK(photons.isValid());
    ATH_CHECK(muons.isValid());
    ATH_CHECK(electrons.isValid());

    m_total_events+=1; //Calculate total_events.
    // Refresh CutEntry objects concerning the "passed or not" information per event.
    for (CutEntry& cut : yybbCuts) cut.passed = false;

    for (const std::string &cut : m_inputCutList) {
      std::string deco_cut = cut; 
      m_decos.at(deco_cut)(*eventInfo) = -99.; 
    };
    m_decos.at("PassAllCuts")(*eventInfo) = -99.; 
  
    if (!m_photonTriggers.empty()) {
      evaluateTriggerCuts(*eventInfo,m_photonTriggers,yybbCuts);
    }

    evaluatePhotonCuts(*photons, yybbCuts);
    evaluateLeptonCuts(*electrons, *muons, yybbCuts);
    evaluateJetCuts(*smallRJets_btag, *smallRJets, yybbCuts);

    //Assign values only to decorated cuts (cuts of input cutlist) but they should also
    //exist in the CutManager. For instance, a trigger cut could be erased from CutManager
    //because the specific input ntuple doesn't include the requested trigger.
    for (const auto &cut : m_inputCutList) {
      if(yybbCuts.exists(cut)) {
        m_decos.at(cut)(*eventInfo) = yybbCuts(cut).passed;
        if (yybbCuts(cut).passed)
          yybbCuts(cut).counter+=1;
      }
    }

    // Check how many consecutive cuts are passed by the event.
    unsigned int consecutive_cuts = 0;
    for (size_t i = 0; i < yybbCuts.size(); ++i) {
      if (yybbCuts[i].passed)
        consecutive_cuts++;
      else
        break;
    }

    // Here we basically increment the  N_events(pass_i  AND pass_i-1  AND ... AND pass_0) for the i-cut.
    // I think this is an elegant way to do it :) . Considering the difficulties a configurable cut list imposes. 
    for (unsigned int i=0; i<consecutive_cuts; i++) {
      yybbCuts[i].relativeCounter+=1;
    }

    if (consecutive_cuts==yybbCuts.size()) {
      m_decos.at("PassAllCuts")(*eventInfo) = 1;
      yybbCuts.PassAllCuts +=1;
    }
    else{
      m_decos.at("PassAllCuts")(*eventInfo) = 0;
    }


    return StatusCode::SUCCESS;

  }


  StatusCode SelectionFlagsyybbAlg::finalize()
  {

    ATH_MSG_INFO("Total events = " << m_total_events <<std::endl);
    yybbCuts.CheckCutResults(); // Print CheckCutResults

    if(m_saveCutFlow) {
      yybbCuts.DoAbsoluteEfficiency(m_total_events, hist("AbsoluteEfficiency"));
      yybbCuts.DoRelativeEfficiency(m_total_events, hist("RelativeEfficiency"));
      yybbCuts.DoStandardCutFlow(m_total_events, hist("StandardCutFlow"));
    }
    else {
      delete hist("AbsoluteEfficiency");
      delete hist("RelativeEfficiency");
      delete hist("StandardCutFlow");
    }


     return StatusCode::SUCCESS;

  }

  void SelectionFlagsyybbAlg::evaluateTriggerCuts(const xAOD::EventInfo& eventInfo, const std::vector<std::string> &photonTriggers, 
                                                  CutManager& yybbCuts) {

    if (!yybbCuts.exists("PASS_TRIGGER"))
        return;

    for (const std::string &trigger : photonTriggers)
    {
      std::string trigAccessorName = "trigPassed_"+trigger;
      const SG::AuxElement::ConstAccessor<bool> TriggerAccessor(trigAccessorName);
      //If the event passes any of the available (single or di-) photon triggers, set the overall trigger cut to true.
      if (TriggerAccessor(eventInfo)) {
        yybbCuts("PASS_TRIGGER").passed = true;
        break;
      }
    }

  }

  void SelectionFlagsyybbAlg::evaluatePhotonCuts(const ConstDataVector<xAOD::PhotonContainer>& photons, CutManager& yybbCuts)
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
      myy = (photons[0]->p4() + photons[1]->p4()).M();

      for (const xAOD::Photon* photon : {photons[0], photons[1]})
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


  void SelectionFlagsyybbAlg::evaluateLeptonCuts(const ConstDataVector<xAOD::ElectronContainer>& electrons,
                                const ConstDataVector<xAOD::MuonContainer>& muons, CutManager& yybbCuts)
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
      /*Loose_VarRad WP based on
      https://twiki.cern.ch/twiki/bin/view/AtlasProtected/RecommendedIsolationWPsRel22#Muon_isolation_working_points*/
      PassMuonIso = muon->isolation(xAOD::Iso::topoetcone20)/muon->pt() < 0.30 &&  
                      muon->isolation(xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000)/muon->pt() < 0.15 ;
      PassMuonMedium = DFCommonMuonPassIDCuts(*muon) && DFCommonMuonPassPreselection(*muon);
      if (PassMuonIso && PassMuonMedium)
        n_leptons+=1;
    }

    // No medium+isolated electrons and muons.
    if (n_leptons==0)
      yybbCuts("EXACTLY_ZERO_LEPTONS").passed = true;

  }

  void SelectionFlagsyybbAlg::evaluateJetCuts(const ConstDataVector<xAOD::JetContainer>& smallRJets_btag,
                            const ConstDataVector<xAOD::JetContainer>& smallRJets, CutManager& yybbCuts)
  {
    int CentralJets=0;

    ///All jets in the containers should have pT>25GeV. Check minPt of your JetSelectorAlg in the yybb_config file.
    for (const xAOD::Jet *jet : smallRJets)
    {// Jets here can be every type of jet (No Working point selected)
      // check if jet is central
      if(std::abs(jet->eta())<2.5)
        CentralJets+=1;
    }

    if (CentralJets<6 && yybbCuts.exists("LESS_THAN_SIX_CENTRAL_JETS"))
      yybbCuts("LESS_THAN_SIX_CENTRAL_JETS").passed = true;

    // If Forward Jets + Central jets >=2 --> The event passes.
    if (smallRJets.size() >= 2 && yybbCuts.exists("AT_LEAST_TWO_JETS"))
      yybbCuts("AT_LEAST_TWO_JETS").passed = true;

    if (smallRJets_btag.size()>=1 && yybbCuts.exists("AT_LEAST_ONE_B_JET"))
      yybbCuts("AT_LEAST_ONE_B_JET").passed = true;
    if (smallRJets_btag.size()==1 && yybbCuts.exists("EXACTLY_ONE_B_JET"))
      yybbCuts("EXACTLY_ONE_B_JET").passed = true;
    if (smallRJets_btag.size()>=2 && yybbCuts.exists("AT_LEAST_TWO_B_JETS"))
      yybbCuts("AT_LEAST_TWO_B_JETS").passed = true;
    if (smallRJets_btag.size()==2 && yybbCuts.exists("EXACTLY_TWO_B_JETS")) 
      yybbCuts("EXACTLY_TWO_B_JETS").passed = true;

  }

}