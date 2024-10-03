/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

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
    static const SG::AuxElement::ConstAccessor<int>    R10TruthLabel_R22v1("R10TruthLabel_R22v1");
    static const SG::AuxElement::ConstAccessor<float>  GN2Xv01_phbb("GN2Xv01_phbb");
    static const SG::AuxElement::ConstAccessor<float>  GN2Xv01_pqcd("GN2Xv01_pqcd");
    static const SG::AuxElement::ConstAccessor<float>  GN2Xv01_phcc("GN2Xv01_phcc");
    static const SG::AuxElement::ConstAccessor<float>  GN2Xv01_ptop("GN2Xv01_ptop");

    static const SG::AuxElement::ConstAccessor<float>  Tau2_wta("Tau2_wta");
    static const SG::AuxElement::ConstAccessor<float>  Tau3_wta("Tau3_wta");

    for (const auto& sys : m_systematicsList.systematicsVector()) {
      // container we read in
      const xAOD::EventInfo *eventInfo = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (eventInfo, sys));

      const xAOD::JetContainer *largeRjets = nullptr;
      ANA_CHECK (m_LargeRJetHandle.retrieve (largeRjets, sys));
      std::size_t n_largeRjets = largeRjets->size();

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

        float tau32, phbb_score, pqcd_score, phcc_score, ptop_score, hbb_disc;

        for (std::size_t i=0; i<2; i++){
          std::string prefix = "boosted_h"+std::to_string(i+1);

          tau32 = Tau3_wta(*largeRjets->at(i))/Tau2_wta(*largeRjets->at(i));
          m_Fdecos.at(prefix+"_Tau32_wta").set(*eventInfo, tau32, sys);

          phbb_score = GN2Xv01_phbb(*largeRjets->at(i));
          pqcd_score = GN2Xv01_pqcd(*largeRjets->at(i));
          phcc_score = GN2Xv01_phcc(*largeRjets->at(i));
          ptop_score = GN2Xv01_ptop(*largeRjets->at(i));

          m_Fdecos.at(prefix+"_GN2Xv01_phbb").set(*eventInfo, phbb_score, sys);
          m_Fdecos.at(prefix+"_GN2Xv01_pqcd").set(*eventInfo, pqcd_score, sys);
          m_Fdecos.at(prefix+"_GN2Xv01_phcc").set(*eventInfo, phcc_score, sys);
          m_Fdecos.at(prefix+"_GN2Xv01_ptop").set(*eventInfo, ptop_score, sys);

          hbb_disc = calculateGN2Xv01_disc(phbb_score, pqcd_score, phcc_score, ptop_score);
          m_Fdecos.at(prefix+"_GN2Xv01_disc").set(*eventInfo, hbb_disc, sys);

          if (m_isMC){
            int truthLabel_i = R10TruthLabel_R22v1(*largeRjets->at(i));
            m_Fdecos.at(prefix+"_truthLabel").set(*eventInfo, truthLabel_i, sys);
          }
        }
      }
    }
    return StatusCode::SUCCESS;
  }
}
