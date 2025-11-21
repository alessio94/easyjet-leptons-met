/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "PhotonJetVarsAlg.h"

namespace HHBBYY
{
    PhotonJetVarsAlg::PhotonJetVarsAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {}

  StatusCode PhotonJetVarsAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("       PhotonJetVarsAlg       \n");
    ATH_MSG_INFO("*********************************\n");
    ATH_CHECK (m_photonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
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

  StatusCode PhotonJetVarsAlg::execute()
  {
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
        // In case of special Higgs sample, run only on NOSYS
        if (!m_doSystematics && sys.name()!="") continue;

        // container we read in
        const xAOD::EventInfo *event = nullptr;
        ANA_CHECK (m_eventHandle.retrieve (event, sys));
    
        // Photons
        const xAOD::PhotonContainer* photons = nullptr;
        ANA_CHECK (m_photonHandle.retrieve(photons, sys));

        // Jets
        const xAOD::JetContainer* jets = nullptr;
        ANA_CHECK (m_jetHandle.retrieve(jets, sys));

        const std::size_t nPhotons = photons->size();
        const std::size_t nJets    = jets->size();

        const xAOD::Photon* ph1 = nullptr;
        const xAOD::Photon* ph2 = nullptr;
        const xAOD::Jet*    j1  = nullptr;
        const xAOD::Jet*    j2  = nullptr;

        if (nPhotons >= 1) ph1 = (*photons)[0];
        if (nPhotons >= 2) ph2 = (*photons)[1];
        if (nJets    >= 1) j1  = (*jets)[0];
        if (nJets    >= 2) j2  = (*jets)[1];

        TLorentzVector vYY(0., 0., 0., 0.);
        TLorentzVector vJJ(0., 0., 0., 0.);
        TLorentzVector vYYJJ(0., 0., 0., 0.);
        
        if (ph1 && ph2) {
          vYY = ph1->p4() + ph2->p4();
        }
        if (j1 && j2) {
          vJJ = j1->p4() + j2->p4();
        }
        if (ph1 && ph2 && j1 && j2) {
          vYYJJ = vYY + vJJ;
        }
        // pT(yyjj) and pT(yyjj) with pT(jet)>30 GeV
        // and m(yyjj) 
        float pT_yyjj = -99.;
        float pT_yyjj_30 = -99.;
        float m_yyjj = -99.;
        if (nJets >= 2 && nPhotons >= 2) {
            pT_yyjj = vYYJJ.Pt();
            if ( (j2->pt() > 30000.) ) {
                pT_yyjj_30 = pT_yyjj;
            }
            m_yyjj = vYYJJ.M(); // Calculate m(yyjj)
        }
        m_Fbranches.at("pT_yyjj").set(*event, pT_yyjj, sys);
        m_Fbranches.at("pT_yyjj_30").set(*event, pT_yyjj_30, sys);
        m_Fbranches.at("m_yyjj").set(*event, m_yyjj, sys);

        // pT(yyj) and m(yyj)
        float pT_yyj = -99.;
        float m_yyj = -99.;
        if (nJets >= 1 && nPhotons >= 2) {
            pT_yyj = (vYY + j1->p4()).Pt();
            m_yyj = (vYY + j1->p4()).M(); // Calculate m(yyj)
        }
        m_Fbranches.at("pT_yyj").set(*event, pT_yyj, sys);
        m_Fbranches.at("m_yyj").set(*event, m_yyj, sys);

        // DeltaPhi(yy,jj) and DeltaY(yy,jj)
        float Dphi_yy_jj = -99.;
        float Dy_yy_jj = -99.;
        if (nJets >= 2 && nPhotons >= 2) {
            Dphi_yy_jj = fabs(vYY.DeltaPhi(vJJ));
            Dy_yy_jj = fabs(vYY.Rapidity() - vJJ.Rapidity());
        }
        m_Fbranches.at("Dphi_yy_jj").set(*event, Dphi_yy_jj, sys);
        m_Fbranches.at("Dy_yy_jj").set(*event, Dy_yy_jj, sys);

        // min DeltaR between photons and jets
        float DRmin_y_j = -99.;
        float dR2min = 99.0, dR2 = 0.0, eta = 0.0, phi = 0.0;
        for (auto photon : *photons) {
          eta = photon->eta(); 
          phi = photon->phi();
  
          for (auto jet : *jets) {
            dR2 = xAOD::P4Helpers::deltaR2(*jet, eta, phi, false); // is deltaR2 a thing?
            if (dR2 < dR2min) { dR2min = dR2; }
          }
        }
        if (dR2min != 99) { DRmin_y_j = sqrt(dR2min); }  
        m_Fbranches.at("DRmin_y_j").set(*event, DRmin_y_j, sys);
    

      float cosTS_yyjj = -99.;
      if (jets->size() >= 2 && photons->size() >= 2) {
          TLorentzVector vH, vZ, vg;
          vg  = j1->p4()  + j2->p4();
          vZ  = ph1->p4() + ph2->p4();
          vH  = vZ + vg;
          
          TVector3 boost = -vH.BoostVector();
          vH.Boost(boost);
          vZ.Boost(boost);
          vg.Boost(boost);
          
          TLorentzVector q, qbar;     
          q.SetPxPyPzE(0, 0, vH.M() / 2, vH.M() / 2);     
          qbar.SetPxPyPzE(0, 0, -vH.M() / 2, vH.M() / 2);

          cosTS_yyjj = (q - qbar).Dot(vZ) / (vH.M() * vZ.P());
      }
      m_Fbranches.at("cosTS_yyjj").set(*event, cosTS_yyjj, sys);
      
      float Zepp = -99.;
      if (nJets >= 2 && nPhotons >= 2) {
        Zepp = vYY.Eta() - (j1->eta() + j2->eta()) / 2.0;
      }
      m_Fbranches.at("Zepp").set(*event, Zepp, sys);

      // Eta of VBF jet 
      // Where the VBF jet is the most forward jet in the event
      float fwdJet_eta = -99.;
      float maxEta = 0;
      TLorentzVector vJetMaxEta;

      for (auto jet : *jets) {
        if (abs(jet->eta()) > maxEta) {
          vJetMaxEta = jet->p4();
          maxEta = abs(jet->eta());
        }
      }
      if (vJetMaxEta.Pt() > 0) { fwdJet_eta = vJetMaxEta.Eta(); }
      m_Fbranches.at("fwdJet_eta").set(*event, fwdJet_eta, sys);

      // Eta of (VBF jet + yy system)
      // Where the VBF jet is the most forward jet in the event
      float m_fwdJet_yy = -99.;
      if (vJetMaxEta.Pt() > 0 && nPhotons >= 2) {
        m_fwdJet_yy = (vYY + vJetMaxEta).M();
      }
      m_Fbranches.at("m_fwdJet_yy").set(*event, m_fwdJet_yy, sys);
    }

    return StatusCode::SUCCESS;
  }

}