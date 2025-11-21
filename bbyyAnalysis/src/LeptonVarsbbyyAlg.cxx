/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LeptonVarsbbyyAlg.h"

namespace HHBBYY
{
  LeptonVarsbbyyAlg::LeptonVarsbbyyAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {}

  StatusCode LeptonVarsbbyyAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("       LeptonVarsbbyyAlg       \n");
    ATH_MSG_INFO("*********************************\n");

    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    for (const std::string &string_var: m_intVariables) {
      CP::SysWriteDecorHandle<int> var {string_var+"_%SYS%", this};
      m_Ibranches.emplace(string_var, var);
      ATH_CHECK (m_Ibranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }

    for (const std::string &string_var: m_floatVariables) {
      CP::SysWriteDecorHandle<float> var {string_var+"_%SYS%", this};
      m_Fbranches.emplace(string_var, var);
      ATH_CHECK (m_Fbranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }
  
    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode LeptonVarsbbyyAlg::execute()
  {
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      // In case of special Higgs sample, run only on NOSYS
      if (!m_doSystematics && sys.name()!="") continue;

      // container we read in
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::MissingETContainer* met_container = nullptr;
      ANA_CHECK (m_metHandle.retrieve(met_container, sys));

      m_Ibranches.at("nLeptons").set(*event, electrons->size() + muons->size(), sys);

      // Electron variables
      float m_ee_os = -99.;
      float pT_ee_os = -99.;
      if(electrons->size()>=2){
        const xAOD::Electron* e1 = (*electrons)[0];
        const xAOD::Electron* e2 = (*electrons)[1];
        if (e1->charge()*e2->charge()<0) {
          m_ee_os = (e1->p4() + e2->p4()).M();
          pT_ee_os = (e1->p4() + e2->p4()).Pt();
        }
      }

      // Muon variables
      float m_mumu_os = -99.;
      float pT_mumu_os = -99.;
      if(muons->size()>=2){
        const xAOD::Muon* m1 = (*muons)[0];
        const xAOD::Muon* m2 = (*muons)[1];
        if (m1->charge()*m2->charge()<0) {
          m_mumu_os = (m1->p4() + m2->p4()).M();
          pT_mumu_os = (m1->p4() + m2->p4()).Pt();
        }
      }
      


      // PT (MET + lepton) 
      float pTlepMET = -99.;
      float mTlepMET = 0; // bug in default value used for the training
      const xAOD::MissingET* met_tst = (*met_container)["Final"];
      if ((electrons->size()>0 || muons->size()>0) && met_tst != nullptr) {
        TLorentzVector metvec;
        metvec.SetPtEtaPhiM(met_tst->met(), 0, met_tst->phi(), 0);
        bool useEl = (electrons->size() > 0);
        if (useEl && (muons->size() > 0)) {
          useEl = (electrons->at(0)->pt() > muons->at(0)->pt());
        }
        if (useEl)  { 
          pTlepMET = (metvec + electrons->at(0)->p4()).Pt(); 
          mTlepMET = sqrt(2. * electrons->at(0)->pt() * met_tst->met() * (1. - cos(electrons->at(0)->phi() - met_tst->phi()))); 
        }
        else        { 
          pTlepMET = (metvec + muons->at(0)->p4()).Pt(); 
          mTlepMET = sqrt(2. * muons->at(0)->pt() * met_tst->met() * (1. - cos(muons->at(0)->phi() - met_tst->phi())));
        }
      }
      

      // Flag that tells whether MET(TST) - MET (hardVertexTST) > 30 GeV
      int metTST_HVTST_flag = 0;
      const xAOD::MissingET* met_hvtst = (*met_container)["HardVertexTST"];
      if (met_tst != nullptr && met_hvtst != nullptr) {
        if ( (met_tst->met() - met_hvtst->met()) > 30.) metTST_HVTST_flag = 1;
      }

      if (m_do_HHbbyy_Hyy_Analysis)
      {
        m_Ibranches.at("nElectrons").set(*event, electrons->size(), sys);
        m_Ibranches.at("nMuons").set(*event, muons->size(), sys);

        m_Fbranches.at("m_ee_os").set(*event, m_ee_os, sys);
        m_Fbranches.at("pT_ee_os").set(*event, pT_ee_os, sys);
        
        m_Fbranches.at("m_mumu_os").set(*event, m_mumu_os, sys);
        m_Fbranches.at("pT_mumu_os").set(*event, pT_mumu_os, sys);

        // max pT of opposite sign dilepton system (e or mu)
        m_Fbranches.at("pT_ll_os_max").set(*event, std::max(pT_ee_os, pT_mumu_os), sys);
        m_Fbranches.at("pTlepMET").set(*event, pTlepMET, sys);
        m_Fbranches.at("mTlepMET").set(*event, mTlepMET, sys);
        m_Fbranches.at("metTST_HVTST_flag").set(*event, metTST_HVTST_flag, sys);

      }
    }

    return StatusCode::SUCCESS;
  }

}
