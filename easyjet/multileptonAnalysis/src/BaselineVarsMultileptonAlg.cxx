/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AthContainers/AuxElement.h"
#include "BaselineVarsMultileptonAlg.h"
#include <AthContainers/ConstDataVector.h>

#include "EasyjetHub/MT2_ROOT.h"
#include "Math/VectorUtil.h"
#include "SubChannelClassify.h"
#include "TLorentzVector.h"

namespace MULTILEPTON
{
  BaselineVarsMultileptonAlg::BaselineVarsMultileptonAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {

  }

  StatusCode BaselineVarsMultileptonAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("   BaselineVarsMultileptonAlg    \n");
    ATH_MSG_INFO("*********************************\n");

    // Read syst-aware input handles
    ATH_CHECK(m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK(m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK(m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK(m_tauHandle.initialize(m_systematicsList));
    ATH_CHECK(m_metHandle.initialize(m_systematicsList));
    ATH_CHECK(m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK(m_kfJetHandle.initialize(m_systematicsList));
    ATH_CHECK(m_KF_MBB.initialize(m_systematicsList, m_eventHandle));

    // Initialise syst-aware output decorators
    for(const std::string &var : m_floatVariables){
      CP::SysWriteDecorHandle<float> whandle{var+"_%SYS%", this};
      m_Fbranches.emplace(var, whandle);
      ATH_CHECK(m_Fbranches.at(var).initialize(m_systematicsList, m_eventHandle));
    }
    
    for(const std::string &var : m_intVariables){
      CP::SysWriteDecorHandle<int> whandle{var+"_%SYS%", this};
      m_Ibranches.emplace(var, whandle);
      ATH_CHECK(m_Ibranches.at(var).initialize(m_systematicsList, m_eventHandle));
    }

    if (!m_isBtag.empty()){
      ATH_CHECK(m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    ATH_CHECK (m_nmuons.initialize(m_systematicsList, m_jetHandle));

    // Initialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());


    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsMultileptonAlg::execute()
  {
    // Loop over all systematics
    for (const auto& sys : m_systematicsList.systematicsVector()){
      // Retrieve inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      const xAOD::TauJetContainer *taus = nullptr;
      ANA_CHECK (m_tauHandle.retrieve (taus, sys));

      // Retrieve KF jets container (separate from original)
      const xAOD::JetContainer *KFJets = nullptr;
      if (m_doKinematicFit)
      {
        ANA_CHECK(m_kfJetHandle.retrieve(KFJets, sys));
      }

      for (const std::string &string_var: m_floatVariables) {
        m_Fbranches.at(string_var).set(*event, -999., sys);
      }

      for (const auto& var: m_intVariables) {
        m_Ibranches.at(var).set(*event, -999, sys);
      }

      int n_electrons = electrons->size();
      int n_muons = muons->size();
      int n_taus = taus->size();
      int n_totalJets = jets->size();
      int n_centralJets = 0;

      // seperate jets into b-jets and light jets
      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      auto lightJets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      auto centralJets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);

      for(const xAOD::Jet* jet : *jets) {
        // count central jets
        if (std::abs(jet->eta())<2.5) {
          n_centralJets++;
          centralJets->push_back(jet);
          if (WPgiven && m_isBtag.get(*jet, sys)){
            bjets->push_back(jet);
          }
          else {
            lightJets->push_back(jet);
          }
        }
        else {
          lightJets->push_back(jet);
        }
      }
      int n_bjets = bjets->size();
      int n_lightJets = lightJets->size();

      // Select Higgs candidate jets from ORIGINAL container
      const xAOD::Jet *Hbb_Jet1 = nullptr;
      const xAOD::Jet *Hbb_Jet2 = nullptr;
      if (n_centralJets >= 2)
      {
        if (bjets->size() == 0)
        {
          Hbb_Jet1 = centralJets->at(0);
          Hbb_Jet2 = centralJets->at(1);
        }
        else if (bjets->size() == 1)
        {
          Hbb_Jet1 = bjets->at(0);
          int index2 = (centralJets->at(0) == Hbb_Jet1) ? 1 : 0;
          Hbb_Jet2 = centralJets->at(index2);
        }
        else
        {
          Hbb_Jet1 = bjets->at(0);
          Hbb_Jet2 = bjets->at(1);
        }
      }

      // === KF JETS SELECTION (apply IDENTICAL logic to KF container) ===
      const xAOD::Jet *Hbb_KFJet1 = nullptr;
      const xAOD::Jet *Hbb_KFJet2 = nullptr;
      int n_KF_centralJets = 0; // Declare this variable

      if (m_doKinematicFit && KFJets && KFJets->size() >= 2)
      {
        auto KF_bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>>(
            SG::VIEW_ELEMENTS);
        auto KF_centralJets =
            std::make_unique<ConstDataVector<xAOD::JetContainer>>(
                SG::VIEW_ELEMENTS);

        // Apply SAME selection criteria to KF jets
        for (const xAOD::Jet *KFjet : *KFJets)
        {
          if (std::abs(KFjet->eta()) < 2.5)
          {
            n_KF_centralJets++;
            KF_centralJets->push_back(KFjet);
            if (WPgiven && m_isBtag.get(*KFjet, sys))
            {
              KF_bjets->push_back(KFjet);
            }
          }
        }

        if (n_KF_centralJets >= 2)
        {
          if (KF_bjets->size() == 0)
          {
            Hbb_KFJet1 = KF_centralJets->at(0); 
            Hbb_KFJet2 = KF_centralJets->at(1); 
          }
          else if (KF_bjets->size() == 1)
          {
            Hbb_KFJet1 = KF_bjets->at(0);
            int index2 = (KF_centralJets->at(0) == Hbb_KFJet1) ? 1 : 0;
            Hbb_KFJet2 = KF_centralJets->at(index2);
          }
          else
          {
            Hbb_KFJet1 = KF_bjets->at(0); 
            Hbb_KFJet2 = KF_bjets->at(1); 
          }
        }
      }

      m_Ibranches.at("nTotalJets").set(*event, n_totalJets, sys);
      m_Ibranches.at("nElectrons").set(*event, n_electrons, sys);
      m_Ibranches.at("nMuons").set(*event, n_muons, sys);
      m_Ibranches.at("nTaus").set(*event, n_taus, sys);
      m_Ibranches.at("nLightJets").set(*event, n_lightJets, sys);
      m_Ibranches.at("nBJets").set(*event, n_bjets, sys);
      m_Ibranches.at("nCentralJets").set(*event, n_centralJets, sys);

      auto classifier = SubChannelClassify(
          muons, electrons, taus, bjets.get()
          );

      m_Ibranches.at("nLeptons").set(
          *event, classifier.getNLeptons(), sys);
      m_Ibranches.at("totalLepCharge").set(
          *event, classifier.getTotalChargeLep(), sys);
      m_Ibranches.at("totalTauCharge").set(
          *event, classifier.getTotalChargeTau(), sys);
      m_Ibranches.at("subChannelID").set(
          *event, static_cast<int>(classifier.getSubChannelId()), sys);
      m_Ibranches.at("subChannelFlavor").set(
          *event, classifier.getSubChannelFlavor(), sys);

      if (m_save_extrabb4l_vars)
      {
        if (!Hbb_Jet1 || !Hbb_Jet2)
        {
          ATH_MSG_DEBUG("One or both Hbb jets are null pointers. Hbb_Jet1: "
                          << (Hbb_Jet1 ? "valid" : "null")
                          << ", Hbb_Jet2: " << (Hbb_Jet2 ? "valid" : "null"));
          return StatusCode::SUCCESS;
        }

        std::vector<const xAOD::Jet *> originalHbbJets = {Hbb_Jet1, Hbb_Jet2};

        if (m_doKinematicFit && Hbb_KFJet1 && Hbb_KFJet2) {
          TLorentzVector test_h_bb_kf = Hbb_KFJet1->p4() + Hbb_KFJet2->p4();
          if (std::isnan(test_h_bb_kf.M()) || std::isnan(test_h_bb_kf.Pt()) || 
              std::isnan(test_h_bb_kf.Eta()) || std::isnan(test_h_bb_kf.Phi())) {
              continue;
          } else {
              std::vector<const xAOD::Jet*> KFHbbJets = {Hbb_KFJet1, Hbb_KFJet2};
              ANA_CHECK(SaveExtrabb4lVars(originalHbbJets, KFHbbJets, event, sys));
          }
        }
      }
    }
    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsMultileptonAlg::SaveExtrabb4lVars(
    const std::vector<const xAOD::Jet*> &Hbbjets,
    const std::vector<const xAOD::Jet*> &KFJets,  
    const xAOD::EventInfo *event, 
    const auto& sys)
  {

    for(unsigned int i =0; i<2; i++){
        const xAOD::Jet* jet = Hbbjets[i];

        std::string prefix_bjet = "HbbCandidate_Jet"+std::to_string(i+1);
        if(m_nmuons.get(*jet,sys))
        m_Ibranches.at(prefix_bjet+"_n_muons").set
          (*event, m_nmuons.get(*jet, sys), sys);

        float uncorrPt = jet->jetP4("NoBJetCalibMomentum").Pt();
        m_Fbranches.at(prefix_bjet+"_uncorrPt").set(*event, uncorrPt, sys);
        float muonCorrPt = jet->jetP4("MuonCorrMomentum").Pt();
        m_Fbranches.at(prefix_bjet+"_muonCorrPt").set(*event, muonCorrPt, sys);

        if (i == 0){
          m_jet1_uncorr = jet->jetP4("NoBJetCalibMomentum");
          m_jet1_muonCorr = jet->jetP4("MuonCorrMomentum");
        }
        else if (i == 1){
          m_jet2_uncorr = jet->jetP4("NoBJetCalibMomentum");
          m_jet2_muonCorr = jet->jetP4("MuonCorrMomentum");
        }

        TLorentzVector jet_tlv = jet->p4();
        m_Fbranches.at(prefix_bjet+"_pt").set(*event, jet_tlv.Pt(), sys);
        m_Fbranches.at(prefix_bjet+"_eta").set(*event, jet_tlv.Eta(), sys);
        m_Fbranches.at(prefix_bjet+"_phi").set(*event, jet_tlv.Phi(), sys);
        m_Fbranches.at(prefix_bjet+"_E").set(*event, jet_tlv.E(), sys);
    }

    //NoBjetCalibCorrection
    xAOD::JetFourMom_t H_bb_uncorr = m_jet1_uncorr + m_jet2_uncorr;
    std::string prefix = "HbbCand_uncorr_";
    m_Fbranches.at(prefix+"m_bb").set(*event, H_bb_uncorr.M(), sys);
        m_Fbranches.at(prefix+"pt_bb").set(*event, H_bb_uncorr.Pt(), sys);
        m_Fbranches.at(prefix+"eta_bb").set(*event, H_bb_uncorr.Eta(), sys);
        m_Fbranches.at(prefix+"phi_bb").set(*event, H_bb_uncorr.Phi(), sys);
        m_Fbranches.at(prefix+"dR_bb").set(*event,ROOT::Math::VectorUtil::DeltaR(m_jet1_uncorr, m_jet2_uncorr), sys);
        m_Fbranches.at(prefix+"Jet1_pt").set(*event, m_jet1_uncorr.Pt(), sys);
        m_Fbranches.at(prefix+"Jet1_eta").set(*event, m_jet1_uncorr.Eta(), sys);
        m_Fbranches.at(prefix+"Jet1_phi").set(*event, m_jet1_uncorr.Phi(), sys);
        m_Fbranches.at(prefix+"Jet1_m").set(*event, m_jet1_uncorr.M(), sys);
        m_Fbranches.at(prefix+"Jet2_pt").set(*event, m_jet2_uncorr.Pt(), sys);
        m_Fbranches.at(prefix+"Jet2_eta").set(*event, m_jet2_uncorr.Eta(), sys);
        m_Fbranches.at(prefix+"Jet2_phi").set(*event, m_jet2_uncorr.Phi(), sys);
        m_Fbranches.at(prefix+"Jet2_m").set(*event, m_jet2_uncorr.M(), sys);

    //MuonCorr only
    xAOD::JetFourMom_t H_bb_muonCorr = m_jet1_muonCorr + m_jet2_muonCorr;
    prefix = "HbbCand_muonCorr_";
    m_Fbranches.at(prefix+"m_bb").set(*event, H_bb_muonCorr.M(), sys);
        m_Fbranches.at(prefix+"pt_bb").set(*event, H_bb_muonCorr.Pt(), sys);
        m_Fbranches.at(prefix+"eta_bb").set(*event, H_bb_muonCorr.Eta(), sys);
        m_Fbranches.at(prefix+"phi_bb").set(*event, H_bb_muonCorr.Phi(), sys);
        m_Fbranches.at(prefix+"dR_bb").set(*event,ROOT::Math::VectorUtil::DeltaR(m_jet1_muonCorr, m_jet2_muonCorr), sys);
        m_Fbranches.at(prefix+"Jet1_pt").set(*event, m_jet1_muonCorr.Pt(), sys);
        m_Fbranches.at(prefix+"Jet1_eta").set(*event, m_jet1_muonCorr.Eta(), sys);
        m_Fbranches.at(prefix+"Jet1_phi").set(*event, m_jet1_muonCorr.Phi(), sys);
        m_Fbranches.at(prefix+"Jet1_m").set(*event, m_jet1_muonCorr.M(), sys);
        m_Fbranches.at(prefix+"Jet2_pt").set(*event, m_jet2_muonCorr.Pt(), sys);
        m_Fbranches.at(prefix+"Jet2_eta").set(*event, m_jet2_muonCorr.Eta(), sys);
        m_Fbranches.at(prefix+"Jet2_phi").set(*event, m_jet2_muonCorr.Phi(), sys);
        m_Fbranches.at(prefix+"Jet2_m").set(*event, m_jet2_muonCorr.M(), sys);

    //MuonCorr + PTReco
   TLorentzVector H_bb = Hbbjets[0]->p4() + Hbbjets[1]->p4();
    prefix = "HbbCand_PtCorr_";
    m_Fbranches.at(prefix+"m_bb").set(*event, H_bb.M(), sys);
        m_Fbranches.at(prefix+"pt_bb").set(*event, H_bb.Pt(), sys);
        m_Fbranches.at(prefix+"eta_bb").set(*event, H_bb.Eta(), sys);
        m_Fbranches.at(prefix+"phi_bb").set(*event, H_bb.Phi(), sys);
        m_Fbranches.at(prefix+"dR_bb").set(*event, Hbbjets[0]->p4().DeltaR(Hbbjets[1]->p4()), sys);
        m_Fbranches.at(prefix+"Jet1_pt").set(*event, Hbbjets[0]->p4().Pt(), sys);
        m_Fbranches.at(prefix+"Jet1_eta").set(*event, Hbbjets[0]->p4().Eta(), sys);
        m_Fbranches.at(prefix+"Jet1_phi").set(*event, Hbbjets[0]->p4().Phi(), sys);
        m_Fbranches.at(prefix+"Jet1_m").set(*event, Hbbjets[0]->p4().M(), sys);
        m_Fbranches.at(prefix+"Jet2_pt").set(*event, Hbbjets[1]->p4().Pt(), sys);
        m_Fbranches.at(prefix+"Jet2_eta").set(*event, Hbbjets[1]->p4().Eta(), sys);
        m_Fbranches.at(prefix+"Jet2_phi").set(*event, Hbbjets[1]->p4().Phi(), sys);
        m_Fbranches.at(prefix+"Jet2_m").set(*event, Hbbjets[1]->p4().M(), sys);

    // Add kinematic fit information if available
   TLorentzVector H_bb_KF = KFJets[0]->p4() + KFJets[1]->p4();
    prefix = "HbbCand_KF_";
    m_Fbranches.at("HbbCand_KF_m_bb_unconstrained").set(*event, m_KF_MBB.get(*event, sys), sys);
    m_Fbranches.at(prefix + "m_bb").set(*event, H_bb_KF.M(), sys);
        m_Fbranches.at(prefix + "pt_bb").set(*event, H_bb_KF.Pt(), sys);
        m_Fbranches.at(prefix + "eta_bb").set(*event, H_bb_KF.Eta(), sys);
        m_Fbranches.at(prefix + "phi_bb").set(*event, H_bb_KF.Phi(), sys);
        m_Fbranches.at(prefix + "dR_bb").set(*event, KFJets[0]->p4().DeltaR(KFJets[1]->p4()), sys);
        m_Fbranches.at(prefix + "Jet1_pt").set(*event, KFJets[0]->p4().Pt(), sys);
        m_Fbranches.at(prefix + "Jet1_eta").set(*event, KFJets[0]->p4().Eta(), sys);
        m_Fbranches.at(prefix + "Jet1_phi").set(*event, KFJets[0]->p4().Phi(), sys);
        m_Fbranches.at(prefix + "Jet1_m").set(*event, KFJets[0]->p4().M(), sys);
        m_Fbranches.at(prefix + "Jet2_pt").set(*event, KFJets[1]->p4().Pt(), sys);
        m_Fbranches.at(prefix + "Jet2_eta").set(*event, KFJets[1]->p4().Eta(), sys);
        m_Fbranches.at(prefix + "Jet2_phi").set(*event, KFJets[1]->p4().Phi(), sys);
        m_Fbranches.at(prefix + "Jet2_m").set(*event, KFJets[1]->p4().M(), sys);

    return StatusCode::SUCCESS;

  }

}
