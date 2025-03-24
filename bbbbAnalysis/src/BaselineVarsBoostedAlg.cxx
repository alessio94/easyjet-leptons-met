/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Frederic Renner

#include "BaselineVarsBoostedAlg.h"
#include "AthContainers/AuxElement.h"

namespace HH4B
{
  BaselineVarsBoostedAlg ::BaselineVarsBoostedAlg(const std::string &name,
                                                  ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode BaselineVarsBoostedAlg ::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("      BaselineVarsBoostedAlg     \n");
    ATH_MSG_INFO("*********************************\n");

    ATH_CHECK (m_LargeRJetHandle.initialize(m_systematicsList));
    if(m_UseVBFRNN){
      ATH_CHECK (m_RNNjetBoostedHandle.initialize(m_systematicsList));
    }
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    ATH_CHECK(m_R10TruthLabel.initialize(m_systematicsList, m_LargeRJetHandle));
    ATH_CHECK(m_GN2Xv01_phbb.initialize(m_systematicsList, m_LargeRJetHandle));
    ATH_CHECK(m_GN2Xv01_phcc.initialize(m_systematicsList, m_LargeRJetHandle));
    ATH_CHECK(m_GN2Xv01_pqcd.initialize(m_systematicsList, m_LargeRJetHandle));
    ATH_CHECK(m_GN2Xv01_ptop.initialize(m_systematicsList, m_LargeRJetHandle));
    ATH_CHECK(m_Tau2_wta.initialize(m_systematicsList, m_LargeRJetHandle));
    ATH_CHECK(m_Tau3_wta.initialize(m_systematicsList, m_LargeRJetHandle));

    // make decorators
    for (const std::string &string_var: m_Fvars) {
      CP::SysWriteDecorHandle<float> var {string_var+"_%SYS%", this};
      m_Fdecos.emplace(string_var, var);
      ATH_CHECK (m_Fdecos.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }

    // Initialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsBoostedAlg ::execute()
  {
    for (const auto& sys : m_systematicsList.systematicsVector()) {
      // container we read in
      const xAOD::EventInfo *eventInfo = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (eventInfo, sys));

      const xAOD::JetContainer *largeRjets = nullptr;
      ANA_CHECK (m_LargeRJetHandle.retrieve (largeRjets, sys));
      std::size_t n_largeRjets = largeRjets->size();

      const xAOD::JetContainer *RNNJets= nullptr;
      if(m_UseVBFRNN) {
        ANA_CHECK (m_RNNjetBoostedHandle.retrieve (RNNJets, sys));
      }

      // set defaults
      for (const std::string& var : m_Fvars) {
        m_Fdecos.at(var).set(*eventInfo, -1, sys);
      }

      // large jet sector
      if (n_largeRjets >= 2)
      {
        TLorentzVector h1_v4 = largeRjets->at(0)->p4();
        TLorentzVector h2_v4 = largeRjets->at(1)->p4();

        m_Fdecos.at("boosted_h1_m").set(*eventInfo, h1_v4.M(), sys);
        m_Fdecos.at("boosted_h1_pt").set(*eventInfo, h1_v4.Pt(), sys);
        m_Fdecos.at("boosted_h1_eta").set(*eventInfo, h1_v4.Eta(), sys);
        m_Fdecos.at("boosted_h1_phi").set(*eventInfo, h1_v4.Phi(), sys);
        m_Fdecos.at("boosted_h1_E").set(*eventInfo, h1_v4.E(), sys);

        m_Fdecos.at("boosted_h2_m").set(*eventInfo, h2_v4.M(), sys);
        m_Fdecos.at("boosted_h2_pt").set(*eventInfo, h2_v4.Pt(), sys);
        m_Fdecos.at("boosted_h2_eta").set(*eventInfo, h2_v4.Eta(), sys);
        m_Fdecos.at("boosted_h2_phi").set(*eventInfo, h2_v4.Phi(), sys);
        m_Fdecos.at("boosted_h2_E").set(*eventInfo, h2_v4.E(), sys);

        TLorentzVector hh_v4 = h1_v4 + h2_v4;
        m_Fdecos.at("boosted_hh_m").set(*eventInfo, hh_v4.M(), sys);
        m_Fdecos.at("boosted_hh_pt").set(*eventInfo, hh_v4.Pt(), sys);
        m_Fdecos.at("boosted_hh_delta_eta").set(*eventInfo, h1_v4.Eta() - h2_v4.Eta(), sys);
        m_Fdecos.at("boosted_hh_delta_phi").set(*eventInfo, h1_v4.DeltaPhi(h2_v4), sys);

        for (std::size_t i=0; i<2; i++){

          std::string prefix = "boosted_h"+std::to_string(i+1);

          float tau32 = m_Tau3_wta.get(*largeRjets->at(i),sys) /
	    m_Tau2_wta.get(*largeRjets->at(i),sys);
          m_Fdecos.at(prefix+"_Tau32_wta").set(*eventInfo, tau32, sys);

          float phbb_score = m_GN2Xv01_phbb.get(*largeRjets->at(i), sys);
          float pqcd_score = m_GN2Xv01_pqcd.get(*largeRjets->at(i), sys);
          float phcc_score = m_GN2Xv01_phcc.get(*largeRjets->at(i), sys);
          float ptop_score = m_GN2Xv01_ptop.get(*largeRjets->at(i), sys);

          m_Fdecos.at(prefix+"_GN2Xv01_phbb").set(*eventInfo, phbb_score, sys);
          m_Fdecos.at(prefix+"_GN2Xv01_pqcd").set(*eventInfo, pqcd_score, sys);
          m_Fdecos.at(prefix+"_GN2Xv01_phcc").set(*eventInfo, phcc_score, sys);
          m_Fdecos.at(prefix+"_GN2Xv01_ptop").set(*eventInfo, ptop_score, sys);

          float hbb_disc = calculateGN2Xv01_disc(phbb_score, pqcd_score, phcc_score, ptop_score);
          m_Fdecos.at(prefix+"_GN2Xv01_disc").set(*eventInfo, hbb_disc, sys);

          if (m_isMC){
            int truthLabel_i = m_R10TruthLabel.get(*largeRjets->at(i), sys);
            m_Fdecos.at(prefix+"_truthLabel").set(*eventInfo, truthLabel_i, sys);
          }
        }

        if (m_UseVBFRNN){
          std::string RNNJets_names = "boosted_RNNJets";

          std::vector<float> rnn_jet_m(2, -99.0f);
          std::vector<float> rnn_jet_pt(2, -99.0f);
          std::vector<float> rnn_jet_eta(2, -99.0f);
          std::vector<float> rnn_jet_phi(2, -99.0f);
          for(unsigned int j=0; j<std::min(size_t(2),RNNJets->size()); j++) {
            const xAOD::Jet* RNNJet = RNNJets->at(j);
            rnn_jet_m[j] = RNNJet->m();
            rnn_jet_pt[j] = RNNJet->pt();
            rnn_jet_eta[j] = RNNJet->eta();
            rnn_jet_phi[j] = RNNJet->phi();
          }

          for(unsigned int j=0; j<2; j++) {
            std::string prefix = "Jet"+std::to_string(j+1);

            m_Fdecos.at(RNNJets_names+"_"+prefix+"_m").set(*eventInfo, rnn_jet_m[j], sys);
            m_Fdecos.at(RNNJets_names+"_"+prefix+"_pt").set(*eventInfo, rnn_jet_pt[j], sys);
            m_Fdecos.at(RNNJets_names+"_"+prefix+"_eta").set(*eventInfo, rnn_jet_eta[j], sys);
            m_Fdecos.at(RNNJets_names+"_"+prefix+"_phi").set(*eventInfo, rnn_jet_phi[j], sys);
          }
        } // end of RNN jets
      }
    }
    return StatusCode::SUCCESS;
  }
}
