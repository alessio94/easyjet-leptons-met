/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ZQGTruthSelectorAlg.h"
#include <AthenaKernel/Units.h>
#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AthContainers/ConstDataVector.h>


namespace ZQG
{
  
  ZQGTruthSelectorAlg::ZQGTruthSelectorAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : EL::AnaAlgorithm(name, pSvcLocator)
  {
  }


  StatusCode ZQGTruthSelectorAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("     ZQGTruthSelectorAlg      \n");
    ATH_MSG_INFO("*********************************\n");


    ATH_CHECK (m_truthjetsInKey.initialize());
    ATH_CHECK (m_truthlargejetInKey.initialize());
    ATH_CHECK (m_truthelectronInKey.initialize());
    ATH_CHECK (m_truthmuonInKey.initialize());
    ATH_CHECK (m_eventInKey.initialize());   
    ATH_CHECK (m_passTruthCutsKey.initialize());
    ATH_CHECK (m_jetTruthFlavourKey.initialize());

    for (auto& [boolKey, boolName] : m_boolnames) {   
      m_bools.emplace(boolKey, false);
      std::string fullKey = m_eventInKey.key() + "." + boolName;
      SG::WriteDecorHandleKey<xAOD::TruthEventContainer> handleKey(fullKey);
      ATH_CHECK(handleKey.initialize());
      m_BbranchKeys.emplace(boolKey, std::move(handleKey));
      ATH_CHECK(m_BbranchKeys.at(boolKey).initialize());
    }   
    

    for (const std::string &var : m_floatVariables) {
      std::string varkey = m_eventInKey.key() + "." + var;
      SG::WriteDecorHandleKey<xAOD::TruthEventContainer> floatVarKey(varkey);
      m_FbranchesKeys.emplace(var, std::move(floatVarKey));
      ATH_CHECK (m_FbranchesKeys.at(var).initialize());
    }
    
    if(m_saveCutFlow) ATH_CHECK (initialiseCutflow());

