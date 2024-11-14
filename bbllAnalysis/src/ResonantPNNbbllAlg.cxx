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
    //Retrieve NW
    //
    CP::SysReadDecorHandle<float> rhandle{std::string("NW_neutrinoweight")+"_%SYS%", this};
    m_NW = rhandle;
    ATH_CHECK (m_NW.initialize(m_systematicsList, m_eventHandle));
    //ATH_CHECK(m_reco_pairings.retrieve());
    //
    ATH_CHECK (m_systematicsList.initialize());
    //
    std::ifstream input_stream_pnn_cv1(PathResolverFindCalibFile("bbllAnalysis/PNN_setA.json"));
    std::ifstream input_stream_pnn_cv2(PathResolverFindCalibFile("bbllAnalysis/PNN_setB.json"));
    std::ifstream input_stream_pnn_cv3(PathResolverFindCalibFile("bbllAnalysis/PNN_setC.json"));
    m_model_PNN_setA = std::make_unique<lwt::LightweightGraph> (lwt::parse_json_graph(input_stream_pnn_cv1));
    m_model_PNN_setB = std::make_unique<lwt::LightweightGraph> (lwt::parse_json_graph(input_stream_pnn_cv2));
    m_model_PNN_setC = std::make_unique<lwt::LightweightGraph> (lwt::parse_json_graph(input_stream_pnn_cv3));
    //
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
      //
      // Electron sector
      const xAOD::Electron* ele0 = nullptr;
      const xAOD::Electron* ele1 = nullptr;
      
      for(unsigned int i=0; i<std::min(size_t(2),electrons->size()); i++){
	std::string prefix = "Electron"+std::to_string(i+1);
	const xAOD::Electron* ele = electrons->at(i);
	if(i==0) ele0 = ele;
	else if(i==1) ele1 = ele;
      }
      // Muon sector
      const xAOD::Muon* mu0 = nullptr;
      const xAOD::Muon* mu1 = nullptr;
      for(unsigned int i=0; i<std::min(size_t(2),muons->size()); i++){
	std::string prefix = "Muon"+std::to_string(i+1);
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
	std::string prefix = "Lepton"+std::to_string(i+1);
	TLorentzVector tlv = leptons[i].first->p4();
	if(i==0) Leading_lep = tlv;
	else if(i==1) Subleading_lep = tlv;
      }    
      if(leptons.size()>=2){
	ll = Leading_lep + Subleading_lep;
      }
      //b-jet sector
      for(unsigned int i=0; i<std::min(size_t(2),bjets->size()); i++){
	std::string prefix = "Jet_b"+std::to_string(i+1);
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
      }
      //
      //Retrieve NW
      double nw_weight = m_NW.get(*event,sys);
      double pt_b1, pt_b2, eta_b1, eta_b2 = -99;
      double mll, pTll, dRll, etall, phill = -99;
      double pTbb, mbb, dRbb, etabb = -99;
      double MET, MET_sumMET, MET_sig, MET_phi = -99;
      double mT_Lepton1_Met, mT_Lepton2_Met, HT2, var_mbbll, var_mbbllmet = -99;
      double HT2r = -99;      
      //
      pt_b1 = bjets->size() >= 1 ? bjets->at(0)->pt() : 0.;
      pt_b2 = bjets->size() >= 2 ? bjets->at(1)->pt() : 0.;
      eta_b1 = bjets->size() >= 1 ? bjets->at(0)->eta() : 0.;
      eta_b2 = bjets->size() >= 2 ? bjets->at(1)->eta() : 0.;
      mll = leptons.size()>=2 ? ll.M() : 0.;
      pTll = leptons.size()>=2 ? ll.Pt() : 0.;
      etall = leptons.size()>=2 ? ll.Eta() : 0.;
      phill = leptons.size()>=2 ? ll.Phi() : 0.;
      dRll = leptons.size()>=2 ? Leading_lep.DeltaR(Subleading_lep) : 0.;
      pTbb = bjets->size()>=2 ? bb.Pt() : 0.;
      mbb = bjets->size()>=2 ? bb.M() : 0.;
      etabb = bjets->size()>=2 ? bb.Eta() : 0.;
      dRbb = bjets->size()>=2 ? Leading_bjet.DeltaR(Subleading_bjet) : 0.;
      MET = met_vector.Pt();
      MET_sig = met_sig;
      MET_phi = met_vector.Phi();
      MET_sumMET = sumet;
      var_mbbll = (bjets->size()>=2 && leptons.size()>=2) ? bbll.M() : 0.;
      var_mbbllmet = (bjets->size()>=2 && leptons.size()>=2) ? bbllmet.M() : 0.;
      HT2 = (bjets->size()>=2 && leptons.size()>=2) ? (met_vector + ll).Perp() + bb.Perp() : 0.;
      HT2r = (bjets->size()>=2 && leptons.size()>=2) ? HT2 / (met->met() + Leading_lep.Pt() + Subleading_lep.Pt() + Leading_bjet.Pt() + Subleading_bjet.Pt()) : 0.;
      mT_Lepton1_Met = leptons.size()>=1 ? TMath::Sqrt(2 * met->met() * Leading_lep.Pt() * (1 - TMath::Cos(Leading_lep.DeltaPhi(met_vector)))) : 0.;
      mT_Lepton2_Met = leptons.size()>=2 ? TMath::Sqrt(2 * met->met() * Subleading_lep.Pt() * (1 - TMath::Cos(Subleading_lep.DeltaPhi(met_vector)))) : 0.;  
      //
      pnn_inputs["bbll_Etabb_NOSYS"] = etabb;
      pnn_inputs["bbll_Etall_NOSYS"] = etall;
      pnn_inputs["bbll_HT2_NOSYS"] = HT2/Athena::Units::GeV;
      pnn_inputs["bbll_HT2r_NOSYS"] = HT2r/Athena::Units::GeV;
      pnn_inputs["bbll_Jet_b1_eta_NOSYS"] = eta_b1;
      pnn_inputs["bbll_Jet_b1_pt_NOSYS"] = pt_b1/Athena::Units::GeV;
      pnn_inputs["bbll_Jet_b2_eta_NOSYS"] = eta_b2;
      pnn_inputs["bbll_Jet_b2_pt_NOSYS"] = pt_b2/Athena::Units::GeV;
      pnn_inputs["bbll_MET_sig_NOSYS"] = MET_sig/Athena::Units::GeV;
      pnn_inputs["bbll_NW_neutrinoweight_NOSYS"] = nw_weight;
      pnn_inputs["bbll_Phill_NOSYS"] = phill;
      pnn_inputs["bbll_dRbb_NOSYS"] = dRbb;
      pnn_inputs["bbll_dRll_NOSYS"] = dRll;
      pnn_inputs["bbll_mT_Lepton1_Met_NOSYS"] = mT_Lepton1_Met/Athena::Units::GeV;
      pnn_inputs["bbll_mT_Lepton2_Met_NOSYS"] = mT_Lepton2_Met/Athena::Units::GeV;
      pnn_inputs["bbll_mbb_NOSYS"] = mbb/Athena::Units::GeV;
      pnn_inputs["bbll_mbbll_NOSYS"] = var_mbbll/Athena::Units::GeV;
      pnn_inputs["bbll_mbbllmet_NOSYS"] = var_mbbllmet/Athena::Units::GeV;
      pnn_inputs["bbll_mll_NOSYS"] = mll/Athena::Units::GeV;
      pnn_inputs["bbll_pTbb_NOSYS"] = pTbb/Athena::Units::GeV;
      pnn_inputs["bbll_pTll_NOSYS"] = pTll/Athena::Units::GeV;
      pnn_inputs["met_met_NOSYS"] = MET/Athena::Units::GeV;
      pnn_inputs["met_phi_NOSYS"] = MET_phi;
      pnn_inputs["met_sumet_NOSYS"] = MET_sumMET/Athena::Units::GeV;
      //
      uint64_t eventNum = event->eventNumber();
      std::map<std::string, std::map<std::string, double> > in_nodes;
      double PNN_Score = -99;
      //
      for (const auto& mX : m_mX_values) {
	pnn_inputs["mass_scaled"] = mX;
	in_nodes["node_0"] = pnn_inputs;
	//
	if (eventNum % 3 == 0){PNN_Score = m_model_PNN_setC->compute(in_nodes)["out_0"];}
	else if (eventNum % 3 == 1){PNN_Score = m_model_PNN_setA->compute(in_nodes)["out_0"];}
	else if (eventNum % 3 == 2){PNN_Score = m_model_PNN_setB->compute(in_nodes)["out_0"];}
	//
	std::string branch_name =  m_PNN_ScoreLabel+ "_X" + std::to_string(int(mX));
	m_Fbranches.at(branch_name).set(*event, PNN_Score, sys);
      }
    }
    return StatusCode::SUCCESS;
  }
}
