/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ResonantPNNbbllAlg.h"
#include "NeutrinoWeightingAlg.h"
#include "BaselineVarsbbllAlg.h"
#include <AthContainers/ConstDataVector.h>
#include "lwtnn/parse_json.hh"
#include <iostream>
#include <fstream>
#include "PathResolver/PathResolver.h"
#include <AthenaKernel/Units.h>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/member.hpp>

namespace HHBBLL
{
  ResonantPNNbbllAlg::ResonantPNNbbllAlg(const std::string &name,
					   ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {}

  StatusCode ResonantPNNbbllAlg ::initialize()
  {
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    //
    for (const std::string &var : m_floatVariables) {
      CP::SysWriteDecorHandle<float> whandle{var+"_%SYS%", this};
      m_Fbranches.emplace(var, whandle);
      ATH_CHECK (m_Fbranches.at(var).initialize(m_systematicsList, m_eventHandle));
    }
    //
    ATH_CHECK (m_met_sig.initialize(m_systematicsList, m_metHandle));
    //Retrieve NW and mT2bb
    ATH_CHECK (m_NW.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_mT2bb.initialize(m_systematicsList, m_eventHandle));
    //
    ATH_CHECK(m_year.initialize(m_systematicsList, m_eventHandle));
    //
    ATH_CHECK (m_systematicsList.initialize());
    //
    if(m_run_Run2_train) { //Use Run2 trainings for SR1 and SR2
      //SR1
      std::ifstream input_stream_pnn_sr1_cv1(PathResolverFindCalibFile("bbllAnalysis/PNN_SetA_Run2_SR1.json"));
      if(!input_stream_pnn_sr1_cv1.is_open()) {
	ATH_MSG_ERROR("Could not open Run2 bbllAnalysis/PNN_SetA_Run2_SR1.json");
	return StatusCode::FAILURE;
      }
      std::ifstream input_stream_pnn_sr1_cv2(PathResolverFindCalibFile("bbllAnalysis/PNN_SetB_Run2_SR1.json"));
      if(!input_stream_pnn_sr1_cv2.is_open()) {
	ATH_MSG_ERROR("Could not open Run2 bbllAnalysis/PNN_SetB_Run2_SR1.json");
	return StatusCode::FAILURE;
      }
      std::ifstream input_stream_pnn_sr1_cv3(PathResolverFindCalibFile("bbllAnalysis/PNN_SetC_Run2_SR1.json"));
      if(!input_stream_pnn_sr1_cv3.is_open()) {
	ATH_MSG_ERROR("Could not open Run2 bbllAnalysis/PNN_SetC_Run2_SR1.json");
	return StatusCode::FAILURE;
      }
      //SR2
      std::ifstream input_stream_pnn_sr2_cv1(PathResolverFindCalibFile("bbllAnalysis/PNN_SetA_Run2_SR2.json"));
      if(!input_stream_pnn_sr2_cv1.is_open()) {
	ATH_MSG_ERROR("Could not open Run2 bbllAnalysis/PNN_SetA_Run2_SR2.json");
	return StatusCode::FAILURE;
      }
      std::ifstream input_stream_pnn_sr2_cv2(PathResolverFindCalibFile("bbllAnalysis/PNN_SetB_Run2_SR2.json"));
      if(!input_stream_pnn_sr2_cv2.is_open()) {
	ATH_MSG_ERROR("Could not open Run2 bbllAnalysis/PNN_SetB_Run2_SR2.json");
	return StatusCode::FAILURE;
      }
      std::ifstream input_stream_pnn_sr2_cv3(PathResolverFindCalibFile("bbllAnalysis/PNN_SetC_Run2_SR2.json"));
      if(!input_stream_pnn_sr2_cv3.is_open()) {
	ATH_MSG_ERROR("Could not open Run2 bbllAnalysis/PNN_SetC_Run2_SR2.json");
	return StatusCode::FAILURE;
      }
      m_model_PNN_setA_SR1_Run2 = std::make_unique<lwt::LightweightGraph> (lwt::parse_json_graph(input_stream_pnn_sr1_cv1));
      m_model_PNN_setB_SR1_Run2 = std::make_unique<lwt::LightweightGraph> (lwt::parse_json_graph(input_stream_pnn_sr1_cv2));
      m_model_PNN_setC_SR1_Run2 = std::make_unique<lwt::LightweightGraph> (lwt::parse_json_graph(input_stream_pnn_sr1_cv3));
      m_model_PNN_setA_SR2_Run2 = std::make_unique<lwt::LightweightGraph> (lwt::parse_json_graph(input_stream_pnn_sr2_cv1));
      m_model_PNN_setB_SR2_Run2 = std::make_unique<lwt::LightweightGraph> (lwt::parse_json_graph(input_stream_pnn_sr2_cv2));
      m_model_PNN_setC_SR2_Run2 = std::make_unique<lwt::LightweightGraph> (lwt::parse_json_graph(input_stream_pnn_sr2_cv3));
    }
    else { //Run3 trainings
      //SR1
      std::ifstream input_stream_pnn_sr1_cv1(PathResolverFindCalibFile("bbllAnalysis/PNN_SetA_Run3_SR1.json"));
      if(!input_stream_pnn_sr1_cv1.is_open()) {
	ATH_MSG_ERROR("Could not open Run3 bbllAnalysis/PNN_SetA_Run3_SR1.json");
	return StatusCode::FAILURE;
      }
      std::ifstream input_stream_pnn_sr1_cv2(PathResolverFindCalibFile("bbllAnalysis/PNN_SetB_Run3_SR1.json"));
      if(!input_stream_pnn_sr1_cv2.is_open()) {
	ATH_MSG_ERROR("Could not open Run3 bbllAnalysis/PNN_SetB_Run3_SR1.json");
	return StatusCode::FAILURE;
      }
      std::ifstream input_stream_pnn_sr1_cv3(PathResolverFindCalibFile("bbllAnalysis/PNN_SetC_Run3_SR1.json"));
      if(!input_stream_pnn_sr1_cv3.is_open()) {
	ATH_MSG_ERROR("Could not open Run3 bbllAnalysis/PNN_SetC_Run3_SR1.json");
	return StatusCode::FAILURE;
      }
      //SR2
      std::ifstream input_stream_pnn_sr2_cv1(PathResolverFindCalibFile("bbllAnalysis/PNN_SetA_Run3_SR2.json"));
      if(!input_stream_pnn_sr2_cv1.is_open()) {
	ATH_MSG_ERROR("Could not open Run3 bbllAnalysis/PNN_SetA_Run3_SR2.json");
	return StatusCode::FAILURE;
      }
      std::ifstream input_stream_pnn_sr2_cv2(PathResolverFindCalibFile("bbllAnalysis/PNN_SetB_Run3_SR2.json"));
      if(!input_stream_pnn_sr2_cv2.is_open()) {
	ATH_MSG_ERROR("Could not open Run3 bbllAnalysis/PNN_SetB_Run3_SR2.json");
	return StatusCode::FAILURE;
      }
      std::ifstream input_stream_pnn_sr2_cv3(PathResolverFindCalibFile("bbllAnalysis/PNN_SetC_Run3_SR2.json"));
      if(!input_stream_pnn_sr2_cv3.is_open()) {
	ATH_MSG_ERROR("Could not open Run3 bbllAnalysis/PNN_SetC_Run3_SR2.json");
	return StatusCode::FAILURE;
      }
      m_model_PNN_setA_SR1_Run3 = std::make_unique<lwt::LightweightGraph> (lwt::parse_json_graph(input_stream_pnn_sr1_cv1));
      m_model_PNN_setB_SR1_Run3 = std::make_unique<lwt::LightweightGraph> (lwt::parse_json_graph(input_stream_pnn_sr1_cv2));
      m_model_PNN_setC_SR1_Run3 = std::make_unique<lwt::LightweightGraph> (lwt::parse_json_graph(input_stream_pnn_sr1_cv3));
      m_model_PNN_setA_SR2_Run3 = std::make_unique<lwt::LightweightGraph> (lwt::parse_json_graph(input_stream_pnn_sr2_cv1));
      m_model_PNN_setB_SR2_Run3 = std::make_unique<lwt::LightweightGraph> (lwt::parse_json_graph(input_stream_pnn_sr2_cv2));
      m_model_PNN_setC_SR2_Run3 = std::make_unique<lwt::LightweightGraph> (lwt::parse_json_graph(input_stream_pnn_sr2_cv3));
    }
    return StatusCode::SUCCESS;
  }

  StatusCode ResonantPNNbbllAlg::execute()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      // container we read in
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      const xAOD::MissingETContainer *metCont = nullptr;
      ANA_CHECK (m_metHandle.retrieve (metCont, sys));
      const xAOD::MissingET* met = (*metCont)["Final"];
      if (!met) {
	ATH_MSG_ERROR("Could not retrieve MET");
       	return StatusCode::FAILURE;	
      }
      //
      int year = m_year.get(*event, sys);
      // Count objects
      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      for(const xAOD::Jet* jet : *jets) {
	if (std::abs(jet->eta())<2.5) {
	  if (WPgiven && m_isBtag.get(*jet, sys)) bjets->push_back(jet);
	}
      }
      //
      for (const std::string &string_var: m_floatVariables) {
        m_Fbranches.at(string_var).set(*event, -99., sys);
      }
      //
      TLorentzVector Leading_lep;
      TLorentzVector Subleading_lep;
      TLorentzVector Leading_bjet;
      TLorentzVector Subleading_bjet;
      TLorentzVector met_vector;
      TLorentzVector bb;
      TLorentzVector ll;
      TLorentzVector bbll;
      TLorentzVector bbllmet;
      TLorentzVector b1l1;
      TLorentzVector b1l2;
      TLorentzVector b2l1;
      TLorentzVector b2l2;
      double m_bl = -99;
      //
      // Electron sector
      const xAOD::Electron* ele0 = nullptr;
      const xAOD::Electron* ele1 = nullptr;
      
      for(unsigned int i=0; i<std::min(size_t(2),electrons->size()); i++){
	const xAOD::Electron* ele = electrons->at(i);
	if(i==0) ele0 = ele;
	else if(i==1) ele1 = ele;
      }
      // Muon sector
      const xAOD::Muon* mu0 = nullptr;
      const xAOD::Muon* mu1 = nullptr;
      for(unsigned int i=0; i<std::min(size_t(2),muons->size()); i++){
	const xAOD::Muon* mu = muons->at(i);
	if(i==0) mu0 = mu;
	else if(i==1) mu1 = mu;
      }
      
      std::vector<std::pair<const xAOD::IParticle*, int>> leptons;
      if(ele0) leptons.emplace_back(ele0, -11*ele0->charge());
      if(mu0) leptons.emplace_back(mu0, -13*mu0->charge());
      if(ele1) leptons.emplace_back(ele1, -11*ele1->charge());
      if(mu1) leptons.emplace_back(mu1, -13*mu1->charge());
      
      std::sort(leptons.begin(), leptons.end(),
		[](const std::pair<const xAOD::IParticle*, int>& a,
		   const std::pair<const xAOD::IParticle*, int>& b) {
		  return a.first->pt() > b.first->pt(); });
      
      for(unsigned int i=0; i<std::min(size_t(2),leptons.size()); i++){
	TLorentzVector tlv = leptons[i].first->p4();
	if(i==0) Leading_lep = tlv;
	else if(i==1) Subleading_lep = tlv;
      }    
      if(leptons.size()>=2){
	ll = Leading_lep + Subleading_lep;
      }
      //b-jet sector
      for(unsigned int i=0; i<std::min(size_t(2),bjets->size()); i++){
	if(i==0) Leading_bjet = bjets->at(i)->p4();
	else if(i==1) Subleading_bjet = bjets->at(i)->p4();
      }
      
      if (bjets->size()>=2) {
	// build the H(bb) candidate
	bb = Leading_bjet + Subleading_bjet;
      }
      //
      met_vector.SetPtEtaPhiE(met->met(), 0, met->phi(), met->met());
      double met_sig = m_met_sig.get(*met, sys);
      double sumet = met->sumet();
      //
      if (bjets->size()>=2 && leptons.size()>=2) {
	bbll = ll + bb;
	bbllmet = bbll + met_vector;
	b1l1 = Leading_bjet+Leading_lep;
	b2l1 = Subleading_bjet+Leading_lep;
	b1l2 = Leading_bjet+Subleading_lep;
	b2l2 = Subleading_bjet+Subleading_lep;
	double m_b1l1 = b1l1.M();
	double m_b2l1 = b2l1.M();
	double m_b1l2 = b1l2.M();
	double m_b2l2 = b2l2.M();
	m_bl = std::min(std::max(m_b1l1, m_b2l2), std::max(m_b2l1, m_b1l2));
      }
      //
      //Retrieve NW
      double nw_weight = m_NW.get(*event,sys);
      double mT2_bb = m_mT2bb.get(*event,sys);
      //
      double pt_b1 = -99;
      double pt_b2 = -99;
      double mll = -99;
      double pTll = -99;
      double dRll = -99;
      double phill = -99;
      double pTbb = -99;
      double mbb = -99;
      double dRbb = -99;
      double MET = -99;
      double MET_sumMET = -99;
      double MET_sig = -99;
      double mT_Lepton1_Met = -99;
      double mT_Lepton2_Met = -99;
      double HT2 = -99;
      double var_mbbll = -99;
      double var_mbbllmet = -99;
      double HT2r = -99;
      double pt_l1 = -99;
      double pt_l2 = -99;
      double var_mbl = -99;
      double var_mT2_bb = -99;
      double var_met_ptll = -99;
      //
      pt_b1 = bjets->size() >= 1 ?  Leading_bjet.Pt() : 0.;
      pt_b2 = bjets->size() >= 2 ?  Subleading_bjet.Pt() : 0.;
      pt_l1 = leptons.size()>=1 ? Leading_lep.Pt() : 0.;
      pt_l2 = leptons.size()>=2 ? Subleading_lep.Pt() : 0.;
      mll = leptons.size()>=2 ? ll.M() : 0.;
      pTll = leptons.size()>=2 ? ll.Pt() : 0.;
      phill = leptons.size()>=2 ? ll.Phi() : 0.;
      dRll = leptons.size()>=2 ? Leading_lep.DeltaR(Subleading_lep) : 0.;
      pTbb = bjets->size()>=2 ? bb.Pt() : 0.;
      mbb = bjets->size()>=2 ? bb.M() : 0.;
      dRbb = bjets->size()>=2 ? Leading_bjet.DeltaR(Subleading_bjet) : 0.;
      MET = met_vector.Pt();
      MET_sig = met_sig;
      MET_sumMET = sumet;
      var_mbbll = (bjets->size()>=2 && leptons.size()>=2) ? bbll.M() : 0.;
      var_mbbllmet = (bjets->size()>=2 && leptons.size()>=2) ? bbllmet.M() : 0.;
      HT2 = (bjets->size()>=2 && leptons.size()>=2) ? (met_vector + ll).Perp() + bb.Perp() : 0.;
      HT2r = (bjets->size()>=2 && leptons.size()>=2) ? HT2 / (met->met() + Leading_lep.Pt() + Subleading_lep.Pt() + Leading_bjet.Pt() + Subleading_bjet.Pt()) : 0.;
      mT_Lepton1_Met = leptons.size()>=1 ? TMath::Sqrt(2 * met->met() * Leading_lep.Pt() * (1 - TMath::Cos(Leading_lep.DeltaPhi(met_vector)))) : 0.;
      mT_Lepton2_Met = leptons.size()>=2 ? TMath::Sqrt(2 * met->met() * Subleading_lep.Pt() * (1 - TMath::Cos(Subleading_lep.DeltaPhi(met_vector)))) : 0.;
      var_mbl = (bjets->size()>=2 && leptons.size()>=2) ? m_bl : 0.;
      var_mT2_bb = bjets->size()>=2 ? mT2_bb : 0.;
      var_met_ptll = leptons.size()>=2 ? std::abs(MET + pTll) : 0.;
      //
      pnn_inputs_SR1["bbll_Jet_b2_pt_NOSYS"] = pt_b2;
      pnn_inputs_SR1["bbll_mll_NOSYS"] = mll;
      pnn_inputs_SR1["bbll_Phill_NOSYS"] = phill;
      pnn_inputs_SR1["bbll_pTbb_NOSYS"] = pTbb;
      pnn_inputs_SR1["bbll_mbb_NOSYS"] = mbb;
      pnn_inputs_SR1["bbll_dRbb_NOSYS"] = dRbb;
      pnn_inputs_SR1["met_met_NOSYS"] = MET;
      pnn_inputs_SR1["met_sumet_NOSYS"] = MET_sumMET;
      pnn_inputs_SR1["bbll_mbbll_NOSYS"] = var_mbbll;
      pnn_inputs_SR1["bbll_mT_Lepton1_Met_NOSYS"] = mT_Lepton1_Met;
      pnn_inputs_SR1["bbll_mT_Lepton2_Met_NOSYS"] = mT_Lepton2_Met;
      pnn_inputs_SR1["bbll_mbbllmet_NOSYS"] = var_mbbllmet;
      pnn_inputs_SR1["bbll_HT2_NOSYS"] = HT2;
      pnn_inputs_SR1["bbll_NW_neutrinoweight_NOSYS"] = nw_weight;
      //
      pnn_inputs_SR2["bbll_Lepton1_pt_NOSYS"] = pt_l1;
      pnn_inputs_SR2["bbll_Lepton2_pt_NOSYS"] = pt_l2;
      pnn_inputs_SR2["bbll_Jet_b1_pt_NOSYS"] = pt_b1;
      pnn_inputs_SR2["bbll_Jet_b2_pt_NOSYS"] = pt_b2;
      pnn_inputs_SR2["bbll_mll_NOSYS"] = mll;
      pnn_inputs_SR2["bbll_pTll_NOSYS"] = pTll;
      pnn_inputs_SR2["bbll_dRll_NOSYS"] = dRll;
      pnn_inputs_SR2["bbll_pTbb_NOSYS"] = pTbb;
      pnn_inputs_SR2["bbll_mbb_NOSYS"] = mbb;
      pnn_inputs_SR2["bbll_dRbb_NOSYS"] = dRbb;
      pnn_inputs_SR2["met_met_NOSYS"] = MET;
      pnn_inputs_SR2["bbll_MET_sig_NOSYS"] = MET_sig;
      pnn_inputs_SR2["bbll_mbl_NOSYS"] = var_mbl;
      pnn_inputs_SR2["bbll_mT2_bb_NOSYS"] = var_mT2_bb;
      pnn_inputs_SR2["bbll_mbbll_NOSYS"] = var_mbbll;
      pnn_inputs_SR2["bbll_mT_Lepton1_Met_NOSYS"] = mT_Lepton1_Met;
      pnn_inputs_SR2["bbll_mT_Lepton2_Met_NOSYS"] = mT_Lepton2_Met;
      pnn_inputs_SR2["bbll_mbbllmet_NOSYS"] = var_mbbllmet;
      pnn_inputs_SR2["bbll_HT2r_NOSYS"] = HT2r;
      pnn_inputs_SR2["bbll_NW_neutrinoweight_NOSYS"] = nw_weight;
      pnn_inputs_SR2["abs_met_ptll"] = var_met_ptll;      
      //
      uint64_t eventNum = event->eventNumber();
      std::map<std::string, std::map<std::string, double> > in_nodes_SR1;
      std::map<std::string, std::map<std::string, double> > in_nodes_SR2;
      double PNN_Score_SR1 = -99;
      double PNN_Score_SR2 = -99;
      //
      for (const auto& mX : m_mX_values) {
	pnn_inputs_SR1["mass_scaled"] = mX;
	pnn_inputs_SR2["mass_scaled"] = mX;
	in_nodes_SR1["node_0"] = pnn_inputs_SR1;
	in_nodes_SR2["node_0"] = pnn_inputs_SR2;
	//
	std::string branch_name_SR1 =  m_PNN_ScoreLabel_SR1+ "_X" + std::to_string(int(mX));
	std::string branch_name_SR2 =  m_PNN_ScoreLabel_SR2+ "_X" + std::to_string(int(mX));
	if (year>=2015 && year <=2018){
	  if (eventNum % 3 == 0){
	    PNN_Score_SR1 = m_model_PNN_setC_SR1_Run2->compute(in_nodes_SR1)["out_0"];
	    PNN_Score_SR2 = m_model_PNN_setC_SR2_Run2->compute(in_nodes_SR2)["out_0"];
	  }
	  else if (eventNum % 3 == 1){
	    PNN_Score_SR1 = m_model_PNN_setA_SR1_Run2->compute(in_nodes_SR1)["out_0"];
	    PNN_Score_SR2 = m_model_PNN_setA_SR2_Run2->compute(in_nodes_SR2)["out_0"];
	  }
	  else if (eventNum % 3 == 2){
	    PNN_Score_SR1 = m_model_PNN_setB_SR1_Run2->compute(in_nodes_SR1)["out_0"];
	    PNN_Score_SR2 = m_model_PNN_setB_SR2_Run2->compute(in_nodes_SR2)["out_0"];
	  }
	}
	else if(year>=2022 && year<=2023){
	  if (eventNum % 3 == 0){
	    PNN_Score_SR1 = m_model_PNN_setC_SR1_Run3->compute(in_nodes_SR1)["out_0"];
	    PNN_Score_SR2 = m_model_PNN_setC_SR2_Run3->compute(in_nodes_SR2)["out_0"];
	  }
	  else if (eventNum % 3 == 1){
	    PNN_Score_SR1 = m_model_PNN_setA_SR1_Run3->compute(in_nodes_SR1)["out_0"];
	    PNN_Score_SR2 = m_model_PNN_setA_SR2_Run3->compute(in_nodes_SR2)["out_0"];
	  }
	  else if (eventNum % 3 == 2){
	    PNN_Score_SR1 = m_model_PNN_setB_SR1_Run3->compute(in_nodes_SR1)["out_0"];
	    PNN_Score_SR2 = m_model_PNN_setB_SR2_Run3->compute(in_nodes_SR2)["out_0"];
	  }
	}
	m_Fbranches.at(branch_name_SR1).set(*event, PNN_Score_SR1, sys);
	m_Fbranches.at(branch_name_SR2).set(*event, PNN_Score_SR2, sys);
      }
    }
    return StatusCode::SUCCESS;
  }
}
