/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "BaselineVarsbbyyAlg.h"

#include "AthContainers/AuxElement.h"
#include <FourMomUtils/xAODP4Helpers.h>
#include "PathResolver/PathResolver.h"

#include "TMatrixDSym.h"
#include "TMatrixDSymEigen.h"
#include "TVectorD.h"
#include "TFile.h"

#include <AthenaKernel/Units.h>

namespace HHBBYY
{
  BaselineVarsbbyyAlg::BaselineVarsbbyyAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {}

  StatusCode BaselineVarsbbyyAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("       BaselineVarsbbyyAlg       \n");
    ATH_MSG_INFO("*********************************\n");

    ATH_CHECK (m_bbyyJetHandle.initialize(m_systematicsList));


    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_bbyyJetHandle));
    }

    if (!m_PCBT.empty()) {
      ATH_CHECK (m_PCBT.initialize(m_systematicsList, m_bbyyJetHandle));
    }

    if (m_isMC) ATH_CHECK (m_truthFlav.initialize(m_systematicsList, m_bbyyJetHandle));
    ATH_CHECK (m_nmuons.initialize(m_systematicsList, m_bbyyJetHandle));

    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK(m_year.initialize(m_systematicsList, m_eventHandle));
    if(m_doKF){
      ATH_CHECK (m_KFJetHandle.initialize(m_systematicsList));
      ATH_CHECK (m_KF_MBB.initialize(m_systematicsList, m_eventHandle));
    }

    for (const std::string &string_var: m_floatVariables) {
      CP::SysWriteDecorHandle<float> var {string_var+"_%SYS%", this};
      m_Fbranches.emplace(string_var, var);
      ATH_CHECK (m_Fbranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }

    for (const std::string &string_var: m_intVariables) {
      CP::SysWriteDecorHandle<int> var {string_var+"_%SYS%", this};
      m_Ibranches.emplace(string_var, var);
      ATH_CHECK (m_Ibranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }

    // Load BDT models
    if(m_do_nonresonant_BDTs){
      for (const std::string &path: m_bdts_path){
        std::unique_ptr<MVAUtils::BDT> bdt;
        loadBDT(path, bdt);
        m_bdts.push_back(std::move(bdt));
      }
    }

    // Load GNN models
    if(m_doGNN_tagging)
    {
      for (const std::string &path: m_GNNs_path)
      {
        loadGNN(path);
      }
    }

    // convert string to enum (VBFjetsMethod)
    m_vbfjets_method = stringToVBFjetsMethod(m_vbfjets_method_str);

    // Check compatibility of flags
    if(!m_do_nonresonant_BDTs && (m_vbfjets_method == HHBBYY::VBFjetsMethod::BDT)){
      ANA_MSG_ERROR("do_nonresonant_BDTs set to false, but VBF jets method set to BDT. getVBFjets_BDT only works if BDTs are loaded.");
      return StatusCode::FAILURE;
    }

    ATH_CHECK (m_Photon1_pt.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_Photon1_eta.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_Photon1_phi.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_Photon1_E.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK (m_Photon2_pt.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_Photon2_eta.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_Photon2_phi.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_Photon2_E.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK (m_nPhotons.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsbbyyAlg::execute()
  {
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      // In case of special Higgs sample, run only on NOSYS
      if (!m_doSystematics && sys.name()!="") continue;

      // container we read in
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_bbyyJetHandle.retrieve (jets, sys));

      const xAOD::MissingETContainer *metCont = nullptr;
      ANA_CHECK (m_metHandle.retrieve (metCont, sys));
      const xAOD::MissingET* met = (*metCont)["Final"];
      if (!met) {
        ATH_MSG_ERROR("Could not retrieve MET");
        return StatusCode::FAILURE;
      }
      const xAOD::JetContainer *KFJets = nullptr;
      if(m_doKF){
        ANA_CHECK (m_KFJetHandle.retrieve (KFJets, sys));
      }

      // initialize
      TLorentzVector H_bb(0.,0.,0.,0.);
      TLorentzVector H_yy(0.,0.,0.,0.);
      TLorentzVector HH(0.,0.,0.,0.);
      int year = m_year.get(*event, sys);

      int j_passWP=-99;
      int PCBTjet = -99;

      // Maps for per-event outputs
      std::map<HHBBYY::Var, float> eventFloats;
      std::map<HHBBYY::Var, int> eventInts;
      // Initialize all variables to -99
      for (int i=0; i<HHBBYY::Var::size_enum; i++) {
        eventFloats[(HHBBYY::Var)i] = -99.;
        eventInts[(HHBBYY::Var)i] = -99;
      }

      for (const std::string &string_var: m_floatVariables) {
        m_Fbranches.at(string_var).set(*event, -99., sys);
      }

      for (const std::string &string_var: m_intVariables) {
        m_Ibranches.at(string_var).set(*event, -99, sys);
      }

      int nCentralJets = 0;
      double HT = 0.; // scalar sum of jet pT
      double KF_HT = 0;

      bool WPgiven = !m_isBtag.empty();
      bool PCBTgiven = !m_PCBT.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      auto KF_bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);

      for(const xAOD::Jet* jet : *jets) {
        // Compute scalar pt sum (Ht) for all the jets in the event |eta|<4.5
        HT += jet->pt();

        // count central jets
        if (std::abs(jet->eta())<2.5){
          nCentralJets++;
          // check if jet is btagged
          if (WPgiven && m_isBtag.get(*jet, sys)) bjets->push_back(jet);
        }
      }

      eventFloats.at(HHBBYY::Var::jets_HT) = HT;

      // inclusive jet sector
      if (m_save_nonresonant_BDTInput_variables){
        for (std::size_t i=0; i<std::min(jets->size(),(std::size_t)4); i++){
          TLorentzVector j = jets->at(i)->p4();
          j_passWP = static_cast<int>(m_isBtag.get(*jets->at(i), sys));
          PCBTjet= m_PCBT.get(*jets->at(i), sys);

          m_Fbranches.at("Jet"+std::to_string(i+1)+"_pt").set(*event, j.Pt(), sys);
          m_Fbranches.at("Jet"+std::to_string(i+1)+"_eta").set(*event, j.Eta(), sys);
          m_Fbranches.at("Jet"+std::to_string(i+1)+"_phi").set(*event, j.Phi(), sys);
          m_Fbranches.at("Jet"+std::to_string(i+1)+"_E").set(*event, j.E(), sys);

          m_Ibranches.at("Jet"+std::to_string(i+1)+"_PassWP").set(*event,j_passWP,sys);

          if (PCBTgiven)
            m_Ibranches.at("Jet"+std::to_string(i+1)+"_pcbt").set(*event,PCBTjet,sys);

          if (m_isMC){
            m_Ibranches.at("Jet"+std::to_string(i+1)+"_truthLabel").set
              (*event, m_truthFlav.get(*jets->at(i), sys), sys);
          }
        }
      }

      const xAOD::Jet *Hbb_Jet1 = nullptr;
      const xAOD::Jet *Hbb_Jet2 = nullptr;
      if (jets->size() >= 2) {
        if (bjets->size() == 0 ) {
          Hbb_Jet1 = jets->at(0);
          Hbb_Jet2 = jets->at(1);
        } else if (bjets->size() ==1 ) {
          Hbb_Jet1 = bjets->at(0);
          int index2 = (jets->at(0)==Hbb_Jet1) ? 1 : 0;
          Hbb_Jet2 = jets->at(index2);

        } else{
          Hbb_Jet1 = bjets->at(0);
          Hbb_Jet2 = bjets->at(1);
        }
        H_bb = Hbb_Jet1->p4() + Hbb_Jet2->p4();
      }

      std::vector<const xAOD::Jet*> Hbb_jets = {Hbb_Jet1, Hbb_Jet2};

      TLorentzVector ph1(0.,0.,0.,0.);
      ph1.SetPtEtaPhiE(m_Photon1_pt.get(*event, sys),
                       m_Photon1_eta.get(*event, sys),
                       m_Photon1_phi.get(*event, sys),
                       m_Photon1_E.get(*event, sys));
      TLorentzVector ph2(0.,0.,0.,0.);
        ph2.SetPtEtaPhiE(m_Photon2_pt.get(*event, sys),
                         m_Photon2_eta.get(*event, sys),
                         m_Photon2_phi.get(*event, sys),
                         m_Photon2_E.get(*event, sys));
      int n_photons = m_nPhotons.get(*event, sys);

      std::vector<TLorentzVector> Hyy_photons = {ph1, ph2};

      // Fill the bb and bbyy branches
      if (jets->size() >= 2) {
        fill_bb_branches(Hbb_jets, "", event, sys);
        if (m_doKF) {eventFloats.at(HHBBYY::Var::bb_m_KF_unconstrained) = m_KF_MBB.get(*event, sys);}

        if(n_photons >= 2){
          H_yy = ph1 + ph2;
          HH = H_yy + H_bb;
          double Higgs_mass = 125. * Athena::Units::GeV;
          float bbyy_mStar = HH.M() - (H_bb.M() - Higgs_mass)-(H_yy.M() - Higgs_mass);
          eventFloats.at(HHBBYY::Var::bbyy_mStar) = bbyy_mStar;
          fill_bbyy_branches(Hbb_jets, Hyy_photons, "", event, sys);
        }
      }

      // More global variables
      if(m_save_nonresonant_BDTInput_variables){
        float topness = compute_Topness(jets);
        m_Fbranches.at("topness").set(*event, topness, sys);

        std::vector<float> eventShapes = compute_EventShapes(Hbb_Jet1, Hbb_Jet2, Hyy_photons, n_photons);
        m_Fbranches.at("sphericityT").set(*event, eventShapes[0], sys);
        m_Fbranches.at("planarFlow").set(*event, eventShapes[1], sys);

        eventFloats.at(HHBBYY::Var::sphericityT) = eventShapes[0];
        eventFloats.at(HHBBYY::Var::planarFlow) = eventShapes[1];
        eventFloats.at(HHBBYY::Var::topness) = topness;

        float pTBalance = compute_pTBalance(Hbb_Jet1, Hbb_Jet2, Hyy_photons, n_photons);
        m_Fbranches.at("pTBalance").set(*event, pTBalance, sys);
        m_Fbranches.at("HT").set(*event, HT, sys);
      }

      m_Ibranches.at("nJets").set(*event, jets->size(), sys);
      m_Ibranches.at("nCentralJets").set(*event, nCentralJets, sys);
      m_Ibranches.at("nBJets").set(*event, bjets->size(), sys);

      // bdt (vbf jets selection)
      ATH_CHECK(vbf_calculations(ph1,ph2,n_photons,Hbb_Jet1, Hbb_Jet2, jets, HT, HH, "Jet_vbf_j", "Jet_vbf_jj",eventFloats, event, sys));

      // bdt (low and high mHH categorations)
      if(m_do_nonresonant_BDTs){
        if (n_photons >= 2 && Hbb_Jet1 && Hbb_Jet2) {
          performCategorisationBDT(ph1, ph2, Hbb_Jet1, Hbb_Jet2, jets, metCont, sys, eventFloats, eventInts, false, false, false, false, year);
        }
        m_Fbranches.at("bdtSel_score").set(*event, eventFloats.at(HHBBYY::Var::bdt_sel_score), sys);
        m_Ibranches.at("bdtSel_category").set(*event, eventInts.at(HHBBYY::Var::bdt_sel_category), sys);
      }

      //KF jets sector
      if (m_doKF){
        for(const xAOD::Jet* KFjet : *KFJets) {
          // Compute scalar pt sum (Ht) for all the jets in the event |eta|<4.5
          KF_HT += KFjet->pt();
            // check if jet is btagged
            if (WPgiven && m_isBtag.get(*KFjet, sys)) KF_bjets->push_back(KFjet);
          }
          m_Fbranches.at("KF_HT").set(*event, KF_HT, sys);
          eventFloats.at(HHBBYY::Var::jets_HT_KF) = KF_HT;

          //inclusive jets sector
          for (std::size_t i=0; i<std::min(KFJets->size(),(std::size_t)4); i++){
            TLorentzVector j = KFJets->at(i)->p4();
            m_Fbranches.at("KF_Jet"+std::to_string(i+1)+"_pt").set(*event, j.Pt(), sys);
            m_Fbranches.at("KF_Jet"+std::to_string(i+1)+"_eta").set(*event, j.Eta(), sys);
            m_Fbranches.at("KF_Jet"+std::to_string(i+1)+"_phi").set(*event, j.Phi(), sys);
            m_Fbranches.at("KF_Jet"+std::to_string(i+1)+"_E").set(*event, j.E(), sys);
          }

          //KF candidate jets, Hbb variables
          const xAOD::Jet* Hbb_KFJet1 = nullptr;
          const xAOD::Jet* Hbb_KFJet2 = nullptr;
          if (KFJets->size() >= 2) {
            if (KF_bjets->size() == 0 ) {
              Hbb_KFJet1 = KFJets->at(0);
              Hbb_KFJet2 = KFJets->at(1);
            } else if (KF_bjets->size() ==1 ) {
              Hbb_KFJet1 = KF_bjets->at(0);
              int index2 = (KFJets->at(0)==Hbb_KFJet1) ? 1 : 0;
              Hbb_KFJet2 = KFJets->at(index2);

            } else{
              Hbb_KFJet1 = KF_bjets->at(0);
              Hbb_KFJet2 = KF_bjets->at(1);
            }

            std::vector<TLorentzVector> Hbb_KFcandidates = {Hbb_KFJet1->p4(), Hbb_KFJet2->p4()};

            if (m_save_HbbCand_vars){
              for(unsigned int i=0; i<2; i++){
                std::string prefix = "KF_HbbCandidate_Jet"+std::to_string(i+1);
                m_Fbranches.at(prefix+"_pt").set(*event, Hbb_KFcandidates[i].Pt(), sys);
                m_Fbranches.at(prefix+"_eta").set(*event, Hbb_KFcandidates[i].Eta(), sys);
                m_Fbranches.at(prefix+"_phi").set(*event, Hbb_KFcandidates[i].Phi(), sys);
                m_Fbranches.at(prefix+"_E").set(*event, Hbb_KFcandidates[i].E(), sys);
              }
            }

            TLorentzVector H_bb_KF = Hbb_KFcandidates[0] + Hbb_KFcandidates[1];
            float KF_dRbb = (Hbb_KFcandidates[0]).DeltaR(Hbb_KFcandidates[1]);

            m_Fbranches.at("KF_pTbb").set(*event, H_bb_KF.Pt(), sys);
            m_Fbranches.at("KF_Etabb").set(*event, H_bb_KF.Eta(), sys);
            m_Fbranches.at("KF_Phibb").set(*event, H_bb_KF.Phi(), sys);
            m_Fbranches.at("KF_dRbb").set(*event, KF_dRbb, sys);

            //build the KF HH candidate
            double Higgs_mass = 125. * Athena::Units::GeV;
            TLorentzVector HH_KF = H_yy + H_bb_KF;
            float KF_bbyy_mStar = HH_KF.M() - (H_bb_KF.M() - Higgs_mass)-(H_yy.M() - Higgs_mass);
            eventFloats.at(HHBBYY::Var::bbyy_mStar_KF) = KF_bbyy_mStar;
            m_Fbranches.at("KF_mbbyy").set(*event, HH_KF.M(), sys);
            m_Fbranches.at("KF_mbbyy_star").set(*event, KF_bbyy_mStar, sys);
            m_Fbranches.at("KF_pTbbyy").set(*event, HH_KF.Pt(), sys);
            m_Fbranches.at("KF_Etabbyy").set(*event, HH_KF.Eta(), sys);
            m_Fbranches.at("KF_Phibbyy").set(*event, HH_KF.Phi(), sys);
            m_Fbranches.at("KF_dRHH").set(*event, H_yy.DeltaR(H_bb_KF), sys);

            //vbf jets selection KF
            ATH_CHECK(vbf_calculations(ph1,ph2,n_photons,Hbb_KFJet1, Hbb_KFJet2, KFJets, KF_HT, HH_KF, "KF_Jet_vbf_j", "KF_Jet_vbf_jj", eventFloats, event, sys));
          }

          //mva variables
          float KF_topness = compute_Topness(KFJets);
          std::vector<float> KF_eventShapes = compute_EventShapes(Hbb_KFJet1, Hbb_KFJet2, Hyy_photons, n_photons);
          float KF_pTBalance = compute_pTBalance(Hbb_KFJet1, Hbb_KFJet2, Hyy_photons, n_photons);
          m_Fbranches.at("KF_topness").set(*event, KF_topness, sys);
          m_Fbranches.at("KF_sphericityT").set(*event, KF_eventShapes[0], sys);
          m_Fbranches.at("KF_planarFlow").set(*event, KF_eventShapes[1], sys);
          m_Fbranches.at("KF_pTBalance").set(*event, KF_pTBalance, sys);
          eventFloats.at(HHBBYY::Var::sphericityT_KF) = KF_eventShapes[0];
          eventFloats.at(HHBBYY::Var::planarFlow_KF) = KF_eventShapes[1];
          eventFloats.at(HHBBYY::Var::topness_KF) = KF_topness;

          if(m_do_nonresonant_BDTs){
            if (n_photons >= 2 && Hbb_KFJet1 && Hbb_KFJet2){
              performCategorisationBDT(ph1, ph2, Hbb_KFJet1, Hbb_KFJet2, KFJets, metCont, sys, eventFloats, eventInts, true, false, false, false, year);
              m_Fbranches.at("KF_bdtSel_score").set(*event, eventFloats.at(HHBBYY::Var::bdt_sel_score_KF), sys);
              m_Ibranches.at("KF_bdtSel_category").set(*event, eventInts.at(HHBBYY::Var::bdt_sel_category_KF), sys);

              // correlated
              performCategorisationBDT(ph1, ph2, Hbb_KFJet1, Hbb_KFJet2, KFJets, metCont, sys, eventFloats, eventInts, true, false, true, false, year);
              m_Fbranches.at("KF_corr_bdtSel_score").set(*event, eventFloats.at(HHBBYY::Var::bdt_sel_score_KF_corr), sys);
              m_Ibranches.at("KF_corr_bdtSel_category").set(*event, eventInts.at(HHBBYY::Var::bdt_sel_category_KF_corr), sys);

              // with 2024 data
              performCategorisationBDT(ph1, ph2, Hbb_KFJet1, Hbb_KFJet2, KFJets, metCont, sys, eventFloats, eventInts, true, false, false, true, year);
              m_Fbranches.at("KF_with2024_bdtSel_score").set(*event, eventFloats.at(HHBBYY::Var::bdt_sel_score_KF_with2024), sys);
              m_Ibranches.at("KF_with2024_bdtSel_category").set(*event, eventInts.at(HHBBYY::Var::bdt_sel_category_KF_with2024), sys);

              // correlated with 2024 data
              performCategorisationBDT(ph1, ph2, Hbb_KFJet1, Hbb_KFJet2, KFJets, metCont, sys, eventFloats, eventInts, true, false, true, true, year);
              m_Fbranches.at("KF_corr_with2024_bdtSel_score").set(*event, eventFloats.at(HHBBYY::Var::bdt_sel_score_KF_corr_with2024), sys);
              m_Ibranches.at("KF_corr_with2024_bdtSel_category").set(*event, eventInts.at(HHBBYY::Var::bdt_sel_category_KF_corr_with2024), sys);
            }
          }
      }

      //GNN Implementation
      if(m_doGNN_tagging)
      {
        float pile_up;
        if(2015<=year && year<=2016) {
          pile_up = event->averageInteractionsPerCrossing();
        }
        else {
          pile_up = event->actualInteractionsPerCrossing();
        }
        for(int i=0; i<2; i++)
        {
          if(nCentralJets>=2 && n_photons>=2)
          {
            float GNN_maxscore = -99.;
            float GNN_vbf_jj_maxscore=0.;
            std::string prefix;
            std::vector<const xAOD::Jet*> GNN_jets;
            std::array<TLorentzVector, 2> GNN_vbf_j;

            if(i == HHBBYY::GNN::ggFTarget) {
              prefix = "ggFTarget";
              GNN_jets = getHbb_GNN_ggFTarget(jets, ph1, ph2, GNN_maxscore, pile_up, sys);

              if(m_vbfjets_method == HHBBYY::VBFjetsMethod::BDT) {
                GNN_vbf_jj_maxscore = getVBFjets_BDT(HT, ph1, ph2, GNN_jets[0], GNN_jets[1], jets, GNN_vbf_j);
              }else if(m_vbfjets_method == HHBBYY::VBFjetsMethod::mjj) {
                getVBFjets_mjj(GNN_jets[0], GNN_jets[1], jets, GNN_vbf_j);
              }else if(m_vbfjets_method == HHBBYY::VBFjetsMethod::pTsorting) {
                getVBFjets_pTsorting(GNN_jets[0], GNN_jets[1], jets, GNN_vbf_j);
              }else if(m_vbfjets_method == HHBBYY::VBFjetsMethod::invalid) {
                ANA_MSG_ERROR("Invalid vbfjets method imported!!! The default BDT method is called!!!");
                return StatusCode::FAILURE;
              }
            }
            else if(i == HHBBYY::GNN::VBFTarget) {
              if(jets->size()<4) continue;
              prefix = "VBFTarget";
              GNN_jets = getHbb_GNN_VBFTarget(jets, ph1, ph2, GNN_maxscore, pile_up, sys);
              GNN_vbf_j[0] = GNN_jets[2]->p4();
              GNN_vbf_j[1] = GNN_jets[3]->p4();
              GNN_vbf_jj_maxscore = GNN_maxscore;
            }

            fill_bb_branches(GNN_jets, "GNN_" + prefix + "_", event, sys);
            fill_bbyy_branches(GNN_jets, Hyy_photons, "GNN_" + prefix + "_", event, sys);
            m_Fbranches.at("GNN_" + prefix + "_maxscore").set(*event, GNN_maxscore, sys);

            double Higgs_mass = 125. * Athena::Units::GeV;
            TLorentzVector GNN_H_bb = GNN_jets[0]->p4() + GNN_jets[1]->p4();
            TLorentzVector GNN_HH = H_yy + GNN_H_bb;
            float GNN_bbyy_mStar = GNN_HH.M() - (GNN_H_bb.M() - Higgs_mass)-(H_yy.M() - Higgs_mass);
            eventFloats.at(HHBBYY::Var::bbyy_mStar_GNN) = GNN_bbyy_mStar;

            TLorentzVector GNN_vbf_jj = GNN_vbf_j[0] + GNN_vbf_j[1];
            m_Fbranches.at("GNN_" + prefix + "_Jet_vbf_jj_maxscore").set(*event, GNN_vbf_jj_maxscore, sys);
            m_Fbranches.at("GNN_" + prefix + "_Jet_vbf_jj_m").set(*event, GNN_vbf_jj.M(), sys);
            m_Fbranches.at("GNN_" + prefix + "_Jet_vbf_jj_deta").set(*event, std::fabs(GNN_vbf_j[0].Eta() - GNN_vbf_j[1].Eta()), sys);
            eventFloats.at(HHBBYY::Var::vbfjj_m_GNN) = GNN_vbf_jj.M();
            eventFloats.at(HHBBYY::Var::vbfjj_dEta_GNN) = std::fabs(GNN_vbf_j[0].Eta() - GNN_vbf_j[1].Eta());

            std::vector<float> GNN_eventShapes = compute_EventShapes(GNN_jets[0], GNN_jets[1], Hyy_photons, n_photons);
            m_Fbranches.at("GNN_" + prefix + "_sphericityT").set(*event, GNN_eventShapes[0], sys);
            m_Fbranches.at("GNN_" + prefix + "_planarFlow").set(*event, GNN_eventShapes[1], sys);
            eventFloats.at(HHBBYY::Var::sphericityT_GNN) = GNN_eventShapes[0];
            eventFloats.at(HHBBYY::Var::planarFlow_GNN) = GNN_eventShapes[1];

            float GNN_pTBalance = compute_pTBalance(GNN_jets[0], GNN_jets[1], Hyy_photons, n_photons);
            m_Fbranches.at("GNN_" + prefix + "_pTBalance").set(*event, GNN_pTBalance, sys);

            performCategorisationBDT(ph1, ph2, GNN_jets[0], GNN_jets[1], jets, metCont, sys, eventFloats, eventInts, false, true, false, false, year);
            m_Fbranches.at("GNN_" + prefix + "_bdtSel_score").set(*event, eventFloats.at(HHBBYY::Var::bdt_sel_score_GNN), sys);
            m_Ibranches.at("GNN_" + prefix + "_bdtSel_category").set(*event, eventInts.at(HHBBYY::Var::bdt_sel_category_GNN), sys);
          }
        }
      }
    }

    return StatusCode::SUCCESS;
  }

  void BaselineVarsbbyyAlg::fill_bb_branches(const std::vector<const xAOD::Jet*> &Hbb_jets, const std::string &prefix, const xAOD::EventInfo *event, const auto& sys) {

    if (m_save_HbbCand_vars){
      for(unsigned int i =0; i<2; i++){
        const xAOD::Jet* jet = Hbb_jets[i];

        bool PCBTgiven = !m_PCBT.empty();

        std::string prefix_bjet = prefix + "HbbCandidate_Jet"+std::to_string(i+1);
        m_Ibranches.at(prefix_bjet+"_n_muons").set
	        (*event, m_nmuons.get(*jet, sys), sys);

        if(m_save_extra_vars){
          float uncorrPt = jet->jetP4("NoBJetCalibMomentum").Pt();
          m_Fbranches.at(prefix_bjet+"_uncorrPt").set(*event, uncorrPt, sys);
          float muonCorrPt = jet->jetP4("MuonCorrMomentum").Pt();
          m_Fbranches.at(prefix_bjet+"_muonCorrPt").set(*event, muonCorrPt, sys);
        }

        TLorentzVector jet_tlv = jet->p4();
        m_Fbranches.at(prefix_bjet+"_pt").set(*event, jet_tlv.Pt(), sys);
        m_Fbranches.at(prefix_bjet+"_eta").set(*event, jet_tlv.Eta(), sys);
        m_Fbranches.at(prefix_bjet+"_phi").set(*event, jet_tlv.Phi(), sys);
        m_Fbranches.at(prefix_bjet+"_E").set(*event, jet_tlv.E(), sys);


        if(prefix.find("GNN") != std::string::npos)
          m_Ibranches.at(prefix_bjet+"_PassWP").set(*event, static_cast<int>(m_isBtag.get(*jet,sys)), sys);
        if(PCBTgiven)
          m_Ibranches.at(prefix_bjet+"_pcbt").set(*event, m_PCBT.get(*jet, sys), sys);

        if(m_isMC){
          m_Ibranches.at(prefix_bjet+"_truthLabel").set
	        (*event, m_truthFlav.get(*jet, sys), sys);
        }
      }
    }

    TLorentzVector H_bb = Hbb_jets[0]->p4() + Hbb_jets[1]->p4();

    m_Fbranches.at(prefix+"mbb").set(*event, H_bb.M(), sys);
    m_Fbranches.at(prefix+"pTbb").set(*event, H_bb.Pt(), sys);
    m_Fbranches.at(prefix+"Etabb").set(*event, H_bb.Eta(), sys);
    m_Fbranches.at(prefix+"Phibb").set(*event, H_bb.Phi(), sys);
    m_Fbranches.at(prefix+"dRbb").set(*event, Hbb_jets[0]->p4().DeltaR(Hbb_jets[1]->p4()), sys);
  }

  void BaselineVarsbbyyAlg::fill_bbyy_branches(const std::vector<const xAOD::Jet*> &Hbb_jets, const std::vector<TLorentzVector> &Hyy_photons, const std::string &prefix, const xAOD::EventInfo *event, const auto& sys) {
    TLorentzVector H_bb = Hbb_jets[0]->p4() + Hbb_jets[1]->p4();
    TLorentzVector H_yy = Hyy_photons[0] + Hyy_photons[1];

    TLorentzVector HH = H_yy + H_bb;

    double Higgs_mass = 125. * Athena::Units::GeV;
    float bbyy_mStar = HH.M() - (H_bb.M() - Higgs_mass)-(H_yy.M() - Higgs_mass);

    m_Fbranches.at(prefix+"mbbyy").set(*event, HH.M(), sys);
    m_Fbranches.at(prefix+"mbbyy_star").set(*event, bbyy_mStar, sys);
    m_Fbranches.at(prefix+"pTbbyy").set(*event, HH.Pt(), sys);
    m_Fbranches.at(prefix+"Etabbyy").set(*event, HH.Eta(), sys);
    m_Fbranches.at(prefix+"Phibbyy").set(*event, HH.Phi(), sys);
    m_Fbranches.at(prefix+"dRbbyy").set(*event, H_yy.DeltaR(H_bb), sys);

    // Want to compute the three body mass for the SHbbyy
    // analysis, onebtag region (variable used for PNN)
    if (m_doResonantonebtag){
      TLorentzVector H_byy = Hbb_jets[0]->p4() + H_yy;
      m_Fbranches.at(prefix+"mbyy").set(*event, H_byy.M(), sys);
    }

    if (m_save_extra_vars){
      std::vector<double> vec_angular_variables_CM=compute_angular_variables_CM(Hyy_photons[0], Hyy_photons[1], Hbb_jets[0]->p4(), Hbb_jets[1]->p4());

      m_Fbranches.at(prefix+"cos_theta_yy_cm_bbyy").set(*event,vec_angular_variables_CM[0],sys);
      m_Fbranches.at(prefix+"phi_yy_cm_bbyy").set(*event,vec_angular_variables_CM[1], sys);

      m_Fbranches.at(prefix+"Photon1_cos_theta_cm_gamgam").set(*event,vec_angular_variables_CM[2], sys);
      m_Fbranches.at(prefix+"Photon1_phi_cm_gamgam").set(*event,vec_angular_variables_CM[3], sys);

      m_Fbranches.at(prefix+"HbbCandidate_Jet1_cos_theta_cm_bb").set(*event,vec_angular_variables_CM[4], sys);
      m_Fbranches.at(prefix+"HbbCandidate_Jet1_phi_cm_bb").set(*event,vec_angular_variables_CM[5], sys);
      m_Fbranches.at(prefix+"DeltaPhi_bb_yy_cm_bbyy").set(*event,vec_angular_variables_CM[6], sys);
    }

  }

  void BaselineVarsbbyyAlg::loadGNN(const std::string &filePath) {
    std::string resolvedPath = PathResolverFindCalibFile(filePath);
    std::unique_ptr<Ort::Env> env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_FATAL, "");

    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(1);
    session_options.SetLogSeverityLevel(4);
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);

    std::unique_ptr<Ort::Session> session = std::make_unique<Ort::Session>(*env, resolvedPath.c_str(), session_options);

    size_t num_input_nodes = session->GetInputCount();
    size_t num_output_nodes = session->GetOutputCount();

    std::vector<std::string> input_node_names;
    std::vector<std::string> output_node_names;
    std::vector<std::vector<int64_t>> input_node_dims_vector;
    std::vector<std::vector<int64_t>> output_node_dims_vector;
    std::vector<int64_t> input_node_dims_sum;
    std::vector<int64_t> output_node_dims_sum;

    Ort::AllocatorWithDefaultOptions allocator;

    for (size_t i = 0; i < num_input_nodes; i++) {
      std::string input_name = session->GetInputNameAllocated(i, allocator).get();
      input_node_names.push_back(input_name);

      Ort::TypeInfo type_info = session->GetInputTypeInfo(i);
      auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
      auto input_node_dims = tensor_info.GetShape();
      input_node_dims_vector.emplace_back(input_node_dims);

      int64_t sums = 1;
      for (size_t j = 0; j < input_node_dims.size(); j++) {
          sums *= input_node_dims[j];
      }
      input_node_dims_sum.emplace_back(sums);

    }

    for (size_t i = 0; i < num_output_nodes; i++) {
      std::string output_name = session->GetOutputNameAllocated(i, allocator).get();
      output_node_names.push_back(output_name);

      Ort::TypeInfo type_info = session->GetOutputTypeInfo(i);
      auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
      auto output_node_dims = tensor_info.GetShape();
      output_node_dims_vector.emplace_back(output_node_dims);

      int64_t sums = 1;
      for (size_t j = 0; j < output_node_dims.size(); j++) {
          sums *= output_node_dims[j];
      }
      output_node_dims_sum.emplace_back(sums);
    }

    m_sessions.push_back(std::move(session));
    m_envs.push_back(std::move(env));
    m_input_node_names.push_back(input_node_names);
    m_output_node_names.push_back(output_node_names);
    m_input_node_dims_vector.push_back(input_node_dims_vector);
    m_output_node_dims_vector.push_back(output_node_dims_vector);
    m_input_node_dims_sum.push_back(input_node_dims_sum);
    m_output_node_dims_sum.push_back(output_node_dims_sum);
  }

  std::vector<const xAOD::Jet*> BaselineVarsbbyyAlg::getHbb_GNN_ggFTarget(const xAOD::JetContainer *jets, const TLorentzVector& ph1, const TLorentzVector& ph2, float& max_score, float pile_up, const auto &sys){
    TLorentzVector H_yy = ph1 + ph2;

    std::vector<float> node3 = {static_cast<float>(H_yy.Pt()/H_yy.M()), static_cast<float>(H_yy.Eta()), static_cast<float>(H_yy.Phi()), static_cast<float>(H_yy.E()/H_yy.M()), -2., 1.};

    std::vector<const char*> input_node_names;
    std::vector<const char*> output_node_names;
    input_node_names.reserve(m_input_node_names.at(HHBBYY::GNN::ggFTarget).size());
    output_node_names.reserve(m_output_node_names.at(HHBBYY::GNN::ggFTarget).size());
    for(const auto& name : m_input_node_names.at(HHBBYY::GNN::ggFTarget)) {
      input_node_names.push_back(name.c_str());
    }
    for(const auto& name : m_output_node_names.at(HHBBYY::GNN::ggFTarget)) {
      output_node_names.push_back(name.c_str());
    }

    Ort::Session &session = *(m_sessions.at(HHBBYY::GNN::ggFTarget));

    std::vector<const xAOD::Jet*> GNN_jets;
    for(size_t i = 0; i < jets->size(); i++) {
      const xAOD::Jet *jet1 = jets->at(i);
      if(std::fabs(jet1->eta()) > 2.5) continue;

      for(size_t j = i+1; j < jets->size(); j++) {
        const xAOD::Jet *jet2 = jets->at(j);
        if(std::fabs(jet2->eta()) > 2.5) continue;

        if(jet2->pt()>jet1->pt()) std::swap(jet1, jet2);
        TLorentzVector j1 = jet1->p4();
        TLorentzVector j2 = jet2->p4();

        std::vector<Ort::Value> input_tensors;
        auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

        TLorentzVector jj = j1 + j2;
        int PCBT_j1 = m_PCBT.get(*jet1, sys);
        int PCBT_j2 = m_PCBT.get(*jet2, sys);

        std::vector<float> nodes;
        std::vector<float> edges;
        std::vector<float> globals;
        std::vector<long int> senders;
        std::vector<long int> receivers;
        std::vector<long int> n_node = {3};
        std::vector<long int> n_edge = {6};

        std::vector<float> node1 = {static_cast<float>(j1.Pt()/jj.M()), static_cast<float>(j1.Eta()), static_cast<float>(j1.Phi()), static_cast<float>(j1.E()/jj.M()), static_cast<float>(PCBT_j1), 0.};
        std::vector<float> node2 = {static_cast<float>(j2.Pt()/jj.M()), static_cast<float>(j2.Eta()), static_cast<float>(j2.Phi()), static_cast<float>(j2.E()/jj.M()), static_cast<float>(PCBT_j2), 0.};
        nodes.insert(nodes.end(), node1.begin(), node1.end());
        nodes.insert(nodes.end(), node2.begin(), node2.end());
        nodes.insert(nodes.end(), node3.begin(), node3.end());
        for(int k = 0; k < 3; k++)
        {
          for(int l = 0; l < 3; l++)
          {
            if(k == l) continue;
            senders.push_back(k);
            receivers.push_back(l);
            float dEta = nodes[k*node1.size()+1] - nodes[l*node1.size()+1];
            float dPhi = nodes[k*node1.size()+2] - nodes[l*node1.size()+2];
            if(dPhi > M_PI) dPhi -= 2* M_PI;
            if(dPhi < -M_PI) dPhi += 2* M_PI;
            float dR = std::sqrt(dEta*dEta + dPhi*dPhi);
            std::vector<float> edge = {dEta, dPhi, dR};
            edges.insert(edges.end(), edge.begin(), edge.end());
          }
        }

        globals.push_back(pile_up);

        input_tensors.push_back(Ort::Value::CreateTensor<float>(memory_info, nodes.data(), m_input_node_dims_sum.at(HHBBYY::GNN::ggFTarget)[0], m_input_node_dims_vector.at(HHBBYY::GNN::ggFTarget)[0].data(), m_input_node_dims_vector.at(HHBBYY::GNN::ggFTarget)[0].size()));
        input_tensors.push_back(Ort::Value::CreateTensor<float>(memory_info, edges.data(), m_input_node_dims_sum.at(HHBBYY::GNN::ggFTarget)[1], m_input_node_dims_vector.at(HHBBYY::GNN::ggFTarget)[1].data(), m_input_node_dims_vector.at(HHBBYY::GNN::ggFTarget)[1].size()));
        input_tensors.push_back(Ort::Value::CreateTensor<float>(memory_info, globals.data(), m_input_node_dims_sum.at(HHBBYY::GNN::ggFTarget)[2], m_input_node_dims_vector.at(HHBBYY::GNN::ggFTarget)[2].data(), m_input_node_dims_vector.at(HHBBYY::GNN::ggFTarget)[2].size()));
        input_tensors.push_back(Ort::Value::CreateTensor<long int>(memory_info, receivers.data(), m_input_node_dims_sum.at(HHBBYY::GNN::ggFTarget)[3], m_input_node_dims_vector.at(HHBBYY::GNN::ggFTarget)[3].data(), m_input_node_dims_vector.at(HHBBYY::GNN::ggFTarget)[3].size()));
        input_tensors.push_back(Ort::Value::CreateTensor<long int>(memory_info, senders.data(), m_input_node_dims_sum.at(HHBBYY::GNN::ggFTarget)[4], m_input_node_dims_vector.at(HHBBYY::GNN::ggFTarget)[4].data(), m_input_node_dims_vector.at(HHBBYY::GNN::ggFTarget)[4].size()));
        input_tensors.push_back(Ort::Value::CreateTensor<long int>(memory_info, n_node.data(), m_input_node_dims_sum.at(HHBBYY::GNN::ggFTarget)[5], m_input_node_dims_vector.at(HHBBYY::GNN::ggFTarget)[5].data(), m_input_node_dims_vector.at(HHBBYY::GNN::ggFTarget)[5].size()));
        input_tensors.push_back(Ort::Value::CreateTensor<long int>(memory_info, n_edge.data(), m_input_node_dims_sum.at(HHBBYY::GNN::ggFTarget)[6], m_input_node_dims_vector.at(HHBBYY::GNN::ggFTarget)[6].data(), m_input_node_dims_vector.at(HHBBYY::GNN::ggFTarget)[6].size()));

        std::vector<Ort::Value> output_tensors = session.Run(Ort::RunOptions{nullptr}, input_node_names.data(), input_tensors.data(), input_node_names.size(), output_node_names.data(), output_node_names.size());
        float score = *(output_tensors[0].GetTensorMutableData<float>());

        if(score > max_score) {
          max_score = score;
          GNN_jets = {jet1, jet2};
        }
      }
    }

    return GNN_jets;
  }

  std::vector<const xAOD::Jet*> BaselineVarsbbyyAlg::getHbb_GNN_VBFTarget(const xAOD::JetContainer *jets, const TLorentzVector& ph1, const TLorentzVector& ph2, float& max_score, float pile_up, const auto &sys){
    TLorentzVector H_yy = ph1 + ph2;

    std::vector<float> node3 = {static_cast<float>(H_yy.Pt()/H_yy.M()), static_cast<float>(H_yy.Eta()), static_cast<float>(H_yy.Phi()), static_cast<float>(H_yy.E()/H_yy.M()), -2., 1.};

    std::vector<const char*> input_node_names;
    std::vector<const char*> output_node_names;
    input_node_names.reserve(m_input_node_names.at(HHBBYY::GNN::VBFTarget).size());
    output_node_names.reserve(m_output_node_names.at(HHBBYY::GNN::VBFTarget).size());
    for(const auto& name : m_input_node_names.at(HHBBYY::GNN::VBFTarget)) {
      input_node_names.push_back(name.c_str());
    }
    for(const auto& name : m_output_node_names.at(HHBBYY::GNN::VBFTarget)) {
      output_node_names.push_back(name.c_str());
    }

    Ort::Session &session = *(m_sessions.at(HHBBYY::GNN::VBFTarget));

    std::vector<const xAOD::Jet*> GNN_jets;
    for(size_t i = 0; i < jets->size(); i++) {
      const xAOD::Jet *jet1 = jets->at(i);
      if(std::abs(jet1->eta()) > 2.5) continue;

      for(size_t j = i+1; j < jets->size(); j++) {
        const xAOD::Jet *jet2 = jets->at(j);
        if(std::abs(jet2->eta()) > 2.5) continue;
        if(jet2->pt()>jet1->pt()) std::swap(jet1, jet2);

        TLorentzVector j1 = jet1->p4();
        TLorentzVector j2 = jet2->p4();
        TLorentzVector jj = j1 + j2;
        int PCBT_j1 = m_PCBT.get(*jet1, sys);
        int PCBT_j2 = m_PCBT.get(*jet2, sys);

        std::vector<float> node1 = {static_cast<float>(j1.Pt()/jj.M()), static_cast<float>(j1.Eta()), static_cast<float>(j1.Phi()), static_cast<float>(j1.E()/jj.M()), static_cast<float>(PCBT_j1), 0.};
        std::vector<float> node2 = {static_cast<float>(j2.Pt()/jj.M()), static_cast<float>(j2.Eta()), static_cast<float>(j2.Phi()), static_cast<float>(j2.E()/jj.M()), static_cast<float>(PCBT_j2), 0.};

         for(size_t k = 0; k < std::min(jets->size(), (std::size_t)6); k++) {
          if(k==i || k==j) continue;
          const xAOD::Jet *jet3 = jets->at(k);

          for(size_t p = k+1; p < std::min(jets->size(), (std::size_t)6); p++) {
            if(p==i || p==j) continue;
            const xAOD::Jet *jet4 = jets->at(p);

            TLorentzVector j3 = jet3->p4();
            TLorentzVector j4 = jet4->p4();

            std::vector<Ort::Value> input_tensors;
            auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

            TLorentzVector vbf_jj = j3 + j4;
           int PCBT_j3 = m_PCBT.get(*jet3, sys);
            int PCBT_j4 = m_PCBT.get(*jet4, sys);

            std::vector<float> nodes;
            std::vector<float> edges;
            std::vector<float> globals;
            std::vector<long int> senders;
            std::vector<long int> receivers;
            std::vector<long int> n_node = {5};
            std::vector<long int> n_edge = {20};

            std::vector<float> node4 = {static_cast<float>(j3.Pt()/vbf_jj.M()), static_cast<float>(j3.Eta()), static_cast<float>(j3.Phi()), static_cast<float>(j3.E()/vbf_jj.M()), static_cast<float>(PCBT_j3), 2.};
            std::vector<float> node5 = {static_cast<float>(j4.Pt()/vbf_jj.M()), static_cast<float>(j4.Eta()), static_cast<float>(j4.Phi()), static_cast<float>(j4.E()/vbf_jj.M()), static_cast<float>(PCBT_j4), 2.};
            nodes.insert(nodes.end(), node1.begin(), node1.end());
            nodes.insert(nodes.end(), node2.begin(), node2.end());
            nodes.insert(nodes.end(), node3.begin(), node3.end());
            nodes.insert(nodes.end(), node4.begin(), node4.end());
            nodes.insert(nodes.end(), node5.begin(), node5.end());
            for(int m = 0; m < 5; m++)
            {
              for(int n = 0; n < 5; n++)
              {
                if(m == n) continue;
                senders.push_back(m);
                receivers.push_back(n);
                float dEta = nodes[m*node1.size()+1] - nodes[n*node1.size()+1];
                float dPhi = nodes[m*node1.size()+2] - nodes[n*node1.size()+2];
                if(dPhi > M_PI) dPhi -= 2* M_PI;
                if(dPhi < -M_PI) dPhi += 2* M_PI;
                float dR = std::sqrt(dEta*dEta + dPhi*dPhi);
                std::vector<float> edge = {dEta, dPhi, dR};
                edges.insert(edges.end(), edge.begin(), edge.end());
              }
            }

            globals.push_back(pile_up);
            globals.push_back(float(vbf_jj.M()));

            input_tensors.push_back(Ort::Value::CreateTensor<float>(memory_info, nodes.data(), m_input_node_dims_sum.at(HHBBYY::GNN::VBFTarget)[0], m_input_node_dims_vector.at(HHBBYY::GNN::VBFTarget)[0].data(), m_input_node_dims_vector.at(HHBBYY::GNN::VBFTarget)[0].size()));
            input_tensors.push_back(Ort::Value::CreateTensor<float>(memory_info, edges.data(), m_input_node_dims_sum.at(HHBBYY::GNN::VBFTarget)[1], m_input_node_dims_vector.at(HHBBYY::GNN::VBFTarget)[1].data(), m_input_node_dims_vector.at(HHBBYY::GNN::VBFTarget)[1].size()));
            input_tensors.push_back(Ort::Value::CreateTensor<float>(memory_info, globals.data(), m_input_node_dims_sum.at(HHBBYY::GNN::VBFTarget)[2], m_input_node_dims_vector.at(HHBBYY::GNN::VBFTarget)[2].data(), m_input_node_dims_vector.at(HHBBYY::GNN::VBFTarget)[2].size()));
            input_tensors.push_back(Ort::Value::CreateTensor<long int>(memory_info, receivers.data(), m_input_node_dims_sum.at(HHBBYY::GNN::VBFTarget)[3], m_input_node_dims_vector.at(HHBBYY::GNN::VBFTarget)[3].data(), m_input_node_dims_vector.at(HHBBYY::GNN::VBFTarget)[3].size()));
            input_tensors.push_back(Ort::Value::CreateTensor<long int>(memory_info, senders.data(), m_input_node_dims_sum.at(HHBBYY::GNN::VBFTarget)[4], m_input_node_dims_vector.at(HHBBYY::GNN::VBFTarget)[4].data(), m_input_node_dims_vector.at(HHBBYY::GNN::VBFTarget)[4].size()));
            input_tensors.push_back(Ort::Value::CreateTensor<long int>(memory_info, n_node.data(), m_input_node_dims_sum.at(HHBBYY::GNN::VBFTarget)[5], m_input_node_dims_vector.at(HHBBYY::GNN::VBFTarget)[5].data(), m_input_node_dims_vector.at(HHBBYY::GNN::VBFTarget)[5].size()));
            input_tensors.push_back(Ort::Value::CreateTensor<long int>(memory_info, n_edge.data(), m_input_node_dims_sum.at(HHBBYY::GNN::VBFTarget)[6], m_input_node_dims_vector.at(HHBBYY::GNN::VBFTarget)[6].data(), m_input_node_dims_vector.at(HHBBYY::GNN::VBFTarget)[6].size()));

            std::vector<Ort::Value> output_tensors = session.Run(Ort::RunOptions{nullptr}, input_node_names.data(), input_tensors.data(), input_node_names.size(), output_node_names.data(), output_node_names.size());
            float score = *(output_tensors[0].GetTensorMutableData<float>());

            if(score > max_score) {
              max_score = score;
              GNN_jets = {jet1, jet2, jet3, jet4};
            }
          }
        }
      }
    }

    return GNN_jets;
  }


  float BaselineVarsbbyyAlg::compute_Topness(const xAOD::JetContainer *jets){
    float minTopness=std::numeric_limits<float>::max();
    const float wmass=80 * Athena::Units::GeV;
    const float topmass=173 * Athena::Units::GeV;
    std::vector< TLorentzVector > temp_jets;
    for (unsigned int i = 0; i < jets->size(); i++) {
        temp_jets.push_back(jets->at(i)->p4());
    }
    // If there are < 3 jets (min. # required to define ChiWt) fill out the rest with 0, 0, 0, 0 dummy jets
    if (jets->size() < 3) {
        for (unsigned int i = 0; i < 3 - jets->size(); i++) {
            temp_jets.emplace_back(0, 0, 0, 0);
        }
    }

    for(unsigned int j1=0;j1<temp_jets.size();j1++){ // W->j1j2, bjet=j3
      for(unsigned int j2=j1+1;j2<temp_jets.size();j2++){
        for(unsigned int j3=0;j3<temp_jets.size();j3++){
          // compute m_j1j2 and m_j1j2j3
          if(j3==j1 || j3==j2)
              continue;
          float m_j1j2=(temp_jets.at(j1)+temp_jets.at(j2)).M();
          float m_j1j2j3=(temp_jets.at(j1)+temp_jets.at(j2)+temp_jets.at(j3)).M();
          // find minimum topness
          float topness=std::hypot((m_j1j2-wmass)/wmass, (m_j1j2j3-topmass)/topmass);
          minTopness = std::min(topness, minTopness);
        }
      }
    }
    return minTopness;
  }

  /*
  eventShapes[0] = sphericityT;
  eventShapes[1] = planarFlow;
  */
  std::vector<float> BaselineVarsbbyyAlg::compute_EventShapes(const xAOD::Jet *Hbb_Jet1, const xAOD::Jet *Hbb_Jet2,
                                                 const std::vector<TLorentzVector>& photons, const int& n_photons){
    std::vector<float> eventShapes(2);
    if (Hbb_Jet1 && Hbb_Jet2 && n_photons >= 2) {
      TLorentzVector bjet1 = Hbb_Jet1->p4();
      TLorentzVector bjet2 = Hbb_Jet2->p4();
      std::vector<TLorentzVector> p4_vec = {photons[0], photons[1], bjet1, bjet2};

      TMatrixDSym MomentumTensor = TMatrixDSym(3);
      TMatrixDSym MomentumTensorT = TMatrixDSym(3);

      double Sxx = 0.0, Sxy = 0.0, Sxz = 0.0, Syy = 0.0, Syz = 0.0, Szz = 0.0, normal = 0.0;
      for(const auto& p4 : p4_vec){
        Sxx += p4.Px()*p4.Px();
        Sxy += p4.Px()*p4.Py();
        Sxz += p4.Px()*p4.Pz();
        Syy += p4.Py()*p4.Py();
        Syz += p4.Py()*p4.Pz();
        Szz += p4.Pz()*p4.Pz();
        normal += p4.P()*p4.P();
      }

      MomentumTensor[0][0] = Sxx / normal;
      MomentumTensor[0][1] = Sxy / normal;
      MomentumTensor[0][2] = Sxz / normal;
      MomentumTensor[1][0] = MomentumTensor[0][1];
      MomentumTensor[1][1] = Syy / normal;
      MomentumTensor[1][2] = Syz / normal;
      MomentumTensor[2][0] = MomentumTensor[0][2];
      MomentumTensor[2][1] = MomentumTensor[1][2];
      MomentumTensor[2][2] = Szz / normal;

      MomentumTensorT[0][0] = MomentumTensor[0][0];
      MomentumTensorT[0][1] = MomentumTensor[0][1];
      MomentumTensorT[1][1] = MomentumTensor[1][1];
      MomentumTensorT[1][0] = MomentumTensor[1][0];

      TMatrixDSymEigen EigenValues = TMatrixDSymEigen(MomentumTensor);
      TMatrixDSymEigen EigenValuesT = TMatrixDSymEigen(MomentumTensorT);

      const TVectorD& eigenVec = EigenValues.GetEigenValues();
      const TVectorD& eigenVecT = EigenValuesT.GetEigenValues();

      float sphericityT = 2.0 * eigenVecT[1] / (eigenVecT[0] + eigenVecT[1]);
      float planarFlow = -99;

      if ((eigenVec[0] + eigenVec [1]) != 0) {
        planarFlow = 4.0 * eigenVec[0] * eigenVec[1] / std::pow(eigenVec[0] + eigenVec [1], 2);
      }

      eventShapes[0] = sphericityT;
      eventShapes[1] = planarFlow;
    }

    return eventShapes;
  }

  float BaselineVarsbbyyAlg::compute_pTBalance(const xAOD::Jet *Hbb_Jet1, const xAOD::Jet *Hbb_Jet2,
                                            const std::vector<TLorentzVector>& photons,
                                            const int& n_photons){
    float pTBalance = -99;
    if (Hbb_Jet1 && Hbb_Jet2 && n_photons >= 2) {
      TLorentzVector bjet1 = Hbb_Jet1->p4();
      TLorentzVector bjet2 = Hbb_Jet2->p4();
      float numerator, denominator;
      std::vector<TLorentzVector> p4_vec = {photons[0], photons[1], bjet1, bjet2};
      TLorentzVector numerator_p4(0.,0.,0.,0.);
      denominator = 0;
      for(const auto& p4 : p4_vec){
          numerator_p4 += p4;
          denominator += p4.Pt();
      }
      numerator = numerator_p4.Pt();
      if (denominator != 0)
      {
        pTBalance = numerator / denominator;
      }
    }
    return pTBalance;
  }

  void BaselineVarsbbyyAlg::loadBDT(const std::string &filePath, std::unique_ptr<MVAUtils::BDT> &bdt) {
    std::string resolvedPath = PathResolverFindCalibFile(filePath);
    TFile *f = TFile::Open(resolvedPath.c_str());
    if (!f || f->IsZombie()) {
      ATH_MSG_ERROR("Cannot open file \"" << filePath << "\" or the file is in a bad state.");
      return;
    }

    TTree *tree = nullptr;
    f->GetObject("xgboost", tree);
    bdt = std::make_unique<MVAUtils::BDT>(tree);

    f->Close();
  }

  std::vector<float> BaselineVarsbbyyAlg::makeXGBoostDMatrixLegacyNonres(const TLorentzVector& ph1, const TLorentzVector& ph2,
                                                                         ConstDataVector<xAOD::JetContainer> &categorisation_jets,
                                                                         const xAOD::MissingETContainer *met, const auto &sys,
                                                                         const std::map<HHBBYY::Var, float> &eventFloats, bool isKFvariables, bool isGNNvariables) {
    // Assuming that the jets are already sorted by btagging score and then pT (jetSelectorAlg)
    // initialize the variables
    std::vector<float> vars(HHBBYY::Var::NVars, 0.0f);
    vars[HHBBYY::Var::j3_pt] = -99;
    vars[HHBBYY::Var::j3_eta] = -99;
    vars[HHBBYY::Var::j3_phi] = -99;
    vars[HHBBYY::Var::j3_pcbt] = -99;
    vars[HHBBYY::Var::j4_pt] = -99;
    vars[HHBBYY::Var::j4_eta] = -99;
    vars[HHBBYY::Var::j4_phi] = -99;
    vars[HHBBYY::Var::j4_pcbt] = -99;


    // Sort jets by pcbt and then by pT if same pcbt
    TLorentzVector H_bb = categorisation_jets.at(0)->p4() + categorisation_jets.at(1)->p4();

    // photons
    float myy = (ph1 + ph2).M();
    vars[HHBBYY::Var::y1_ptOverMyy] = ph1.Pt() / myy;
    vars[HHBBYY::Var::y1_eta] = ph1.Eta();
    vars[HHBBYY::Var::y1_phi] = ph1.Phi() ;
    vars[HHBBYY::Var::y2_ptOverMyy] = ph2.Pt() / myy;
    vars[HHBBYY::Var::y2_eta] = ph2.Eta();
    vars[HHBBYY::Var::y2_phi] = ph2.Phi();
    // met
    vars[HHBBYY::Var::met] = (*met)["Final"]->met();
    vars[HHBBYY::Var::met_phi] = (*met)["Final"]->phi();


    // jets
    int jet_ith = 0;
    for (const xAOD::Jet *jet : categorisation_jets) {

      // {jet_pt, jet_eta, jet_phi, jet_pseudo_score}
      if (jet_ith <= 1) {
        vars[HHBBYY::Var::j1_pt + (jet_ith * 4)] = jet->pt();
        vars[HHBBYY::Var::j1_eta + (jet_ith * 4)] = jet->eta();
        vars[HHBBYY::Var::j1_phi + (jet_ith * 4)] = jet->phi();
        vars[HHBBYY::Var::j1_pcbt + (jet_ith * 4)] = m_PCBT.get(*jet, sys);

      } else {
        // Input variables for j2 and j3 are not contiguous, so cannot be included in previous if
        vars[HHBBYY::Var::j3_pt + ((jet_ith - 2) * 4)] = jet->pt();
        vars[HHBBYY::Var::j3_eta + ((jet_ith - 2) * 4)] = jet->eta();
        vars[HHBBYY::Var::j3_phi + ((jet_ith - 2) * 4)] = jet->phi();
        vars[HHBBYY::Var::j3_pcbt + ((jet_ith - 2) * 4)] = m_PCBT.get(*jet, sys);
      }
      jet_ith++;
    }

    // jj
    vars[HHBBYY::Var::bb_pt] = H_bb.Pt();
    vars[HHBBYY::Var::bb_eta] = H_bb.Eta();
    vars[HHBBYY::Var::bb_phi] = H_bb.Phi();
    vars[HHBBYY::Var::bb_m] = (isKFvariables) ? eventFloats.at(HHBBYY::Var::bb_m_KF_unconstrained) : H_bb.M(); // for KF we need to use the version of bb_m from the second iteration

    vars[HHBBYY::Var::bb_dR] = categorisation_jets.at(0)->p4().DeltaR(categorisation_jets.at(1)->p4());
    vars[HHBBYY::Var::yy_dR] = ph1.DeltaR(ph2);

    vars[HHBBYY::Var::jets_HT] = (isKFvariables) ? eventFloats.at(HHBBYY::Var::jets_HT_KF) : eventFloats.at(HHBBYY::Var::jets_HT);
      // ChiWt variable
    vars[HHBBYY::Var::topness] = (isKFvariables) ? eventFloats.at(HHBBYY::Var::topness_KF) : eventFloats.at(HHBBYY::Var::topness);

    // event level jet variables
    if (isKFvariables && !isGNNvariables){
      if(categorisation_jets.size()==4){
        //Use non-KF variables for VBF to get closure with local training for now
        vars[HHBBYY::Var::vbfjj_dEta] = eventFloats.at(HHBBYY::Var::vbfjj_dEta_KF);
        vars[HHBBYY::Var::vbfjj_m] = eventFloats.at(HHBBYY::Var::vbfjj_m_KF);
      }else{
        vars[HHBBYY::Var::vbfjj_dEta] = 0; //-999; //another arbitrary convention...
        vars[HHBBYY::Var::vbfjj_m] = 0; //-999;
      }
      vars[HHBBYY::Var::bbyy_mStar] =  eventFloats.at(HHBBYY::Var::bbyy_mStar_KF);
      vars[HHBBYY::Var::sphericityT] = eventFloats.at(HHBBYY::Var::sphericityT_KF);
      vars[HHBBYY::Var::planarFlow] = eventFloats.at(HHBBYY::Var::planarFlow_KF);
    } else if (!isKFvariables && !isGNNvariables){
      if(categorisation_jets.size()==4){
        vars[HHBBYY::Var::vbfjj_dEta] = eventFloats.at(HHBBYY::Var::vbfjj_dEta);
        vars[HHBBYY::Var::vbfjj_m] = eventFloats.at(HHBBYY::Var::vbfjj_m);
      }else{
        vars[HHBBYY::Var::vbfjj_dEta] = 0; //-999;
        vars[HHBBYY::Var::vbfjj_m] = 0; //-999;
      }

      vars[HHBBYY::Var::bbyy_mStar] =  eventFloats.at(HHBBYY::Var::bbyy_mStar);
      vars[HHBBYY::Var::sphericityT] = eventFloats.at(HHBBYY::Var::sphericityT);
      vars[HHBBYY::Var::planarFlow] = eventFloats.at(HHBBYY::Var::planarFlow);
    } else if (!isKFvariables && isGNNvariables){
      if(categorisation_jets.size()==4){
        vars[HHBBYY::Var::vbfjj_dEta] = eventFloats.at(HHBBYY::Var::vbfjj_dEta_GNN);
        vars[HHBBYY::Var::vbfjj_m] = eventFloats.at(HHBBYY::Var::vbfjj_m_GNN);
      }else{
        vars[HHBBYY::Var::vbfjj_dEta] = -999;
        vars[HHBBYY::Var::vbfjj_m] = -999;
      }
      vars[HHBBYY::Var::bbyy_mStar] =  eventFloats.at(HHBBYY::Var::bbyy_mStar_GNN);
      vars[HHBBYY::Var::sphericityT] = eventFloats.at(HHBBYY::Var::sphericityT_GNN);
      vars[HHBBYY::Var::planarFlow] = eventFloats.at(HHBBYY::Var::planarFlow_GNN);
    }

    const float bbyy_sumPt = ph1.Pt() + ph2.Pt() + categorisation_jets.at(0)->pt() + categorisation_jets.at(1)->pt();
    TLorentzVector HH = ph1 + ph2 + categorisation_jets.at(0)->p4() + categorisation_jets.at(1)->p4();
    vars[HHBBYY::Var::bbyy_ptOverSumPt] = HH.Pt() / bbyy_sumPt;

    return vars;
  }

  StatusCode BaselineVarsbbyyAlg::vbf_calculations(const TLorentzVector& ph1, const TLorentzVector& ph2,
                           const int& n_photons,
						   const xAOD::Jet *Hbb_Jet1, const xAOD::Jet *Hbb_Jet2, const xAOD::JetContainer *jets,
						   double HT, const TLorentzVector& HH,
						   const std::string& prefix_j, const std::string& prefix_jj,
						   std::map<HHBBYY::Var, float> &eventFloats, const xAOD::EventInfo *event, const auto &sys) {
    std::array<TLorentzVector, 2> vbf_j;
    TLorentzVector vbf_jj(0., 0., 0., 0.);
    TLorentzVector yybbjj(0., 0., 0., 0.);
    float vbf_jj_maxscore = 0;

    if (m_do_nonresonant_BDTs || m_save_VBF_vars) {
      if (n_photons >= 2 && Hbb_Jet1 && Hbb_Jet2) {
	if (m_vbfjets_method == HHBBYY::VBFjetsMethod::BDT) {
	  vbf_jj_maxscore = getVBFjets_BDT(HT, ph1, ph2, Hbb_Jet1, Hbb_Jet2, jets, vbf_j);
	} else if (m_vbfjets_method == HHBBYY::VBFjetsMethod::mjj) {
	  getVBFjets_mjj(Hbb_Jet1, Hbb_Jet2, jets, vbf_j);
	} else if (m_vbfjets_method == HHBBYY::VBFjetsMethod::pTsorting) {
	  getVBFjets_pTsorting(Hbb_Jet1, Hbb_Jet2, jets, vbf_j);
	} else if (m_vbfjets_method == HHBBYY::VBFjetsMethod::invalid) {
	  ANA_MSG_ERROR("Invalid vbfjets method imported!!! The default BDT method is called!!!");
	  return StatusCode::FAILURE;
	}

	vbf_jj = vbf_j[0] + vbf_j[1];
	yybbjj = vbf_jj + HH;
	eventFloats.at((prefix_jj == "KF_Jet_vbf_jj") ? HHBBYY::Var::vbfjj_m_KF : HHBBYY::Var::vbfjj_m) = vbf_jj.M();
	eventFloats.at((prefix_jj == "KF_Jet_vbf_jj") ? HHBBYY::Var::vbfjj_dEta_KF : HHBBYY::Var::vbfjj_dEta) = std::fabs(vbf_j[0].Eta() - vbf_j[1].Eta());
      }
    }

    if (m_save_VBF_vars) {
      for (unsigned int i = 0; i < 2; i++) {
        std::string full_prefix = prefix_j + std::to_string(i + 1);
        m_Fbranches.at(full_prefix + "_pt").set(*event, vbf_j[i].Pt(), sys);
        m_Fbranches.at(full_prefix + "_eta").set(*event, vbf_j[i].Eta(), sys);
        m_Fbranches.at(full_prefix + "_phi").set(*event, vbf_j[i].Phi(), sys);
        m_Fbranches.at(full_prefix + "_E").set(*event, vbf_j[i].E(), sys);
      }

      if (m_vbfjets_method == HHBBYY::VBFjetsMethod::BDT) m_Fbranches.at(prefix_jj + "_maxscore").set(*event, vbf_jj_maxscore, sys);
      m_Fbranches.at(prefix_jj + "_m").set(*event, vbf_jj.M(), sys);
      m_Fbranches.at(prefix_jj + "_deta").set(*event, std::fabs(vbf_j[0].Eta() - vbf_j[1].Eta()), sys);
    }
    return StatusCode::SUCCESS;
  }

  //#######################################################################################################################################################################################################
  // Convert string to enum (VBF Jets selection method)
  VBFjetsMethod BaselineVarsbbyyAlg::stringToVBFjetsMethod(const std::string& vbfjets_method_str) {
    if (vbfjets_method_str == "BDT") {
      return HHBBYY::VBFjetsMethod::BDT;
    } else if (vbfjets_method_str == "mjj") {
      return HHBBYY::VBFjetsMethod::mjj;
    } else if (vbfjets_method_str == "pTsorting") {
      return HHBBYY::VBFjetsMethod::pTsorting;
    } else {
      ANA_MSG_ERROR("Invalid vbfjets method string");
      return HHBBYY::VBFjetsMethod::invalid;
    }
  }

  // Selects VBF jets based on VBF jet BDT score
  float BaselineVarsbbyyAlg::getVBFjets_BDT(float ht, const TLorentzVector& ph1, const TLorentzVector& ph2,
                                            const xAOD::Jet *Hbb_Jet1, const xAOD::Jet *Hbb_Jet2,
                                            const xAOD::JetContainer *jets, std::array<TLorentzVector, 2>& Jets_vbf) {

    std::vector<float> vars(HHBBYY::VBFVars::nVars, 0.0f);
    vars[HHBBYY::VBFVars::HT] = ht;

    if (m_bdts.size()!=3){
      ANA_MSG_ERROR("3 BDTs are required: 1 for vbf jets selection");
      return 0;
    }

    int idx1 = -1;
    int idx2 = -1;
    float max_jjscore = 0;
    TLorentzVector yybb(0.,0.,0.,0.);

    const int nCandidateVBFJets = jets->size() - 2;
    if (nCandidateVBFJets >= 2) {

      // candidate photons
      TLorentzVector yy = ph1 + ph2;
      // candidate bjets
      TLorentzVector b1 = Hbb_Jet1->p4();
      TLorentzVector b2 = Hbb_Jet2->p4();
      TLorentzVector bb = b1 + b2;
      // yybb system
      yybb = yy + bb;

      const int nComb = (jets->size() * (jets->size() - 1))/2;
      std::vector<int> allVBFJetLeadIdx(nComb);
      std::vector<int> allVBFJetSubIdx(nComb);
      std::vector<float> scores;

      int curComb = 0;

      for (size_t i = 0; i < jets->size()-1; i++) {
        // ignore candidate bjets
        if (jets->at(i)==Hbb_Jet1 || jets->at(i)==Hbb_Jet2) {
          continue;
        }

        for (size_t j = i + 1; j < jets->size(); j++) {
          // ignore candidate bjets
          if (jets->at(j)==Hbb_Jet1 || jets->at(j)==Hbb_Jet2) {
            continue;
          }

          int candidateVBFJetLeadIdx = -1;
          int candidateVBFJetSubIdx = -1;

          if (jets->at(i)->pt() >= jets->at(j)->pt()) {
            candidateVBFJetLeadIdx = i;
            candidateVBFJetSubIdx = j;
          } else {
            candidateVBFJetLeadIdx = j;
            candidateVBFJetSubIdx = i;
          }

          const TLorentzVector candidateVBFJetLead = jets->at(candidateVBFJetLeadIdx)->p4();
          const TLorentzVector candidateVBFJetSub = jets->at(candidateVBFJetSubIdx)->p4();

          TLorentzVector candidateVBFJets = candidateVBFJetLead + candidateVBFJetSub;
          vars[HHBBYY::VBFVars::vbf_jj_m] = candidateVBFJets.M();
          vars[HHBBYY::VBFVars::vbf_jj_deta] = fabs(candidateVBFJetLead.Eta() - candidateVBFJetSub.Eta());
          vars[HHBBYY::VBFVars::dR_yybb_vbfj1] = candidateVBFJetLead.DeltaR(yybb);
          vars[HHBBYY::VBFVars::dR_yybb_vbfj2] = candidateVBFJetSub.DeltaR(yybb);
          vars[HHBBYY::VBFVars::deta_yybb_vbfj1] = fabs(candidateVBFJetLead.Eta() - yybb.Eta());
          vars[HHBBYY::VBFVars::deta_yybb_vbfj2] = fabs(candidateVBFJetSub.Eta() - yybb.Eta());
          vars[HHBBYY::VBFVars::dR_yybb_jj] = candidateVBFJets.DeltaR(yybb);
          vars[HHBBYY::VBFVars::deta_yybb_jj] = fabs(candidateVBFJets.Eta() - yybb.Eta());

          TLorentzVector candidate_yybbjj = yybb + candidateVBFJets;
          vars[HHBBYY::VBFVars::pT_yybbjj] = candidate_yybbjj.Pt();
          vars[HHBBYY::VBFVars::eta_yybbjj] = candidate_yybbjj.Eta();
          vars[HHBBYY::VBFVars::m_yybbjj] = candidate_yybbjj.M();
          vars[HHBBYY::VBFVars::vbf_j1_pt] = candidateVBFJetLead.Pt();
          vars[HHBBYY::VBFVars::vbf_j1_eta] = candidateVBFJetLead.Eta();
          vars[HHBBYY::VBFVars::vbf_j2_pt] = candidateVBFJetSub.Pt();
          vars[HHBBYY::VBFVars::vbf_j2_eta] = candidateVBFJetSub.Eta();

          allVBFJetLeadIdx[curComb] = candidateVBFJetLeadIdx;
          allVBFJetSubIdx[curComb] = candidateVBFJetSubIdx;
          curComb++;

          float Score = m_bdts.at(HHBBYY::BDT::VBFjets)->GetClassification(vars);
          scores.push_back(Score);
        } // end loop second jet
      } // end loop first jet

      auto maxScoreIdx = std::distance(scores.begin(), std::max_element(scores.begin(), scores.end()));
      max_jjscore = scores[maxScoreIdx];
      idx1 = allVBFJetLeadIdx[maxScoreIdx];
      idx2 = allVBFJetSubIdx[maxScoreIdx];

      Jets_vbf[0] = (*jets)[idx1]->p4();
      Jets_vbf[1] = (*jets)[idx2]->p4();
    }  // end valid yybb system

    return max_jjscore;
  }

  // Selects VBF jets based on VBF jets: maximum mjj
  void BaselineVarsbbyyAlg::getVBFjets_mjj(const xAOD::Jet *Hbb_Jet1, const xAOD::Jet *Hbb_Jet2,
                                           const xAOD::JetContainer *jets, std::array<TLorentzVector, 2>& Jets_vbf) {
    float m_jj = -999;

    const int nCandidateVBFJets = jets->size() - 2;
    if (nCandidateVBFJets >= 2) {
      for (size_t i = 0; i < jets->size()-1; i++) {
        // ignore candidate bjets
        if (jets->at(i)==Hbb_Jet1 || jets->at(i)==Hbb_Jet2) {
          continue;
        }

        for (size_t j = i + 1; j < jets->size(); j++) {
          // ignore candidate bjets
          if (jets->at(j)==Hbb_Jet1 || jets->at(j)==Hbb_Jet2) {
            continue;
          }

          TLorentzVector iPair = jets->at(i)->p4() + jets->at(j)->p4();
          if (iPair.M() > m_jj) {
            m_jj = iPair.M();
            Jets_vbf[0] = jets->at(i)->p4();
            Jets_vbf[1] = jets->at(j)->p4();
          }
        } // end loop second jet
      } // end loop first jet
    }
  }

  // Selects VBF jets based on VBF jets: pT sorting
  void BaselineVarsbbyyAlg::getVBFjets_pTsorting(const xAOD::Jet *Hbb_Jet1, const xAOD::Jet *Hbb_Jet2,
                                                 const xAOD::JetContainer *jets, std::array<TLorentzVector, 2>& Jets_vbf) {

    int nCandidateVBFJets = 0;
    const xAOD::Jet* VBF_j1 = nullptr;
    const xAOD::Jet* VBF_j2 = nullptr;

    // select the leading and sub-leading VBF jets
    for (const xAOD::Jet *jet : *jets) {
      // ignore candidate bjets
      if (jet==Hbb_Jet1 || jet==Hbb_Jet2) continue;

      if (!VBF_j1 || jet->pt() > VBF_j1->pt()){
        VBF_j2 = VBF_j1;
        VBF_j1 = jet;
        ++nCandidateVBFJets;
      }else if (!VBF_j2 || jet->pt() > VBF_j2->pt()){
        VBF_j2 = jet;
        ++nCandidateVBFJets;
      }
    }

    if (nCandidateVBFJets >= 2) {
      Jets_vbf[0] = VBF_j1->p4();
      Jets_vbf[1] = VBF_j2->p4();
    }
  }

  // Low and High mass regions (categorization)
  void BaselineVarsbbyyAlg::performCategorisationBDT(const TLorentzVector& ph1, const TLorentzVector& ph2,
                                                     const xAOD::Jet *Hbb_Jet1,
                                                     const xAOD::Jet *Hbb_Jet2,
                                                     const xAOD::JetContainer *jets,
                                                     const xAOD::MissingETContainer *met, const auto &sys,
                                                     std::map<HHBBYY::Var, float> &eventFloats,
                                                     std::map<HHBBYY::Var, int> &eventInts, bool isKFvariables, bool isGNNvariables, bool isCorrelatedModel, bool is2024Model, int year) {

    if (m_bdts.size()!=17){
      ANA_MSG_ERROR("17 BDTs are required");
      return;
    }

    if (isKFvariables && isGNNvariables){
      ANA_MSG_ERROR("There is no BDT models trained using the GNN event selection and the KF");
      return;
    }


    int XGBoostCat = -99;
    float XGBoostScore = -99.;

    // load data array for xgboost model prediction
    ConstDataVector<xAOD::JetContainer> cat_jets = categorisation_jets(Hbb_Jet1, Hbb_Jet2, jets);
    std::vector<float> vars = makeXGBoostDMatrixLegacyNonres(ph1, ph2,  cat_jets, met, sys, eventFloats, isKFvariables, isGNNvariables);

    // High mass channel Selection
    if (!isKFvariables){
      if (eventFloats.at(isGNNvariables ? HHBBYY::Var::bbyy_mStar_GNN : HHBBYY::Var::bbyy_mStar) >= 350 * Athena::Units::GeV) {
        // get BDT score
        float score = -99;
        if (year<2019) {
          score = m_bdts.at(HHBBYY::BDT::HH2025_high_mass_Run2)->GetClassification(vars);

          if (score >= 0.925) {
            XGBoostCat = 3;
          } else if (score >= 0.865) {
            XGBoostCat = 2;
          } else if (score >= 0.67) {
            XGBoostCat = 1;
          } else {
            XGBoostCat = 0;
          }
          XGBoostScore = score;
        } else {
          score = m_bdts.at(HHBBYY::BDT::HH2025_high_mass_Run3)->GetClassification(vars);

          if (score >= 0.870) {
            XGBoostCat = 2003;
          } else if (score >= 0.735) {
            XGBoostCat = 2002;
          } else if (score >= 0.450) {
            XGBoostCat = 2001;
          } else {
            XGBoostCat = 2000;
          }
          XGBoostScore = score;
        }
      } else if (eventFloats.at(isGNNvariables ? HHBBYY::Var::bbyy_mStar_GNN : HHBBYY::Var::bbyy_mStar) < 350 * Athena::Units::GeV) {
        float score = -99;
        if (year<2019) {
          // get BDT score
          score = m_bdts.at(HHBBYY::BDT::HH2025_low_mass_Run2)->GetClassification(vars);

          if (score >= 0.970) {
            XGBoostCat = 1004;
          } else if (score >= 0.940) {
            XGBoostCat = 1003;
          } else if (score >= 0.880) {
            XGBoostCat = 1002;
          } else if (score >= 0.545) {
            XGBoostCat = 1001;
          } else {
            XGBoostCat = 1000;
          }
          XGBoostScore = score;
        } else{
          // get BDT score
          score = m_bdts.at(HHBBYY::BDT::HH2025_low_mass_Run3)->GetClassification(vars);

          if (score >= 0.925) {
            XGBoostCat = 3004;
          } else if (score >= 0.865) {
            XGBoostCat = 3003;
          } else if (score >= 0.750) {
            XGBoostCat = 3002;
          } else if (score >= 0.465) {
            XGBoostCat = 3001;
          } else {
            XGBoostCat = 3000;
          }
          XGBoostScore = score;
        }

      }
      eventInts.at(isGNNvariables ? HHBBYY::Var::bdt_sel_category_GNN : HHBBYY::Var::bdt_sel_category) = XGBoostCat;
      eventFloats.at(isGNNvariables ? HHBBYY::Var::bdt_sel_score_GNN : HHBBYY::Var::bdt_sel_score) = XGBoostScore;
    } else { //KF models
      if (!isCorrelatedModel) { //uncorrelated model
        if (!is2024Model) { //uncorrelated model without 2024
          if (eventFloats.at(HHBBYY::Var::bbyy_mStar_KF) >= 350 * Athena::Units::GeV) {
            float score = -99;
            if (year<2019) {
              // get BDT score
              score = m_bdts.at(HHBBYY::BDT::HH2025_KF_high_mass_Run2)->GetClassification(vars);

              if (score >= 0.935) {
                XGBoostCat = 3;
              } else if (score >= 0.855) {
                XGBoostCat = 2;
              } else if (score >= 0.595) {
                XGBoostCat = 1;
              } else {
                XGBoostCat = 0;
              }

              XGBoostScore = score;
            } else {
              score = m_bdts.at(HHBBYY::BDT::HH2025_KF_high_mass_Run3)->GetClassification(vars);

              if (score >= 0.880) {
                XGBoostCat = 2003;
              } else if (score >= 0.745) {
                XGBoostCat = 2002;
              } else if (score >= 0.400) {
                XGBoostCat = 2001;
              } else {
                XGBoostCat = 2000;
              }

              XGBoostScore = score;
            }
          } else {
            float score = -99;
            if (year<2019) {
              // get BDT score
              score = m_bdts.at(HHBBYY::BDT::HH2025_KF_low_mass_Run2)->GetClassification(vars);

              if (score >= 0.975) {
                XGBoostCat = 1004;
              } else if (score >= 0.945) {
                XGBoostCat = 1003;
              } else if (score >= 0.825) {
                XGBoostCat = 1002;
              } else if (score >= 0.520) {
                XGBoostCat = 1001;
              } else {
                XGBoostCat = 1000;
              }

              XGBoostScore = score;
            } else{
              score = m_bdts.at(HHBBYY::BDT::HH2025_KF_low_mass_Run3)->GetClassification(vars);

              if (score >= 0.920) {
                XGBoostCat = 3004;
              } else if (score >= 0.855) {
                XGBoostCat = 3003;
              } else if (score >= 0.670) {
                XGBoostCat = 3002;
              } else if (score >= 0.370) {
                XGBoostCat = 3001;
              } else {
                XGBoostCat = 3000;
              }

              XGBoostScore = score;
            }

          }
          eventInts.at(HHBBYY::Var::bdt_sel_category_KF) = XGBoostCat;
          eventFloats.at(HHBBYY::Var::bdt_sel_score_KF) = XGBoostScore;
        } else { //uncorrelated model with 2024
          if (eventFloats.at(HHBBYY::Var::bbyy_mStar_KF) >= 350 * Athena::Units::GeV) {
            float score = -99;
            if (year<2019) {
              // get BDT score
              score = m_bdts.at(HHBBYY::BDT::HH2025_KF_high_mass_Run2)->GetClassification(vars);

              if (score >= 0.935) {
                XGBoostCat = 3;
              } else if (score >= 0.855) {
                XGBoostCat = 2;
              } else if (score >= 0.595) {
                XGBoostCat = 1;
              } else {
                XGBoostCat = 0;
              }

              XGBoostScore = score;
            } else {
              score = m_bdts.at(HHBBYY::BDT::HH2025_KF_high_mass_Run3_with2024)->GetClassification(vars);

              if (score >= 0.945) {
                XGBoostCat = 2003;
              } else if (score >= 0.865) {
                XGBoostCat = 2002;
              } else if (score >= 0.595) {
                XGBoostCat = 2001;
              } else {
                XGBoostCat = 2000;
              }

              XGBoostScore = score;
            }
          } else {
            float score = -99;
            if (year<2019) {
              // get BDT score
              score = m_bdts.at(HHBBYY::BDT::HH2025_KF_low_mass_Run2)->GetClassification(vars);

              if (score >= 0.975) {
                XGBoostCat = 1004;
              } else if (score >= 0.945) {
                XGBoostCat = 1003;
              } else if (score >= 0.825) {
                XGBoostCat = 1002;
              } else if (score >= 0.520) {
                XGBoostCat = 1001;
              } else {
                XGBoostCat = 1000;
              }

              XGBoostScore = score;
            } else{
              score = m_bdts.at(HHBBYY::BDT::HH2025_KF_low_mass_Run3_with2024)->GetClassification(vars);

              if (score >= 0.960) {
                XGBoostCat = 3004;
              } else if (score >= 0.935) {
                XGBoostCat = 3003;
              } else if (score >= 0.875) {
                XGBoostCat = 3002;
              } else if (score >= 0.585) {
                XGBoostCat = 3001;
              } else {
                XGBoostCat = 3000;
              }

              XGBoostScore = score;
            }

          }
          eventInts.at(HHBBYY::Var::bdt_sel_category_KF_with2024) = XGBoostCat;
          eventFloats.at(HHBBYY::Var::bdt_sel_score_KF_with2024) = XGBoostScore;

        }
      } else { //correlated model
        if (!is2024Model) { //correlated model without 2024
          if (eventFloats.at(HHBBYY::Var::bbyy_mStar_KF) >= 350 * Athena::Units::GeV) {
            float score = -99;
            // get BDT score
            score = m_bdts.at(HHBBYY::BDT::HH2025_KF_high_mass_Run23)->GetClassification(vars);
            if (score >= 0.950) {
              XGBoostCat = 3;
            } else if (score >= 0.870) {
              XGBoostCat = 2;
            } else if (score >= 0.675) {
              XGBoostCat = 1;
            } else {
              XGBoostCat = 0;
            }
            XGBoostScore = score;
          } else {
            float score = -99;
            // get BDT score
            score = m_bdts.at(HHBBYY::BDT::HH2025_KF_low_mass_Run23)->GetClassification(vars);
            if (score >= 0.975) {
              XGBoostCat = 1004;
            } else if (score >= 0.950) {
              XGBoostCat = 1003;
            } else if (score >= 0.875) {
              XGBoostCat = 1002;
            } else if (score >= 0.605) {
              XGBoostCat = 1001;
            } else {
              XGBoostCat = 1000;
            }
            XGBoostScore = score;
          }
          eventInts.at(HHBBYY::Var::bdt_sel_category_KF_corr) = XGBoostCat;
          eventFloats.at(HHBBYY::Var::bdt_sel_score_KF_corr) = XGBoostScore;
        } else { //correlated model with 2024
          if (eventFloats.at(HHBBYY::Var::bbyy_mStar_KF) >= 350 * Athena::Units::GeV) {
            float score = -99;
            // get BDT score
            score = m_bdts.at(HHBBYY::BDT::HH2025_KF_high_mass_Run23_with2024)->GetClassification(vars);
            if (score >= 0.960) {
              XGBoostCat = 3;
            } else if (score >= 0.925) {
              XGBoostCat = 2;
            } else if (score >= 0.775) {
              XGBoostCat = 1;
            } else {
              XGBoostCat = 0;
            }
            XGBoostScore = score;
          } else {
            float score = -99;
            // get BDT score
            score = m_bdts.at(HHBBYY::BDT::HH2025_KF_low_mass_Run23_with2024)->GetClassification(vars);
            if (score >= 0.980) {
              XGBoostCat = 1004;
            } else if (score >= 0.960) {
              XGBoostCat = 1003;
            } else if (score >= 0.895) {
              XGBoostCat = 1002;
            } else if (score >= 0.620) {
              XGBoostCat = 1001;
            } else {
              XGBoostCat = 1000;
            }
            XGBoostScore = score;
          }
          eventInts.at(HHBBYY::Var::bdt_sel_category_KF_corr_with2024) = XGBoostCat;
          eventFloats.at(HHBBYY::Var::bdt_sel_score_KF_corr_with2024) = XGBoostScore;
        }

      }
    }
  }

  ConstDataVector<xAOD::JetContainer> BaselineVarsbbyyAlg::categorisation_jets(const xAOD::Jet *Hbb_Jet1, const xAOD::Jet *Hbb_Jet2,
                                                                               const xAOD::JetContainer *jets) {
    ConstDataVector<xAOD::JetContainer> categorisation_jets{Hbb_Jet1, Hbb_Jet2};

    // retreive the third and fourth jets
    if (jets->size() > 2) {
      for (const xAOD::Jet *jet : *jets) {
        if (jet == Hbb_Jet1 || jet == Hbb_Jet2) {
          continue;
        }
        categorisation_jets.push_back(jet);
        if (categorisation_jets.size() == 4) {
          break;
        }
      }
    }
    return categorisation_jets;
  }

  //#######################################################################################################################################################################################################
  std::vector<double> BaselineVarsbbyyAlg::compute_angular_variables_CM(
									const TLorentzVector& lz_photon1,
									const TLorentzVector& lz_photon2,
									const TLorentzVector& lz_b_jet1,
									const TLorentzVector& lz_b_jet2)
  {
    //for principles considered, consider page 8 of
    //https://indico.cern.ch/event/1384215/contributions/5854190/attachments/2817099/4918457/escalier_11_March_2024.pdf

    double cos_theta_gamgam_cm_yybb=-99999;
    double phi_gamgam_cm_yybb=-99999;
    double cos_theta_photon1_cm_gamgam=-99999;
    double phi_photon1_cm_gamgam=-99999;
    double cos_theta_b_jet1_cm_bb=-99999;
    double phi_b_jet1_cm_bb=-99999;
    double DeltaPhi_gamgam_bb_cm_yybb=-99999;

    TLorentzVector lz_bbgamgam=lz_photon1+lz_photon2+lz_b_jet1+lz_b_jet2;
    //- - - - - - - - - - - - - - - - - - - - - - - -
    //step 1 : a) rotate around z-axis particles so that X=bbyy is in xOz, in order to suppress arbitrary phase of system of 4 particles
    //         b) rotate around y-axis for next steps
    //         c) boost in X center-of-mass (cm) frame

    double theta_bbgamgam=lz_bbgamgam.Theta();
    double phi_bbgamgam=lz_bbgamgam.Phi();

    ATH_MSG_DEBUG("================\n");
    ATH_MSG_DEBUG("BaselineVarsbbyyAlg::compute_angular_variables_CM\n");
    ATH_MSG_DEBUG("new event\n");
    ATH_MSG_DEBUG("--------------------\n");
    ATH_MSG_DEBUG("begin step 1\n");
    ATH_MSG_DEBUG("theta_bbgamgam"+std::to_string(theta_bbgamgam)+", phi_bbgamgam="+std::to_string(phi_bbgamgam)+"\n\n");

    //a)
    std::vector<TLorentzVector> yybb_particles = {lz_photon1, lz_photon2,
                                                  lz_b_jet1, lz_b_jet2};

    TLorentzVector lz_bbgamgam_step_a;
    for (auto &lz : yybb_particles) {
      lz.RotateZ(-phi_bbgamgam);
      lz_bbgamgam_step_a+=lz;
    }

    //operations done so far : rotZ_minus_phi_bbgamgam;

    ATH_MSG_DEBUG("sanity check: Py should be null\n");
    ATH_MSG_DEBUG("lz_bbgamgam.Px() after rotZ_minus_phi_bbgamgam="+std::to_string(lz_bbgamgam_step_a.Px())+"\n");
    ATH_MSG_DEBUG("lz_bbgamgam.Py() after rotZ_minus_phi_bbgamgam="+std::to_string(lz_bbgamgam_step_a.Py())+"\n");
    ATH_MSG_DEBUG("lz_bbgamgam.Pz() after rotZ_minus_phi_bbgamgam="+std::to_string(lz_bbgamgam_step_a.Pz())+"\n");
    ATH_MSG_DEBUG("\n");

    //b) //this quantity is used for step 2
    TLorentzVector lz_bbgamgam_step_b;
    for (auto &lz : yybb_particles) {
      lz.RotateY(-theta_bbgamgam);
      lz_bbgamgam_step_b+=lz;
    }

    //operations done so far : rotZ_minus_phi_bbgamgam_rotY_minus_theta_bbgamgam

    ATH_MSG_DEBUG("second sanity check: Px and Py should be null\n");
    ATH_MSG_DEBUG("lz_bbgamgam.Px() after rotZ_minus_phi_bbgamgam_rotY_minus_theta_bbgamgam="+std::to_string(lz_bbgamgam_step_b.Px())+"\n");
    ATH_MSG_DEBUG("lz_bbgamgam.Py() after rotZ_minus_phi_bbgamgam_rotY_minus_theta_bbgamgam="+std::to_string(lz_bbgamgam_step_b.Py())+"\n");
    ATH_MSG_DEBUG("lz_bbgamgam.Pz() after rotZ_minus_phi_bbgamgam_rotY_minus_theta_bbgamgam="+std::to_string(lz_bbgamgam_step_b.Pz())+"\n\n");

    //c)
    //Boost in X center of mass

    //careful: Boost() moves from rod frame to the original frame, so one applies a "-" sign
    //https://root.cern.ch/doc/master/classTLorentzVector.html
    //see also discussion: https://root-forum.cern.ch/t/how-to-use-boost-in-tlorentzvector/4102

    TVector3 boost_vector_bbgamgam=lz_bbgamgam_step_b.BoostVector();

    TLorentzVector lz_bbgamgam_step_c;

    for (auto &lz : yybb_particles) {
      lz.Boost(-boost_vector_bbgamgam);
      lz_bbgamgam_step_c+=lz;
    }

    //operations done so far : rotZ_minus_phi_bbgamgam_rotY_minus_theta_bbgamgam_boosted_cm_yybb

    //mandatory to keep this information for the last step of the code (DeltaPhi between the two planes : plane yy and plane bb)
    TLorentzVector lz_photon1_step_1=yybb_particles[0];
    TLorentzVector lz_photon2_step_1=yybb_particles[1];
    TLorentzVector lz_b_jet1_step_1=yybb_particles[2];
    TLorentzVector lz_b_jet2_step_1=yybb_particles[3];

    ATH_MSG_DEBUG("sanity check: 3-vector bbgamgam should be null\n");
    ATH_MSG_DEBUG("lz_bbgamgam.Px() after rotZ_minus_phi_bbgamgam_rotY_minus_theta_bbgamgam_boosted_cm_yybb="+std::to_string(lz_bbgamgam_step_c.Px())+"\n");
    ATH_MSG_DEBUG("lz_bbgamgam.Py() after rotZ_minus_phi_bbgamgam_rotY_minus_theta_bbgamgam_boosted_cm_yybb="+std::to_string(lz_bbgamgam_step_c.Py())+"\n");
    ATH_MSG_DEBUG("lz_bbgamgam.Pz() after rotZ_minus_phi_bbgamgam_rotY_minus_theta_bbgamgam_boosted_cm_yybb="+std::to_string(lz_bbgamgam_step_c.Pz())+"\n\n");

    ATH_MSG_DEBUG("sanity check that gamgam and bb are opposite in cm of X\n");
    ATH_MSG_DEBUG("gamgam_cm_yybb.Px()="+std::to_string((yybb_particles[0]+yybb_particles[1]).Px())+"\n");
    ATH_MSG_DEBUG("gamgam_cm_yybb.Py()="+std::to_string((yybb_particles[0]+yybb_particles[1]).Py())+"\n");
    ATH_MSG_DEBUG("gamgam_cm_yybb.Pz()="+std::to_string((yybb_particles[0]+yybb_particles[1]).Pz())+"\n");

    ATH_MSG_DEBUG("bb_cm_yybb.Px()="+std::to_string((yybb_particles[2]+yybb_particles[3]).Px())+"\n");
    ATH_MSG_DEBUG("bb_cm_yybb.Py()="+std::to_string((yybb_particles[2]+yybb_particles[3]).Py())+"\n");
    ATH_MSG_DEBUG("bb_cm_yybb.Pz()="+std::to_string((yybb_particles[2]+yybb_particles[3]).Pz())+"\n");

    //double m_p_gamgam_cms_yybb=(lz_photon1_step_1+lz_photon2_step_1).P(); //potential additional variable for prospects
    //- - - - - - - - - - - - - - - - - - - - - - - -
    //step 2 : at this stage, X is in the LHC frame (x'Oz') : protons axis and axis to center of LHC ring
    //compute theta_gamgam, phi_gamgam, and prepare next frame

    ATH_MSG_DEBUG("--------------------\n");
    ATH_MSG_DEBUG("begin step 2\n");

    TLorentzVector lz_gamgam_step_2=yybb_particles[0]+yybb_particles[1];
    double theta_gamgam_cm_yybb=lz_bbgamgam_step_b.Angle(lz_gamgam_step_2.Vect());
    cos_theta_gamgam_cm_yybb=cos(theta_gamgam_cm_yybb);
    phi_gamgam_cm_yybb=lz_gamgam_step_2.Phi();

    ATH_MSG_DEBUG("theta_gamgam_cm_yybb="+std::to_string(theta_gamgam_cm_yybb)+"\n");
    ATH_MSG_DEBUG("phi_gamgam_cm_yybb="+std::to_string(phi_gamgam_cm_yybb)+"\n");

    //sanity check : compute theta_bb
    TLorentzVector lz_bb_step_2=yybb_particles[2]+yybb_particles[3]; //this lorentzvector is used also afterwards, at step 4
    double theta_bb_cm_yybb=lz_bbgamgam_step_b.Angle(lz_bb_step_2.Vect());
    double phi_bb_cm_yybb=lz_bb_step_2.Phi();

    ATH_MSG_DEBUG("sanity check : check that theta_bb+theta_gamgam=pi and that phi_gamgam-phi_bb=pi\n");
    ATH_MSG_DEBUG("theta_bb_cm_yybb="+std::to_string(theta_bb_cm_yybb)+", theta_gamgam_cm_yybb+theta_bb_cm_yybb="+std::to_string(theta_gamgam_cm_yybb+theta_bb_cm_yybb)+"\n");
    ATH_MSG_DEBUG("phi_bb_cm_yybb="+std::to_string(phi_bb_cm_yybb)+"\n");
    ATH_MSG_DEBUG("phi_gamgam_cm_yybb-phi_bb_cm_yybb="+std::to_string(phi_gamgam_cm_yybb-phi_bb_cm_yybb)+"\n");
    //- - - - - - - - - - - - - - - - - - - - - - - -
    //step 3 : from X center of frame, rotate particles and go in yy cm frame

    ATH_MSG_DEBUG("--------------------\n");
    ATH_MSG_DEBUG("begin step 3\n");

    //rotate particles
    //rotate around z axis

    std::vector<TLorentzVector> yy_particles = {yybb_particles[0],yybb_particles[1]};
    TLorentzVector lz_gamgam_step_a;

    for (auto &lz : yy_particles) {
      lz.RotateZ(-phi_gamgam_cm_yybb);
      lz_gamgam_step_a+=lz;
    }

    ATH_MSG_DEBUG("sanity check: Py should be null\n");
    ATH_MSG_DEBUG("lz_gamgam_step_a.Px()="+std::to_string(lz_gamgam_step_a.Px())+"\n");
    ATH_MSG_DEBUG("lz_gamgam_step_a.Py()="+std::to_string(lz_gamgam_step_a.Py())+"\n");
    ATH_MSG_DEBUG("lz_gamgam_step_a.Pz()="+std::to_string(lz_gamgam_step_a.Pz())+"\n");

    //rotate around y axis

    TLorentzVector lz_gamgam_step_b;

    for (auto &lz : yy_particles) {
      lz.RotateY(-theta_gamgam_cm_yybb);
      lz_gamgam_step_b+=lz;
    }

    ATH_MSG_DEBUG("second sanity check: Px and Py should be null\n");
    ATH_MSG_DEBUG("lz_gamgam_step_b.Px()="+std::to_string(lz_gamgam_step_b.Px())+"\n");
    ATH_MSG_DEBUG("lz_gamgam_step_b.Py()="+std::to_string(lz_gamgam_step_b.Py())+"\n");
    ATH_MSG_DEBUG("lz_gamgam_step_b.Pz()="+std::to_string(lz_gamgam_step_b.Pz())+"\n");

    //boost to center of frame of gamgam

    TVector3 boost_vector_gamgam_step_b=lz_gamgam_step_b.BoostVector();

    //info for debugging (not used for computation): before rotation
    TVector3 boost_vector_gamgam_step_2=lz_gamgam_step_2.BoostVector();

    ATH_MSG_DEBUG("sanity check: check that boost vector gamgam (here) is in opposite direction to the one of bb (in step 5)\n");
    ATH_MSG_DEBUG("boost_vector_gamgam_step_b.Px()="+std::to_string(boost_vector_gamgam_step_b.Px())+"\n");
    ATH_MSG_DEBUG("boost_vector_gamgam_step_b.Py()="+std::to_string(boost_vector_gamgam_step_b.Py())+"\n");
    ATH_MSG_DEBUG("boost_vector_gamgam_step_b.Pz()="+std::to_string(boost_vector_gamgam_step_b.Pz())+"\n");

    ATH_MSG_DEBUG("boost_vector_gamgam_step_2.Px()="+std::to_string(boost_vector_gamgam_step_2.Px())+"\n");
    ATH_MSG_DEBUG("boost_vector_gamgam_step_2.Py()="+std::to_string(boost_vector_gamgam_step_2.Py())+"\n");
    ATH_MSG_DEBUG("boost_vector_gamgam_step_2.Pz()="+std::to_string(boost_vector_gamgam_step_2.Pz())+"\n");

    for (auto &lz : yy_particles)
      lz.Boost(-boost_vector_gamgam_step_b);

    //operations done so far : rotZ_minus_phi_bbgamgam_rotY_minus_theta_bbgamgam_boosted_cm_yybb_rotZ_minus_phi_gamgam_rotY_minus_theta_gamgam_boosted_cm_gamgam

    double theta_photon1_cm_gamgam=lz_gamgam_step_b.Angle(yy_particles[0].Vect());
    cos_theta_photon1_cm_gamgam=cos(theta_photon1_cm_gamgam);
    phi_photon1_cm_gamgam=yy_particles[0].Phi();

    ATH_MSG_DEBUG("theta_photon1_cm_gamgam="+std::to_string(theta_photon1_cm_gamgam)+", phi_photon1_cm_gamgam="+std::to_string(phi_photon1_cm_gamgam)+"\n");

    bool debug=0; //0 by default in order not to slow down code
    if (debug) { //only or debugging : sanity check : compute theta_photon2, phi_photon2 (it should be opposite to those of photon1)
      double theta_photon2_cm_gamgam=lz_gamgam_step_b.Angle(yy_particles[1].Vect());
      double cos_theta_photon2_cm_gamgam=cos(theta_photon2_cm_gamgam); //unused variable : only for check
      double phi_photon2_cm_gamgam=yy_particles[1].Phi(); //unused variable : only for check

      ATH_MSG_DEBUG("sanity check : check that theta_photon1_cm_gamgam+theta_photon2_cm_gamgam=pi and that theta_photon2_cm_gamgam-theta_photon1_cm_gamgam=pi\n");
      ATH_MSG_DEBUG("theta_photon2_cm_gamgam="+std::to_string(theta_photon2_cm_gamgam)+", theta_photon1_cm_gamgam+theta_photon2_cm_gamgam="+std::to_string(theta_photon1_cm_gamgam+theta_photon2_cm_gamgam)+"\n");
      ATH_MSG_DEBUG("cos_theta_photon2_cm_gamgam="+std::to_string(cos_theta_photon2_cm_gamgam)+"\n");
      ATH_MSG_DEBUG("phi_photon2_cm_gamgam="+std::to_string(phi_photon2_cm_gamgam)+", phi_photon2_cm_gamgam-phi_photon1_cm_gamgam="+std::to_string(phi_photon2_cm_gamgam-phi_photon1_cm_gamgam)+"\n");
    }

    ATH_MSG_DEBUG("photon1.Px() in cm gamgam : "+std::to_string(yybb_particles[0].Px())+"\n");
    ATH_MSG_DEBUG("photon1.Py() in cm gamgam : "+std::to_string(yybb_particles[0].Py())+"\n");
    ATH_MSG_DEBUG("photon1.Pz() in cm gamgam : "+std::to_string(yybb_particles[0].Pz())+"\n");
    ATH_MSG_DEBUG("photon2.Px() in cm gamgam : "+std::to_string(yybb_particles[1].Px())+"\n");
    ATH_MSG_DEBUG("photon2.Py() in cm gamgam : "+std::to_string(yybb_particles[1].Py())+"\n");
    ATH_MSG_DEBUG("photon2.Pz() in cm gamgam : "+std::to_string(yybb_particles[1].Pz())+"\n");
    //- - - - - - - - - - - - - - - - - - - - - - - -
    //step 4 : from X center of frame, rotate particles and go in bb cm frame

    ATH_MSG_DEBUG("--------------------\n");
    ATH_MSG_DEBUG("begin step 4\n");

    //rotate particles around z axis

    std::vector<TLorentzVector> bb_particles = {yybb_particles[2],yybb_particles[3]};
    TLorentzVector lz_bb_step_a;

    for (auto &lz : bb_particles) {
      lz.RotateZ(-phi_bb_cm_yybb);
      lz_bb_step_a+=lz;
    }

    //operations done so far : rotZ_minus_phi_bbgamgam_rotY_minus_theta_bbgamgam_boosted_cm_yybb_rotZ_minus_phi_bb;

    ATH_MSG_DEBUG("sanity check: Py should be null\n");
    ATH_MSG_DEBUG("lz_bb.Px() after rotZ_minus_phi_bbgamgam_rotY_minus_theta_bbgamgam_boosted_cm_yybb_rotZ_minus_phi_bb="+std::to_string(lz_bb_step_a.Px())+"\n");
    ATH_MSG_DEBUG("lz_bb.Py() after rotZ_minus_phi_bbgamgam_rotY_minus_theta_bbgamgam_boosted_cm_yybb_rotZ_minus_phi_bb="+std::to_string(lz_bb_step_a.Py())+"\n");
    ATH_MSG_DEBUG("lz_bb.Pz() after rotZ_minus_phi_bbgamgam_rotY_minus_theta_bbgamgam_boosted_cm_yybb_rotZ_minus_phi_bb="+std::to_string(lz_bb_step_a.Pz())+"\n");

    //rotate particles around y axis

    TLorentzVector lz_bb_step_b; //operations done : rotZ_minus_phi_bbgamgam_rotY_minus_theta_bbgamgam_boosted_cm_yybb_rotZ_minus_phi_bb_rotY_minus_theta_bb;

    for (auto &lz : bb_particles) {
      lz.RotateY(-theta_bb_cm_yybb);
      lz_bb_step_b+=lz;
    }

    //operations done so far : rotZ_minus_phi_bbgamgam_rotY_minus_theta_bbgamgam_boosted_cm_yybb_rotZ_minus_phi_bb_rotY_minus_theta_bb;

    ATH_MSG_DEBUG("second sanity check after operations rotZ_minus_phi_bbgamgam_rotY_minus_theta_bbgamgam_boosted_cm_yybb_rotZ_minus_phi_bb_rotY_minus_theta_bb : Px and Py should be null\n");
    ATH_MSG_DEBUG("lz_bb.Px() after rotZ_minus_phi_bbgamgam_rotY_minus_theta_bbgamgam_boosted_cm_yybb_rotZ_minus_phi_bb_rotY_minus_theta_bb="+std::to_string(lz_bb_step_b.Px())+"\n");
    ATH_MSG_DEBUG("lz_bb.Py() after rotZ_minus_phi_bbgamgam_rotY_minus_theta_bbgamgam_boosted_cm_yybb_rotZ_minus_phi_bb_rotY_minus_theta_bb="+std::to_string(lz_bb_step_b.Py())+"\n");
    ATH_MSG_DEBUG("lz_bb.Pz() after rotZ_minus_phi_bbgamgam_rotY_minus_theta_bbgamgam_boosted_cm_yybb_rotZ_minus_phi_bb_rotY_minus_theta_bb="+std::to_string(lz_bb_step_b.Pz())+"\n\n");

    //boost to center of frame of bb
    TVector3 boost_vector_bb_step_b=lz_bb_step_b.BoostVector();

    //info for debugging (not used for computation): before rotation
    TVector3 boost_vector_bb_step_2=lz_bb_step_2.BoostVector();

    ATH_MSG_DEBUG("sanity check: check that boost vector gamgam (in step 3) is in opposite direction to the one of bb (here)\n");
    ATH_MSG_DEBUG("boost_vector_bb_step_b.Px()="+std::to_string(boost_vector_bb_step_b.Px())+"\n");
    ATH_MSG_DEBUG("boost_vector_bb_step_b.Py()="+std::to_string(boost_vector_bb_step_b.Py())+"\n");
    ATH_MSG_DEBUG("boost_vector_bb_step_b.Pz()="+std::to_string(boost_vector_bb_step_b.Pz())+"\n");

    ATH_MSG_DEBUG("boost_vector_bb_step_2.Px()="+std::to_string(boost_vector_bb_step_2.Px())+"\n");
    ATH_MSG_DEBUG("boost_vector_bb_step_2.Py()="+std::to_string(boost_vector_bb_step_2.Py())+"\n");
    ATH_MSG_DEBUG("boost_vector_bb_step_2.Pz()="+std::to_string(boost_vector_bb_step_2.Pz())+"\n");

    for (auto &lz : bb_particles)
      lz.Boost(-boost_vector_bb_step_b);
    //- - - - - - - - - - - - - - - - - - - - - - - -
    //step 5 : from X center of frame, rotate particles and go in bb cm frame

    ATH_MSG_DEBUG("--------------------\n");
    ATH_MSG_DEBUG("begin step 5\n");

    double theta_b_jet1_cm_bb=lz_bb_step_b.Angle(bb_particles[0].Vect());
    cos_theta_b_jet1_cm_bb=cos(theta_b_jet1_cm_bb);
    phi_b_jet1_cm_bb=bb_particles[0].Phi();

    ATH_MSG_DEBUG("theta_b_jet1_cm_bb="+std::to_string(theta_b_jet1_cm_bb)+", phi_b_jet1_cm_bb="+std::to_string(phi_b_jet1_cm_bb)+"\n");

    //sanity check : compute theta_b_jet2, phi_b_jet2
    double theta_b_jet2_cm_bb=lz_bb_step_b.Angle(bb_particles[1].Vect());
    double phi_b_jet2_cm_bb=bb_particles[1].Phi();

    ATH_MSG_DEBUG("sanity check : check that theta_b_jet1_cm_bb+theta_b_jet2_cm_bb=pi and that theta_b_jet2_cm_bb-theta_b_jet1_cm_bb=pi\n");
    ATH_MSG_DEBUG("theta_b_jet2_cm_bb="+std::to_string(theta_b_jet2_cm_bb)+", theta_b_jet1_cm_bb+theta_b_jet2_cm_bb="+std::to_string(theta_b_jet1_cm_bb+theta_b_jet2_cm_bb)+"\n");
    ATH_MSG_DEBUG("phi_b_jet2_cm_bb="+std::to_string(phi_b_jet2_cm_bb)+", phi_b_jet2_cm_bb-phi_b_jet1_cm_bb="+std::to_string(phi_b_jet2_cm_bb-phi_b_jet1_cm_bb)+"\n");

    ATH_MSG_DEBUG("b_jet1.Px() in cm bb : "+std::to_string(bb_particles[0].Px())+"\n");
    ATH_MSG_DEBUG("b_jet1.Py() in cm bb : "+std::to_string(bb_particles[0].Py())+"\n");
    ATH_MSG_DEBUG("b_jet1.Pz() in cm bb : "+std::to_string(bb_particles[0].Pz())+"\n");

    ATH_MSG_DEBUG("b_jet2.Px() in cm bb : "+std::to_string(bb_particles[1].Px())+"\n");
    ATH_MSG_DEBUG("b_jet2.Py() in cm bb : "+std::to_string(bb_particles[1].Py())+"\n");
    ATH_MSG_DEBUG("b_jet2.Pz() in cm bb : "+std::to_string(bb_particles[1].Pz())+"\n");
    //- - - - - - - - - - - - - - - - - - - - - - - -
    //angle btw (gam, gam) and (b, b), in cm X

    TVector3 vec3_photon1_step_1=lz_photon1_step_1.Vect();
    TVector3 vec3_photon2_step_1=lz_photon2_step_1.Vect();

    TVector3 vec3_cross_product_photon1_photon2_cm_yybb=vec3_photon1_step_1.Cross(vec3_photon2_step_1);

    TVector3 vec3_b_jet1_step_1=lz_b_jet1_step_1.Vect();
    TVector3 vec3_b_jet2_step_1=lz_b_jet2_step_1.Vect();

    TVector3 vec3_cross_product_b_jet1_b_jet2_cm_yybb=vec3_b_jet1_step_1.Cross(vec3_b_jet2_step_1);

    DeltaPhi_gamgam_bb_cm_yybb=vec3_cross_product_photon1_photon2_cm_yybb.Angle(vec3_cross_product_b_jet1_b_jet2_cm_yybb);
    std::vector<double> vec_angular_variables_CM;

    vec_angular_variables_CM.push_back(cos_theta_gamgam_cm_yybb);
    vec_angular_variables_CM.push_back(phi_gamgam_cm_yybb);
    vec_angular_variables_CM.push_back(cos_theta_photon1_cm_gamgam);
    vec_angular_variables_CM.push_back(phi_photon1_cm_gamgam);
    vec_angular_variables_CM.push_back(cos_theta_b_jet1_cm_bb);
    vec_angular_variables_CM.push_back(phi_b_jet1_cm_bb);
    vec_angular_variables_CM.push_back(DeltaPhi_gamgam_bb_cm_yybb);

    return vec_angular_variables_CM;
  }
  //#######################################################################################################################################################################################################

}
