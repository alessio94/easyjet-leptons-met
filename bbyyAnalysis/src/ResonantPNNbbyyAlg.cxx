/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ResonantPNNbbyyAlg.h"
#include <AthContainers/ConstDataVector.h>
#include "lwtnn/parse_json.hh"
#include <fstream>
#include "PathResolver/PathResolver.h"
#include <AthenaKernel/Units.h>

namespace SHBBYY
{
  ResonantPNNbbyyAlg::ResonantPNNbbyyAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {}

  StatusCode ResonantPNNbbyyAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("       ResonantPNNbbyyAlg       \n");
    ATH_MSG_INFO("*********************************\n");

    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    ATH_CHECK (m_photonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    //Low Mass Grid
    for (const auto& mX_mS : m_mX_mS_pairs) {
      double mX = mX_mS.first;
      double mS = mX_mS.second;
      std::string branch_name =  m_PNN_ScoreLabel+ "_X" + std::to_string(int(mX)) + "_S" + std::to_string(int(mS));
      m_mX_mS_all_pairs.push_back(std::make_pair(mX, mS));
      m_Fvarnames.push_back(branch_name);
    }

    //High Mass Grid
    for (const auto& mX : m_mX_values) {
      for (const auto& mS : m_mS_values) {
        if (mX > 500 and mS < 70) { continue; }
        if (mX - mS <= 125) { continue; }
        std::string branch_name =  m_PNN_ScoreLabel+ "_X" + std::to_string(int(mX)) + "_S" + std::to_string(int(mS));
        m_mX_mS_all_pairs.push_back(std::make_pair(mX, mS));
        m_Fvarnames.push_back(branch_name);
      }
    }

    for (const auto& mX : m_mX_1bjet) {
      std::string branch_name =  m_PNN_1bjet_ScoreLabel+ "_X" + std::to_string(int(mX));
      m_Fvarnames.push_back(branch_name);
    }

    // Intialise syst-aware output decorators

    for (const std::string &string_var: m_Fvarnames) {
      CP::SysWriteDecorHandle<float> var {string_var+"_%SYS%", this};
      m_Fbranches.emplace(string_var, var);
      ATH_CHECK (m_Fbranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    std::ifstream input_stream_pnn_cv1(PathResolverFindCalibFile("bbyyAnalysis/PNN_Tight_SH_bbyy_cv1.json"));
    std::ifstream input_stream_pnn_cv2(PathResolverFindCalibFile("bbyyAnalysis/PNN_Tight_SH_bbyy_cv2.json"));
    std::ifstream input_stream_pnn_cv3(PathResolverFindCalibFile("bbyyAnalysis/PNN_Tight_SH_bbyy_cv3.json"));
    m_model_PNN_cv1 = std::unique_ptr<lwt::LightweightGraph> (new lwt::LightweightGraph(lwt::parse_json_graph(input_stream_pnn_cv1)));
    m_model_PNN_cv2 = std::unique_ptr<lwt::LightweightGraph> (new lwt::LightweightGraph(lwt::parse_json_graph(input_stream_pnn_cv2)));
    m_model_PNN_cv3 = std::unique_ptr<lwt::LightweightGraph> (new lwt::LightweightGraph(lwt::parse_json_graph(input_stream_pnn_cv3)));

    std::ifstream input_stream_pnn_1bjet_cv1(PathResolverFindCalibFile("bbyyAnalysis/PNN_1bjet_SH_bbyy_cv1_pt.json"));
    std::ifstream input_stream_pnn_1bjet_cv2(PathResolverFindCalibFile("bbyyAnalysis/PNN_1bjet_SH_bbyy_cv2_pt.json"));
    std::ifstream input_stream_pnn_1bjet_cv3(PathResolverFindCalibFile("bbyyAnalysis/PNN_1bjet_SH_bbyy_cv3_pt.json"));
    m_model_PNN_1bjet_cv1 = std::unique_ptr<lwt::LightweightGraph> (new lwt::LightweightGraph(lwt::parse_json_graph(input_stream_pnn_1bjet_cv1)));
    m_model_PNN_1bjet_cv2 = std::unique_ptr<lwt::LightweightGraph> (new lwt::LightweightGraph(lwt::parse_json_graph(input_stream_pnn_1bjet_cv2)));
    m_model_PNN_1bjet_cv3 = std::unique_ptr<lwt::LightweightGraph> (new lwt::LightweightGraph(lwt::parse_json_graph(input_stream_pnn_1bjet_cv3)));
  

    return StatusCode::SUCCESS;
  }

  StatusCode ResonantPNNbbyyAlg::execute()
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

      int eventNum = event->eventNumber();

      // initialize
      TLorentzVector H_bb(0.,0.,0.,0.);
      TLorentzVector H_yy(0.,0.,0.,0.);
      TLorentzVector HH(0.,0.,0.,0.);
      TLorentzVector y1(0.,0.,0.,0.);
      TLorentzVector y2(0.,0.,0.,0.);
      TLorentzVector b1(0.,0.,0.,0.);
      TLorentzVector b2(0.,0.,0.,0.);
      TLorentzVector yyb(0.,0.,0.,0.);

      for (const std::string &string_var: m_Fvarnames) {
        m_Fbranches.at(string_var).set(*event, -99., sys);
      }

      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      for(const xAOD::Jet* jet : *jets) {
        // check if jet is btagged
        if (WPgiven) {
          if (m_isBtag.get(*jet, sys)) bjets->push_back(jet);
        }
      }

      // photon sector
      if (photons->size() >= 1) {
        const xAOD::Photon* ph1 = photons->at(0);
        y1 = ph1->p4();
      }

      if (photons->size() >= 2) {
        const xAOD::Photon* ph2 = photons->at(1);
        y2 = ph2->p4();
        // Build the H(yy) candidate
        H_yy = y1 + y2;
      }

      // b-jet sector
      if (bjets->size() >= 1) {
        b1 = bjets->at(0)->p4();
      }

      yyb = H_yy + b1;
      if (bjets->size() >= 2) {
        b2 = bjets->at(1)->p4();
        // Build the H(bb) candidate
        H_bb = b1 + b2;
      }
      // Build the HH candidate
      if (photons->size() >= 2 && bjets->size() >= 2) {
        HH = H_yy + H_bb;
      }

      double pt_b1, myy, myyj,mjj, myyjj = -99;
      double myyjj_star, myyj_star = -99;
      double PNN_Score = -99;
      double PNN_1bjet_Score = -99;

      pt_b1 = bjets->size() >= 1 ? bjets->at(0)->pt() : 0.;
      myy = photons->size() >= 2 ? H_yy.M(): -99;
      myyj = bjets->size() >= 1 && photons->size() >= 2 ? yyb.M() : -99;
      mjj = bjets->size() >= 2 ? H_bb.M(): -99;
      myyjj = photons->size() >= 2 && bjets->size() >= 2 ? HH.M(): -99;
      myyjj_star = photons->size() >= 2 && bjets->size() >= 2 ? (myyjj - (myy - 125.0*Athena::Units::GeV)) : -99; //m_yyjj_mod
      myyj_star = bjets->size() >= 1 && photons->size() >= 2 ? myyj - (myy - 125.0*Athena::Units::GeV) : -99; //m_yyj_mod  


      for (const auto& mX_mS : m_mX_mS_all_pairs) {
        double mX = mX_mS.first;
        double mS = mX_mS.second;
        std::map<std::string, std::map<std::string, double>> pnn_inputs;
        pnn_inputs["input_node"] = {{"m_jj", mjj/Athena::Units::GeV},
                                         {"m_yyjj_mod", myyjj_star/Athena::Units::GeV},
                                         {"X_mass", mX}, {"S_mass", mS}};
        if (eventNum % 3 == 0) { PNN_Score = m_model_PNN_cv2->compute(pnn_inputs)["BinaryClassificationOutputName"]; }
        if (eventNum % 3 == 1) { PNN_Score = m_model_PNN_cv3->compute(pnn_inputs)["BinaryClassificationOutputName"]; }
        if (eventNum % 3 == 2) { PNN_Score = m_model_PNN_cv1->compute(pnn_inputs)["BinaryClassificationOutputName"]; }
        std::string branch_name =  m_PNN_ScoreLabel+ "_X" + std::to_string(int(mX)) + "_S" + std::to_string(int(mS));
        m_Fbranches.at(branch_name).set(*event, PNN_Score, sys);  
      }

      //PNN 1bjet
      for (const auto& mX : m_mX_1bjet) {
        std::map<std::string, std::map<std::string, double>> pnn_1bjet_inputs;
        pnn_1bjet_inputs["input_node"] = {{"bjet1_pt", pt_b1/Athena::Units::GeV},
                                          {"m_yyj_mod", myyj_star/Athena::Units::GeV},
                                          {"X_mass", mX}};
        if (eventNum % 3 == 0) { PNN_1bjet_Score = m_model_PNN_1bjet_cv2->compute(pnn_1bjet_inputs)["BinaryClassificationOutputName"]; }
        if (eventNum % 3 == 1) { PNN_1bjet_Score = m_model_PNN_1bjet_cv3->compute(pnn_1bjet_inputs)["BinaryClassificationOutputName"]; }
        if (eventNum % 3 == 2) { PNN_1bjet_Score = m_model_PNN_1bjet_cv1->compute(pnn_1bjet_inputs)["BinaryClassificationOutputName"]; }
        std::string branch_name =  m_PNN_1bjet_ScoreLabel+ "_X" + std::to_string(int(mX));
        m_Fbranches.at(branch_name).set(*event, PNN_1bjet_Score, sys); 
      }
    }

    return StatusCode::SUCCESS;
  }
}