    return StatusCode::SUCCESS;

  }


  StatusCode ZQGTruthSelectorAlg::execute()
  {

    // FilterReporter filter(m_filterParams, false);

    SG::ReadHandle<xAOD::TruthEventContainer> truthEvents(m_eventInKey);
    SG::ReadHandle<xAOD::JetContainer> truthJets(m_truthjetsInKey);
    SG::ReadHandle<xAOD::JetContainer> truthLargeJets(m_truthlargejetInKey);
    SG::ReadHandle<xAOD::TruthParticleContainer> truthElectrons(m_truthelectronInKey);
    SG::ReadHandle<xAOD::TruthParticleContainer> truthMuons(m_truthmuonInKey);
    SG::ReadDecorHandle<xAOD::JetContainer, int> truthFlavour(m_jetTruthFlavourKey);

    auto jets_after_overlap = std::make_unique<ConstDataVector<xAOD::JetContainer>>(SG::VIEW_ELEMENTS);
    auto truthbjets = std::make_unique<ConstDataVector<xAOD::JetContainer>>(SG::VIEW_ELEMENTS);
    auto truthcjets = std::make_unique<ConstDataVector<xAOD::JetContainer>>(SG::VIEW_ELEMENTS);

    for (auto [var, key] : m_FbranchesKeys){
      SG::WriteDecorHandle<xAOD::TruthEventContainer, float> floatDecorator(key);
      m_Fbranches.insert_or_assign(var, std::move(floatDecorator));
    }

    const xAOD::TruthEvent* truthevent = truthEvents->at(0);

    for (const std::string &string_var: m_floatVariables) {
        m_Fbranches.at(string_var)(*truthevent) = -99.;
    }

    truthOverlapRemoval(*truthJets, *truthElectrons, *truthMuons, *jets_after_overlap);
    for (const xAOD::Jet *jet : *jets_after_overlap){
      bool isbjet=false;
      bool iscjet=false;
      int truthlabel = truthFlavour(*jet);
      if (truthlabel == 5){
        isbjet = true;
      } else if (truthlabel == 4){
        iscjet = true;
      }
      if(isbjet) truthbjets->push_back(jet);
      if(iscjet) truthcjets->push_back(jet);
    }

    int n_cjets = truthcjets->size();
    for (int i=0; i<std::min(n_cjets, 2); ++i){
      std::string prefix = "TruthJet_c"+std::to_string(i+1);
      m_Fbranches.at(prefix+"_pt")(*truthevent) = truthcjets->at(i)->pt();
      m_Fbranches.at(prefix+"_eta")(*truthevent) = truthcjets->at(i)->eta();
      m_Fbranches.at(prefix+"_phi")(*truthevent) = truthcjets->at(i)->phi();
      m_Fbranches.at(prefix+"_E")(*truthevent) = truthcjets->at(i)->e();
    }

    // Leptons
    const xAOD::TruthParticle* ele0 = nullptr;
    const xAOD::TruthParticle* ele1 = nullptr;

    for (const xAOD::TruthParticle* electron : *truthElectrons){
      if (!ele0) ele0 = electron;
      else {
        ele1 = electron;
        break;
      }
    }

    const xAOD::TruthParticle* mu0 = nullptr;
    const xAOD::TruthParticle* mu1 = nullptr;

    for (const xAOD::TruthParticle* muon : *truthMuons){
      if (!mu0) mu0 = muon;
      else {
        mu1 = muon;
        break;
      }
    }

    std::vector<const xAOD::TruthParticle*> leptons;
    if(ele0) leptons.emplace_back(ele0);
    if(mu0) leptons.emplace_back(mu0);
    if(ele1) leptons.emplace_back(ele1);
    if(mu1) leptons.emplace_back(mu1);

    std::sort(leptons.begin(), leptons.end(),
    [](const xAOD::TruthParticle*& a,
        const xAOD::TruthParticle*& b) {
      return a->pt() > b->pt(); });


    m_bools.at(ZQG::IS_ee_TRUTH) = false;
    m_bools.at(ZQG::IS_mm_TRUTH) = false;
    m_bools.at(ZQG::IS_em_TRUTH) = false;

    m_bools.at(ZQG::EXACTLY_TWO_LEPTONS_TRUTH) = false;
    m_bools.at(ZQG::OPPOSITE_CHARGE_LEPTONS_TRUTH) = false;
    m_bools.at(ZQG::DILEPTON_MASS_WINDOW_TRUTH) = false;

    m_bools.at(ZQG::ONE_B_JETS_TRUTH) = false;
    m_bools.at(ZQG::TWO_B_JETS_TRUTH) = false;
    m_bools.at(ZQG::ONE_C_JETS_TRUTH) = false;
    m_bools.at(ZQG::TWO_C_JETS_TRUTH) = false;
    m_bools.at(ZQG::ONE_LARGE_JET_TRUTH) = false;

    evaluateTruthLeptonCuts(*truthevent, *truthElectrons, *truthMuons, m_ZQGTruthCuts);
    evaluateTruthBJetCuts(*truthbjets, m_ZQGTruthCuts);
    evaluateTruthCJetCuts(*truthcjets, m_ZQGTruthCuts);
    evaluateTruthLargeJetCuts(*truthLargeJets);
    
    bool pass_truth_baseline=true;
    for (const auto& [key, value] : m_boolnames) {
      auto it = std::find(m_BASELINE_CUTS.begin(), m_BASELINE_CUTS.end(), value);
      if (it != m_BASELINE_CUTS.end()) {
        pass_truth_baseline &= m_bools.at(key);
      }
    }
  
    // Compute total_events
    m_total_events+=1;

    for (const auto &cut : m_inputCutKeys) {
      if (m_ZQGTruthCuts.exists(m_boolnames.at(cut))) {
        m_ZQGTruthCuts(m_boolnames.at(cut)).passed = m_bools.at(cut);
        if (m_ZQGTruthCuts(m_boolnames.at(cut)).passed) {
          m_ZQGTruthCuts(m_boolnames.at(cut)).counter += 1;
        }
      }
    }
    
    unsigned int consecutive_cuts = 0;
    for (size_t i = 0; i < m_ZQGTruthCuts.size(); ++i) {
      if (m_ZQGTruthCuts[i].passed)
        consecutive_cuts++;
      else
        break;
    }

    // Here we basically increment the  N_events(pass_i  AND pass_i-1  AND ... AND pass_0) for the i-cut.
    // I think this is an elegant way to do it :) . Considering the difficulties a configurable cut list imposes.
    for (unsigned int i=0; i<consecutive_cuts; i++) {
      m_ZQGTruthCuts[i].relativeCounter+=1;
    }

    ATH_MSG_VERBOSE("pass_truth_baseline = " << pass_truth_baseline);
    SG::WriteDecorHandle<xAOD::TruthEventContainer, bool> passTruthCutsHandle(m_passTruthCutsKey);
    passTruthCutsHandle(*truthevent) = pass_truth_baseline;

    for (auto& [key, handleKey] : m_BbranchKeys) {
      SG::WriteDecorHandle<xAOD::TruthEventContainer, bool> handle(handleKey);
      handle(*truthevent) = m_bools.at(key);
    }

  return StatusCode::SUCCESS;
  }

  StatusCode ZQGTruthSelectorAlg::finalize()
  {
    ATH_MSG_INFO("Total events = " << m_total_events <<std::endl);
    m_ZQGTruthCuts.CheckCutResults(); // Print CheckCutResults

    if(m_saveCutFlow) {
      m_ZQGTruthCuts.DoAbsoluteEfficiency(m_total_events, efficiency("AbsoluteTruthEfficiency"));
      m_ZQGTruthCuts.DoRelativeEfficiency(m_total_events, efficiency("RelativeTruthEfficiency"));
      m_ZQGTruthCuts.DoStandardCutFlow(m_total_events, efficiency("StandardTruthCutFlow"));
      m_ZQGTruthCuts.DoCutflowLabeling(m_total_events, hist("TruthEventsPassed_BinLabeling"));

    }

    return StatusCode::SUCCESS;
  }

  void ZQGTruthSelectorAlg::truthOverlapRemoval(const xAOD::JetContainer& truthJets, const xAOD::TruthParticleContainer& truthElectrons, const xAOD::TruthParticleContainer& truthMuons, ConstDataVector<xAOD::JetContainer>& jetsAfterOverlap){

    float dRcut = 0.4;
    for (const xAOD::Jet* jet : truthJets){
      TLorentzVector jet_p4 = jet->p4();
      bool keep = true;
      for (const xAOD::TruthParticle* ele : truthElectrons){
        TLorentzVector ele_p4 = ele->p4();
        if (jet_p4.DeltaR(ele_p4) < dRcut){
          keep = false;
          break;
        }
      }
      if (!keep) continue;
      for (const xAOD::TruthParticle* mu : truthMuons){
        TLorentzVector mu_p4 = mu->p4();
        if (jet_p4.DeltaR(mu_p4) < dRcut){
          keep = false;
          break;
        }
      }
      if (keep){
        jetsAfterOverlap.push_back(jet);
      }
    }

  }

  void ZQGTruthSelectorAlg::evaluateTruthLeptonCuts(const xAOD::TruthEvent& truthevent, const xAOD::TruthParticleContainer& truthelectrons, const xAOD::TruthParticleContainer& truthmuons, CutManager& ZQGTruthCuts)
  {
  
    TLorentzVector ll;
    double mll = -99;

    bool OPPOSITE_CHARGE_LEPTONS_TRUTH = false;

    if (truthelectrons.size() + truthmuons.size() == 2) {
    
      m_bools.at(ZQG::EXACTLY_TWO_LEPTONS_TRUTH) = true;

    }
      
    if (truthelectrons.size() >= 2 && truthmuons.size() == 0)
    {
      m_bools.at(ZQG::IS_ee_TRUTH) = true;
      ll = truthelectrons.at(0)->p4() + truthelectrons.at(1)->p4();
      OPPOSITE_CHARGE_LEPTONS_TRUTH = truthelectrons.at(0)->charge()*truthelectrons.at(1)->charge() == -1;
    }

    if (truthmuons.size() >= 2 && truthelectrons.size() == 0)
    {
      m_bools.at(ZQG::IS_mm_TRUTH) = true;
      ll = truthmuons.at(0)->p4() + truthmuons.at(1)->p4();
      OPPOSITE_CHARGE_LEPTONS_TRUTH = truthmuons.at(0)->charge()*truthmuons.at(1)->charge() == -1;
    }

    if (truthelectrons.size() == 1 && truthmuons.size() == 1)
    {
      m_bools.at(ZQG::IS_em_TRUTH) = true;
      ll = truthelectrons.at(0)->p4() + truthmuons.at(0)->p4();
      OPPOSITE_CHARGE_LEPTONS_TRUTH = truthelectrons.at(0)->charge()*truthmuons.at(0)->charge() == -1;
    }
    

    mll = ll.M();


    std::string prefix = "Truth_ll";
    m_Fbranches.at(prefix+"_pt")(truthevent) = ll.Pt();
    m_Fbranches.at(prefix+"_eta")(truthevent) = ll.Eta();
    m_Fbranches.at(prefix+"_phi")(truthevent) = ll.Phi();
    m_Fbranches.at(prefix+"_E")(truthevent) = ll.E();
    m_Fbranches.at(prefix+"_m")(truthevent) = mll;


    if(ZQGTruthCuts.exists("OPPOSITE_CHARGE_LEPTONS_TRUTH")) m_bools.at(ZQG::OPPOSITE_CHARGE_LEPTONS_TRUTH) = OPPOSITE_CHARGE_LEPTONS_TRUTH;
    if(ZQGTruthCuts.exists("DILEPTON_MASS_WINDOW_TRUTH")) m_bools.at(ZQG::DILEPTON_MASS_WINDOW_TRUTH) = ( mll >= 76.*Athena::Units::GeV && mll <= 106.*Athena::Units::GeV );

  }

  void ZQGTruthSelectorAlg::evaluateTruthBJetCuts
  (const ConstDataVector<xAOD::JetContainer>& truthbjets, CutManager& ZQGTruthCuts)
  { 

    ///All jets in the containers should have pT>20GeV. Check minPt of your JetSelectorAlg in the ZQG_config file.
    if (truthbjets.size() >= 1 && ZQGTruthCuts.exists("ONE_B_JETS_TRUTH")){
      m_bools.at(ZQG::ONE_B_JETS_TRUTH) = true;
    }

    if (truthbjets.size() >= 2 && ZQGTruthCuts.exists("TWO_B_JETS_TRUTH")){
      m_bools.at(ZQG::TWO_B_JETS_TRUTH) = true;
    }

  }  
  
  void ZQGTruthSelectorAlg::evaluateTruthCJetCuts
  (const ConstDataVector<xAOD::JetContainer>& truthcjets, CutManager& ZQGTruthCuts)
  { 

    ///All jets in the containers should have pT>20GeV. Check minPt of your JetSelectorAlg in the ZQG_config file.
    if (truthcjets.size() >= 1 && ZQGTruthCuts.exists("ONE_C_JETS_TRUTH")){
      m_bools.at(ZQG::ONE_C_JETS_TRUTH) = true;
    }

    if (truthcjets.size() >= 2 && ZQGTruthCuts.exists("TWO_C_JETS_TRUTH")){
      m_bools.at(ZQG::TWO_C_JETS_TRUTH) = true;
    }

  }  

  void ZQGTruthSelectorAlg::evaluateTruthLargeJetCuts
  (const xAOD::JetContainer& truthlargeJets)
  {
    m_bools.at(ZQG::ONE_LARGE_JET_TRUTH) = (truthlargeJets.size() >= 1);
  }


  StatusCode ZQGTruthSelectorAlg::initialiseCutflow(){


    std::vector<std::string> boolnameslist;
    for (const auto& [key, value] : m_boolnames) {
      boolnameslist.push_back(value);
    }
    m_ZQGTruthCuts.CheckInputCutList(m_inputCutList, boolnameslist);

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
      m_ZQGTruthCuts.add(m_boolnames[cut]);
    }

    //After filling the CutManager, book your histograms.
    const unsigned int nbins = m_ZQGTruthCuts.size() + 1; //  need an extra bin for the total num of events.
    ATH_MSG_DEBUG("Booking AbsoluteTruthEfficiency histogram...");
    ANA_CHECK (book (TEfficiency("AbsoluteTruthEfficiency","Absolute Efficiency of ZQG truth cuts;Cuts;#epsilon",
				 nbins, 0.5, nbins + 0.5)));
    ATH_MSG_DEBUG("Booking RelativeTruthEfficiency histogram...");
    ANA_CHECK (book (TEfficiency("RelativeTruthEfficiency","Relative Efficiency of ZQG truth cuts;Cuts;#epsilon",
				 nbins, 0.5, nbins + 0.5)));
    ATH_MSG_DEBUG("Booking StandardTruthCutFlow histogram...");
    ANA_CHECK (book (TEfficiency("StandardTruthCutFlow","StandardCutFlow of ZQG truth cuts;Cuts;#epsilon",
				 nbins, 0.5, nbins + 0.5)));
    ATH_MSG_DEBUG("Booking TruthEventsPassed_BinLabeling histogram...");
    ANA_CHECK (book (TH1F("TruthEventsPassed_BinLabeling", "Events passed by each truth cut / Bin labeling", nbins, 0.5, nbins + 0.5)));
    ATH_MSG_DEBUG("Histograms booked successfully");
    return StatusCode::SUCCESS;
  }

}

