/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AthContainers/AuxElement.h"
#include "BaselineVarsbbyyAlg.h"
#include <FourMomUtils/xAODP4Helpers.h>
#include "PathResolver/PathResolver.h"

#include "TMatrixDSym.h"
#include "TMatrixDSymEigen.h"
#include "TVectorD.h"
#include "TFile.h"
#include "xAODEgamma/PhotonFwd.h"

#include <AthenaKernel/Units.h>
#include <iostream>

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

    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }
    if (!m_PCBT.empty()) {
      ATH_CHECK (m_PCBT.initialize(m_systematicsList, m_jetHandle));
    }

    ATH_CHECK (m_photonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));

    ATH_CHECK (m_selected_ph.initialize(m_systematicsList, m_photonHandle));

    if(m_isMC){
      m_ph_SF = CP::SysReadDecorHandle<float>("ph_effSF_"+m_photonWPName+"_%SYS%", this);
    }
    ATH_CHECK (m_ph_SF.initialize(m_systematicsList, m_photonHandle, SG::AllowEmpty));
    
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
    for (const std::string &path: m_bdts_path){
      std::unique_ptr<MVAUtils::BDT> bdt;
      loadBDT(path, bdt);
      m_bdts.push_back(std::move(bdt));
    }

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsbbyyAlg::execute()
  {
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      // container we read in
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      const xAOD::PhotonContainer *photons = nullptr;
      ANA_CHECK (m_photonHandle.retrieve (photons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::MissingETContainer *metCont = nullptr;
      ANA_CHECK (m_metHandle.retrieve (metCont, sys));
      const xAOD::MissingET* met = (*metCont)["Final"];
      if (!met) {
        ATH_MSG_ERROR("Could not retrieve MET");
        return StatusCode::FAILURE;	
      }

      static const SG::AuxElement::ConstAccessor<int>  HadronConeExclTruthLabelID("HadronConeExclTruthLabelID");
      static const SG::AuxElement::ConstAccessor<int> cacc_NMu("n_muons");
      static const SG::AuxElement::ConstAccessor<float> cacc_UncorrPt("uncorrPt");
      static const SG::AuxElement::ConstAccessor<float> cacc_MuonCorrPt("muonCorrPt");

      // initialize
      TLorentzVector H_bb(0.,0.,0.,0.);
      TLorentzVector H_yy(0.,0.,0.,0.);
      TLorentzVector HH(0.,0.,0.,0.);
      TLorentzVector y1(0.,0.,0.,0.);
      TLorentzVector y2(0.,0.,0.,0.);
      TLorentzVector j(0.,0.,0.,0.);
      TLorentzVector Hbb_candidate1(0.,0.,0.,0.);
      TLorentzVector Hbb_candidate2(0.,0.,0.,0.);

      int j_passWP=-99;
      int truthLabel_j = -99;
      int PCBTjet = -99;
      int truthLabel_b1 = -99, truthLabel_b2 = -99;
      int PCBT_candidate1 = -99, PCBT_candidate2 = -99;
      double dRHH = -99., dRyy = -99., dRbb = -99.;
      float bbyy_mStar = -99.;
      
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

      bool WPgiven = !m_isBtag.empty();
      bool PCBTgiven = !m_PCBT.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);

      for(const xAOD::Jet* jet : *jets) {
        // Compute scalar pt sum (Ht) for all the jets in the event |eta|<4.4
        HT += jet->pt();

        // count central jets
        if (std::abs(jet->eta())<2.5){
          nCentralJets++;
          // check if jet is btagged
          if (WPgiven && m_isBtag.get(*jet, sys)) bjets->push_back(jet);
        }
      }
      
      eventFloats.at(HHBBYY::Var::jets_HT) = HT;

      const xAOD::Photon* ph1 = nullptr;
      const xAOD::Photon* ph2 = nullptr;
      for(const xAOD::Photon* photon : *photons) {
        if (m_selected_ph.get(*photon, sys)){
          if(!ph1) ph1 = photon;
          else if (!ph2){
            ph2 = photon;
            break;
          }
        }
      }

      // photon sector
      if (ph1){
        y1 = ph1->p4();
        m_Fbranches.at("Photon1_pt").set(*event, y1.Pt(), sys);
        m_Fbranches.at("Photon1_eta").set(*event, y1.Eta(), sys);
        m_Fbranches.at("Photon1_phi").set(*event, y1.Phi(), sys);
        m_Fbranches.at("Photon1_E").set(*event, y1.E(), sys);
        if(m_isMC){
          float ph_SF = m_ph_SF.get(*ph1, sys);
          m_Fbranches.at("Photon1_effSF").set(*event, ph_SF, sys);
        }
      }

      if (ph1 && ph2) {
        y2 = ph2->p4();

        // Build the H(yy) candidate
        H_yy = y1 + y2;
        dRyy = (y1).DeltaR(y2);

        m_Fbranches.at("Photon2_pt").set(*event, y2.Pt(), sys);
        m_Fbranches.at("Photon2_eta").set(*event, y2.Eta(), sys);
        m_Fbranches.at("Photon2_phi").set(*event, y2.Phi(), sys);
        m_Fbranches.at("Photon2_E").set(*event, y2.E(), sys);
        if(m_isMC){
          float ph_SF = m_ph_SF.get(*ph2, sys);
          m_Fbranches.at("Photon2_effSF").set(*event, ph_SF, sys);
        }

        m_Fbranches.at("myy").set(*event, H_yy.M(), sys);
        m_Fbranches.at("pTyy").set(*event, H_yy.Pt(), sys);
        m_Fbranches.at("Etayy").set(*event, H_yy.Eta(), sys);
        m_Fbranches.at("Phiyy").set(*event, H_yy.Phi(), sys);
        m_Fbranches.at("dRyy").set(*event, dRyy, sys);

        m_Fbranches.at("Photon1_ptOvermyy").set(*event, y1.Pt()/H_yy.M(), sys);
        m_Fbranches.at("Photon2_ptOvermyy").set(*event, y2.Pt()/H_yy.M(), sys);

        eventFloats.at(HHBBYY::Var::yy_dR) = dRyy;

      }

      // inclusive jet sector
      for (std::size_t i=0; i<std::min(jets->size(),(std::size_t)4); i++){	 
        j = jets->at(i)->p4();
        if (m_isMC) 
          truthLabel_j = HadronConeExclTruthLabelID(*jets->at(i));
        j_passWP = static_cast<int>(m_isBtag.get(*jets->at(i), sys));
        PCBTjet= m_PCBT.get(*jets->at(i), sys);
      
        m_Fbranches.at("Jet"+std::to_string(i+1)+"_pt").set(*event, j.Pt(), sys);
        m_Fbranches.at("Jet"+std::to_string(i+1)+"_eta").set(*event, j.Eta(), sys);
        m_Fbranches.at("Jet"+std::to_string(i+1)+"_phi").set(*event, j.Phi(), sys);
        m_Fbranches.at("Jet"+std::to_string(i+1)+"_E").set(*event, j.E(), sys);

        m_Ibranches.at("Jet"+std::to_string(i+1)+"_PassWP").set(*event,j_passWP,sys);

        if(PCBTgiven)
          m_Ibranches.at("Jet"+std::to_string(i+1)+"_pcbt").set(*event,PCBTjet,sys);

        if (m_isMC)
          m_Ibranches.at("Jet"+std::to_string(i+1)+"_truthLabel").set(*event, truthLabel_j, sys);
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
        Hbb_candidate1= Hbb_Jet1->p4();
        Hbb_candidate2= Hbb_Jet2->p4();
        PCBT_candidate1 = m_PCBT.get(*Hbb_Jet1, sys);
        PCBT_candidate2 = m_PCBT.get(*Hbb_Jet2, sys);

        if (m_isMC) {      
          truthLabel_b1 = HadronConeExclTruthLabelID(*Hbb_Jet1);
          truthLabel_b2 = HadronConeExclTruthLabelID(*Hbb_Jet2);
        }
        m_Ibranches.at("HbbCandidate_Jet1_n_muons").set(*event, cacc_NMu(*Hbb_Jet1), sys);
        m_Fbranches.at("HbbCandidate_Jet1_uncorrPt").set(*event, cacc_UncorrPt(*Hbb_Jet1), sys);
        m_Fbranches.at("HbbCandidate_Jet1_muonCorrPt").set(*event, cacc_MuonCorrPt(*Hbb_Jet1), sys);
        m_Fbranches.at("HbbCandidate_Jet1_pt").set(*event, Hbb_candidate1.Pt(), sys);
        m_Fbranches.at("HbbCandidate_Jet1_eta").set(*event, Hbb_candidate1.Eta(), sys);
        m_Fbranches.at("HbbCandidate_Jet1_phi").set(*event, Hbb_candidate1.Phi(), sys);
        m_Fbranches.at("HbbCandidate_Jet1_E").set(*event, Hbb_candidate1.E(), sys);

        m_Ibranches.at("HbbCandidate_Jet2_n_muons").set(*event, cacc_NMu(*Hbb_Jet2), sys);
        m_Fbranches.at("HbbCandidate_Jet2_uncorrPt").set(*event, cacc_UncorrPt(*Hbb_Jet2), sys);
        m_Fbranches.at("HbbCandidate_Jet2_muonCorrPt").set(*event, cacc_MuonCorrPt(*Hbb_Jet2), sys);
        m_Fbranches.at("HbbCandidate_Jet2_pt").set(*event, Hbb_candidate2.Pt(), sys);
        m_Fbranches.at("HbbCandidate_Jet2_eta").set(*event, Hbb_candidate2.Eta(), sys);
        m_Fbranches.at("HbbCandidate_Jet2_phi").set(*event, Hbb_candidate2.Phi(), sys);
        m_Fbranches.at("HbbCandidate_Jet2_E").set(*event, Hbb_candidate2.E(), sys);

        if(PCBTgiven){
          m_Ibranches.at("HbbCandidate_Jet1_pcbt").set(*event,PCBT_candidate1,sys);
          m_Ibranches.at("HbbCandidate_Jet2_pcbt").set(*event,PCBT_candidate2,sys);
        }

        if (m_isMC) {
          m_Ibranches.at("HbbCandidate_Jet1_truthLabel").set(*event, truthLabel_b1, sys);
          m_Ibranches.at("HbbCandidate_Jet2_truthLabel").set(*event, truthLabel_b2, sys);
        }

        H_bb = Hbb_candidate1 + Hbb_candidate2;
        dRbb = (Hbb_candidate1).DeltaR(Hbb_candidate2);

        m_Fbranches.at("mbb").set(*event, H_bb.M(), sys);
        m_Fbranches.at("pTbb").set(*event, H_bb.Pt(), sys);
        m_Fbranches.at("Etabb").set(*event, H_bb.Eta(), sys);
        m_Fbranches.at("Phibb").set(*event, H_bb.Phi(), sys);
        m_Fbranches.at("dRbb").set(*event, dRbb, sys);
        
      }

      // Build the HH candidate
      if (ph1 && ph2 && bjets->size() >= 2) {
        HH = H_yy + H_bb;
        dRHH = H_yy.DeltaR(H_bb);

        double Higgs_mass = 125. * Athena::Units::GeV;
        bbyy_mStar = HH.M() - (H_bb.M() - Higgs_mass)-(H_yy.M() - Higgs_mass);

        m_Fbranches.at("mbbyy").set(*event, HH.M(), sys);
        m_Fbranches.at("mbbyy_star").set(*event, bbyy_mStar, sys);
        m_Fbranches.at("pTbbyy").set(*event, HH.Pt(), sys);
        m_Fbranches.at("Etabbyy").set(*event, HH.Eta(), sys);
        m_Fbranches.at("Phibbyy").set(*event, HH.Phi(), sys);
        m_Fbranches.at("dRbbyy").set(*event, dRHH, sys);
        eventFloats.at(HHBBYY::Var::bbyy_mStar) = bbyy_mStar;

	//additional angular variables in referential of center of frame of HH and H

	std::vector<double> vec_angular_variables_CM=compute_angular_variables_CM(y1,y2,Hbb_candidate1,Hbb_candidate2);
	
	m_Fbranches.at("cos_theta_yy_cm_bbyy").set(*event,vec_angular_variables_CM[0],sys);
	m_Fbranches.at("phi_yy_cm_bbyy").set(*event,vec_angular_variables_CM[1], sys);
	
	m_Fbranches.at("Photon1_cos_theta_cm_gamgam").set(*event,vec_angular_variables_CM[2], sys);
	m_Fbranches.at("Photon1_phi_cm_gamgam").set(*event,vec_angular_variables_CM[3], sys);
	
	m_Fbranches.at("HbbCandidate_Jet1_cos_theta_cm_bb").set(*event,vec_angular_variables_CM[4], sys);
	m_Fbranches.at("HbbCandidate_Jet1_phi_cm_bb").set(*event,vec_angular_variables_CM[5], sys);
	m_Fbranches.at("DeltaPhi_bb_yy_cm_bbyy").set(*event,vec_angular_variables_CM[6], sys);
      }
      
      // Find VBF jets
      float vbf_jj_deta = -999;
      float vbf_mjj    = -999;
      float dR_yybb_vbfj1 = -999;
      float dR_yybb_vbfj2 = -999;
      float deta_yybb_vbfj1 = -999;
      float deta_yybb_vbfj2 = -999;
      float dR_yybb_jj = -999;
      float deta_yybb_jj = -999;
      
      TLorentzVector vbf_j[2];
      TLorentzVector vbf_jj(0.,0.,0.,0.);
      TLorentzVector yybbjj(0.,0.,0.,0.);

      if(jets->size()>=4&&bjets->size()>=2){
        for (size_t i=0; i<jets->size()-1; i++){
          TLorentzVector j1 = (*jets)[i]->p4();
          if((*jets)[i]->p4()==Hbb_candidate1||(*jets)[i]->p4()==Hbb_candidate2) continue;
          for (size_t j = i+1; j<jets->size(); j++){
            TLorentzVector j2 = (*jets)[j]->p4();
            if((*jets)[j]->p4()==Hbb_candidate1||(*jets)[j]->p4()==Hbb_candidate2) continue;
            
            TLorentzVector iPair = j1 + j2;
            if(iPair.M() > vbf_mjj){
              vbf_mjj = iPair.M();
              vbf_j[0] = j1;
              vbf_j[1] = j2;
              vbf_jj = iPair;
            }
          }
        }
        vbf_jj_deta = std::fabs(vbf_j[0].Eta() - vbf_j[1].Eta());
        yybbjj = vbf_jj + HH;
        dR_yybb_vbfj1 = vbf_j[0].DeltaR(HH); 
        dR_yybb_vbfj2 = vbf_j[1].DeltaR(HH);
        dR_yybb_jj = vbf_jj.DeltaR(HH);
        deta_yybb_vbfj1 = std::fabs(vbf_j[0].Eta() - HH.Eta()); 
        deta_yybb_vbfj2 = std::fabs(vbf_j[1].Eta() - HH.Eta()); 
        deta_yybb_jj = std::fabs(vbf_jj.Eta() - HH.Eta());
      }

      for(unsigned int i=0; i<2; i++){
        std::string prefix = "Jet_vbf_j"+std::to_string(i+1);
        m_Fbranches.at(prefix+"_pt").set(*event, vbf_j[i].Pt(), sys);
        m_Fbranches.at(prefix+"_eta").set(*event, vbf_j[i].Eta(), sys);
        m_Fbranches.at(prefix+"_phi").set(*event, vbf_j[i].Phi(), sys);
        m_Fbranches.at(prefix+"_E").set(*event, vbf_j[i].E(), sys);
      }
      m_Fbranches.at("Jet_vbf_j1_yybb_dR").set(*event, dR_yybb_vbfj1, sys);
      m_Fbranches.at("Jet_vbf_j2_yybb_dR").set(*event, dR_yybb_vbfj2, sys);
      m_Fbranches.at("Jet_vbf_j1_yybb_deta").set(*event, deta_yybb_vbfj1, sys);
      m_Fbranches.at("Jet_vbf_j2_yybb_deta").set(*event, deta_yybb_vbfj2, sys);
      
      eventFloats.at(HHBBYY::Var::vbfjj_m) = vbf_mjj;
      eventFloats.at(HHBBYY::Var::vbfjj_dEta) = vbf_jj_deta;

      m_Fbranches.at("Jet_vbf_jj_m").set(*event, vbf_mjj, sys);
      m_Fbranches.at("Jet_vbf_jj_deta").set(*event, vbf_jj_deta, sys);
      m_Fbranches.at("Jet_vbf_jj_yybb_dR").set(*event, dR_yybb_jj, sys);
      m_Fbranches.at("Jet_vbf_jj_yybb_deta").set(*event, deta_yybb_jj, sys);
      m_Fbranches.at("Jet_vbf_jj_yybb_pT").set(*event, yybbjj.Pt(), sys);
      m_Fbranches.at("Jet_vbf_jj_yybb_eta").set(*event, yybbjj.Eta(), sys);
      m_Fbranches.at("Jet_vbf_jj_yybb_phi").set(*event, yybbjj.Phi(), sys);
      m_Fbranches.at("Jet_vbf_jj_yybb_m").set(*event, yybbjj.M(), sys);    
    
      // More global variables
      m_Fbranches.at("HT").set(*event, HT, sys);

      float topness = compute_Topness(jets);
      m_Fbranches.at("topness").set(*event, topness, sys);
      
      m_Fbranches.at("missEt").set(*event, met->met(), sys);
      m_Fbranches.at("metphi").set(*event, met->phi(), sys);
      
      float* eventShapes = compute_EventShapes(bjets, photons);
      m_Fbranches.at("sphericityT").set(*event, eventShapes[0], sys);
      m_Fbranches.at("planarFlow").set(*event, eventShapes[1], sys);

      eventFloats.at(HHBBYY::Var::sphericityT) = eventShapes[0];
      eventFloats.at(HHBBYY::Var::planarFlow) = eventShapes[1];
      eventFloats.at(HHBBYY::Var::topness) = topness;


      float pTBalance = compute_pTBalance(bjets, photons);
      m_Fbranches.at("pTBalance").set(*event, pTBalance, sys);

      m_Ibranches.at("nPhotons").set(*event, photons->size(), sys);
      m_Ibranches.at("nJets").set(*event, jets->size(), sys);
      m_Ibranches.at("nCentralJets").set(*event, nCentralJets, sys);
      m_Ibranches.at("nBJets").set(*event, bjets->size(), sys);
      m_Ibranches.at("nLeptons").set(*event, electrons->size() + muons->size(), sys);

      // bdt
      if (ph1 && ph2 && Hbb_Jet1 && Hbb_Jet2) {
        performCategorisationBDT(ph1, ph2, Hbb_Jet1, Hbb_Jet2, jets, metCont, sys, eventFloats, eventInts);
      }
      m_Fbranches.at("bdtSel_score").set(*event, eventFloats.at(HHBBYY::Var::bdt_sel_score), sys);
      m_Ibranches.at("bdtSel_category").set(*event, eventInts.at(HHBBYY::Var::bdt_sel_category), sys);
    }

    return StatusCode::SUCCESS;
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
            temp_jets.push_back(TLorentzVector(0, 0, 0, 0));
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
          if(topness<minTopness) minTopness=topness;
        }
      }
    }
    return minTopness;
  }

  /*
  eventShapes[0] = sphericityT;
  eventShapes[1] = planarFlow;
  */
  float* BaselineVarsbbyyAlg::compute_EventShapes(std::unique_ptr<ConstDataVector<xAOD::JetContainer>> &bjets,
                                                 const xAOD::PhotonContainer *photons){
    static float eventShapes[2] = {0};
    if (bjets->size() >= 2 && photons->size() >= 2) {
      TLorentzVector photon1 = photons->at(0)->p4();
      TLorentzVector photon2 = photons->at(1)->p4();
      TLorentzVector bjet1 = bjets->at(0)->p4();
      TLorentzVector bjet2 = bjets->at(1)->p4();
      std::vector<TLorentzVector> p4_vec = {photon1, photon2, bjet1, bjet2};

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

      TVectorD eigenVec = EigenValues.GetEigenValues();
      TVectorD eigenVecT = EigenValuesT.GetEigenValues();

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

  float BaselineVarsbbyyAlg::compute_pTBalance(std::unique_ptr<ConstDataVector<xAOD::JetContainer>> &bjets,
                                            const xAOD::PhotonContainer *photons){
    float pTBalance = -99;
    if (bjets->size() >= 2 && photons->size() >= 2) {
      TLorentzVector photon1 = photons->at(0)->p4();
      TLorentzVector photon2 = photons->at(1)->p4();
      TLorentzVector bjet1 = bjets->at(0)->p4();
      TLorentzVector bjet2 = bjets->at(1)->p4();
      float numerator, denominator;
      std::vector<TLorentzVector> p4_vec = {photon1, photon2, bjet1, bjet2};
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

  std::vector<float> BaselineVarsbbyyAlg::makeXGBoostDMatrixLegacyNonres(const xAOD::Photon *ph1, const xAOD::Photon *ph2,
                                                                         ConstDataVector<xAOD::JetContainer> &categorisation_jets,
                                                                         const xAOD::MissingETContainer *met, const auto &sys, 
                                                                         const std::map<HHBBYY::Var, float> &eventFloats) {
    // Assuming that the jets are already sorted by btagging score and then pT (jetSelectorAlg)
    // initialize the variables
    std::vector<float> vars(HHBBYY::Var::NVars, 0.0f);
    vars[HHBBYY::Var::j3_pcbt] = -9;
    vars[HHBBYY::Var::j4_pcbt] = -9;

    // Sort jets by pcbt and then by pT if same pcbt
    TLorentzVector H_bb = categorisation_jets.at(0)->p4() + categorisation_jets.at(1)->p4();

    // photons
    float myy = (ph1->p4() + ph2->p4()).M();
    vars[HHBBYY::Var::y1_ptOverMyy] = ph1->pt() / myy;
    vars[HHBBYY::Var::y1_eta] = ph1->eta();
    vars[HHBBYY::Var::y1y1_deltaPhi] = 0;  // dPhi(y1, y1);
    vars[HHBBYY::Var::y2_ptOverMyy] = ph2->pt() / myy;
    vars[HHBBYY::Var::y2_eta] = ph2->eta();
    vars[HHBBYY::Var::y1y2_deltaPhi] = xAOD::P4Helpers::deltaPhi(ph1, ph2);
    // met
    vars[HHBBYY::Var::met] = (*met)["Final"]->met();
    vars[HHBBYY::Var::y1met_deltaPhi] = xAOD::P4Helpers::deltaPhi(ph1, (*met)["Final"]);

    // jets
    int jet_ith = 0;
    for (const xAOD::Jet *jet : categorisation_jets) {

      // {jet_pt, jet_eta, jet_phi, jet_pseudo_score}
      if (jet_ith <= 1) {
        vars[HHBBYY::Var::j1_pt + (jet_ith * 4)] = jet->pt();
        vars[HHBBYY::Var::j1_eta + (jet_ith * 4)] = jet->eta();
        vars[HHBBYY::Var::y1j1_deltaPhi + (jet_ith * 4)] = xAOD::P4Helpers::deltaPhi(ph1, jet);
        int pseudoCont_score = m_PCBT.get(*jet, sys);
        // -9 for non-central jets, otherwise use GN2bin + 1 (just an arbitrary convention in the BDT model)
        pseudoCont_score = (pseudoCont_score <= 0 && std::abs(jet->eta()) > 2.5) ? -9 : pseudoCont_score + 1;
        vars[HHBBYY::Var::j1_pcbt + (jet_ith * 4)] = pseudoCont_score;

      } else {
        // Input variables for j2 and j3 are not contiguous, so cannot be included in previous if 
        vars[HHBBYY::Var::j3_pt + ((jet_ith - 2) * 4)] = jet->pt();
        vars[HHBBYY::Var::j3_eta + ((jet_ith - 2) * 4)] = jet->eta();
        vars[HHBBYY::Var::y1j3_deltaPhi + ((jet_ith - 2) * 4)] = xAOD::P4Helpers::deltaPhi(ph1, jet);
        int pseudoCont_score = m_PCBT.get(*jet, sys);
        // -9 for non-central jets, otherwise use GN2bin + 1 (just an arbitrary convention in the BDT model)
        pseudoCont_score = (pseudoCont_score <= 0 && std::abs(jet->eta()) > 2.5) ? -9 : pseudoCont_score + 1;
        vars[HHBBYY::Var::j3_pcbt + ((jet_ith - 2) * 4)] = pseudoCont_score;
      }
      jet_ith++;
    }

    // jj
    vars[HHBBYY::Var::bb_pt] = H_bb.Pt();
    vars[HHBBYY::Var::bb_eta] = H_bb.Eta();
    vars[HHBBYY::Var::y1bb_deltaPhi] = xAOD::P4Helpers::deltaPhi(ph1->phi(), H_bb.Phi());
    vars[HHBBYY::Var::bb_m] = H_bb.M();
    // event level jet variables
    vars[HHBBYY::Var::jets_HT] = eventFloats.at(HHBBYY::Var::jets_HT);
    // ChiWt variable
    vars[HHBBYY::Var::topness] = eventFloats.at(HHBBYY::Var::topness);

    // /!\ Missing values for VBF variables. Should be added with the VBF selection
    if (eventFloats.at(HHBBYY::Var::vbfjj_dEta) <0) {
      vars[HHBBYY::Var::vbfjj_dEta] = -999;
    } else {
      vars[HHBBYY::Var::vbfjj_dEta] = eventFloats.at(HHBBYY::Var::vbfjj_dEta);
    }
    if (eventFloats.at(HHBBYY::Var::vbfjj_m) <0) {
      vars[HHBBYY::Var::vbfjj_m] = -999;
    } else {
      vars[HHBBYY::Var::vbfjj_m] = eventFloats.at(HHBBYY::Var::vbfjj_m);
    }
    
    vars[HHBBYY::Var::bbyy_mStar] = eventFloats.at(HHBBYY::Var::bbyy_mStar);
    vars[HHBBYY::Var::yy_dR] = eventFloats.at(HHBBYY::Var::yy_dR);
    vars[HHBBYY::Var::bb_dR] = categorisation_jets.at(0)->p4().DeltaR(categorisation_jets.at(1)->p4());
    vars[HHBBYY::Var::sphericityT] = eventFloats.at(HHBBYY::Var::sphericityT);
    vars[HHBBYY::Var::planarFlow] = eventFloats.at(HHBBYY::Var::planarFlow);

    const float bbyy_sumPt = ph1->pt() + ph2->pt() + categorisation_jets.at(0)->pt() + categorisation_jets.at(1)->pt();
    TLorentzVector HH = ph1->p4() + ph2->p4() + categorisation_jets.at(0)->p4() + categorisation_jets.at(1)->p4();
    vars[HHBBYY::Var::bbyy_ptOverSumPt] = HH.Pt() / bbyy_sumPt;
    return vars;
  }

  void BaselineVarsbbyyAlg::performCategorisationBDT(const xAOD::Photon *ph1, const xAOD::Photon *ph2,
                                                     const xAOD::Jet *Hbb_Jet1, 
                                                     const xAOD::Jet *Hbb_Jet2, 
                                                     const xAOD::JetContainer *jets,
                                                     const xAOD::MissingETContainer *met, const auto &sys,
                                                     std::map<HHBBYY::Var, float> &eventFloats, 
                                                     std::map<HHBBYY::Var, int> &eventInts) {

    if (m_bdts.size()!=2){
      ANA_MSG_ERROR("2 BDTs are required for Low and High mass regions");
      return;
    }

    int XGBoostCat = -99;
    float XGBoostScore = -99.;

    // load data array for xgboost model prediction
    ConstDataVector<xAOD::JetContainer> cat_jets = categorisation_jets(Hbb_Jet1, Hbb_Jet2, jets);
    std::vector<float> vars = makeXGBoostDMatrixLegacyNonres(ph1, ph2,  cat_jets, met, sys, eventFloats);

    // High mass channel Selection
    if (eventFloats.at(HHBBYY::Var::bbyy_mStar) >= 350 * Athena::Units::GeV) {
      // get BDT score
      const float score = m_bdts.at(1)->GetClassification(vars);

      if (score >= 0.905) {
        XGBoostCat = 3;
      } else if (score >= 0.830) {
        XGBoostCat = 2;
      } else if (score >= 0.545) {
        XGBoostCat = 1;
      } else {
        XGBoostCat = 0;
      }

      XGBoostScore = score;

    } else {
      // get BDT score
      const float score = m_bdts.at(0)->GetClassification(vars);

      if (score >= 0.950) {
        XGBoostCat = 1004;
      } else if (score >= 0.89) {
        XGBoostCat = 1003;
      } else if (score >= 0.785) {
        XGBoostCat = 1002;
      } else if (score >= 0.430) {
        XGBoostCat = 1001;
      } else {
        XGBoostCat = 1000;
      }

      XGBoostScore = score;
    }
    eventInts.at(HHBBYY::Var::bdt_sel_category) = XGBoostCat;
    eventFloats.at(HHBBYY::Var::bdt_sel_score) = XGBoostScore;
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
