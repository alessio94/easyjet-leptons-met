/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "BoostedVarsbbyyAlg.h"
#include "AthContainers/AuxElement.h"

namespace HHBBYY
{
  BoostedVarsbbyyAlg::BoostedVarsbbyyAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {}

  StatusCode BoostedVarsbbyyAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("       BoostedVarsbbyyAlg       \n");
    ATH_MSG_INFO("*********************************\n");

    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));

    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));


    // Intialise syst-aware output decorators

    for (const std::string &string_var: m_Fvarnames) {
      CP::SysWriteDecorHandle<float> var {string_var+"_%SYS%", this};
      m_Fbranches.emplace(string_var, var);
      ATH_CHECK (m_Fbranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }

    for (const std::string &string_var: m_Ivarnames) {
      CP::SysWriteDecorHandle<int> var {string_var+"_%SYS%", this};
      m_Ibranches.emplace(string_var, var);
      ATH_CHECK (m_Ibranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    } 

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode BoostedVarsbbyyAlg::execute()
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
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      static const SG::AuxElement::ConstAccessor<int>  R10TruthLabel_R22v1("R10TruthLabel_R22v1");
      static const SG::AuxElement::ConstAccessor<float>  GN2Xv01_phbb("GN2Xv01_phbb");
      static const SG::AuxElement::ConstAccessor<float>  GN2Xv01_pqcd("GN2Xv01_pqcd");
      static const SG::AuxElement::ConstAccessor<float>  GN2Xv01_phcc("GN2Xv01_phcc");
      static const SG::AuxElement::ConstAccessor<float>  GN2Xv01_ptop("GN2Xv01_ptop");

      for (const std::string &string_var: m_Fvarnames) {
        m_Fbranches.at(string_var).set(*event, -99., sys);
      }
      
      for (const std::string &string_var: m_Ivarnames) {
        m_Ibranches.at(string_var).set(*event, -99, sys);
      }

      // large jet sector
      for (std::size_t i=0; i<std::min(jets->size(),(std::size_t)2); i++){
        TLorentzVector j = jets->at(i)->p4();

        float phbb_score = GN2Xv01_phbb(*jets->at(i));
        float pqcd_score = GN2Xv01_pqcd(*jets->at(i));
        float phcc_score = GN2Xv01_phcc(*jets->at(i));
        float ptop_score = GN2Xv01_ptop(*jets->at(i));

        std::string prefix = "LargeRJet"+std::to_string(i+1);
        m_Fbranches.at(prefix+"_pt").set(*event, j.Pt(), sys);
        m_Fbranches.at(prefix+"_eta").set(*event, j.Eta(), sys);
        m_Fbranches.at(prefix+"_phi").set(*event, j.Phi(), sys);
        m_Fbranches.at(prefix+"_E").set(*event, j.E(), sys);
        m_Fbranches.at(prefix+"_m").set(*event, j.M(), sys);

        m_Fbranches.at(prefix+"_GN2Xv01_phbb").set(*event, phbb_score, sys);
        m_Fbranches.at(prefix+"_GN2Xv01_pqcd").set(*event, pqcd_score, sys);
        m_Fbranches.at(prefix+"_GN2Xv01_phcc").set(*event, phcc_score, sys);
        m_Fbranches.at(prefix+"_GN2Xv01_ptop").set(*event, ptop_score, sys);

        if (m_isMC){
          int truthLabel_j = R10TruthLabel_R22v1(*jets->at(i));
          m_Ibranches.at(prefix+"_truthLabel").set(*event, truthLabel_j, sys);
        }

      }


      m_Ibranches.at("nLargeRJets").set(*event, jets->size(), sys);
    }

    return StatusCode::SUCCESS;
  }

}
