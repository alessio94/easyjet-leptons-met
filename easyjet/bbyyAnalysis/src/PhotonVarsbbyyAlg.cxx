/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "PhotonVarsbbyyAlg.h"

#include "AthContainers/AuxElement.h"

namespace HHBBYY
{
  PhotonVarsbbyyAlg::PhotonVarsbbyyAlg(const std::string &name,
                                       ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {}

  StatusCode PhotonVarsbbyyAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("       PhotonVarsbbyyAlg       \n");
    ATH_MSG_INFO("*********************************\n");

    ATH_CHECK (m_bbyyPhotonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_isEMTight.initialize(m_systematicsList, m_bbyyPhotonHandle));
    
    ATH_CHECK (m_selected_ph.initialize(m_systematicsList, m_bbyyPhotonHandle));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    
    if(m_isMC){
      ATH_CHECK (m_photonHandle.initialize(m_systematicsList));
      m_ph_SF = CP::SysReadDecorHandle<float>("effSF_"+m_photonWPName+"_%SYS%", this);
      ATH_CHECK (m_ph_SF.initialize(m_systematicsList, m_photonHandle, SG::AllowEmpty));
    }
    
    for (const std::string &string_var: m_floatVariables) {
      CP::SysWriteDecorHandle<float> var { string_var+"_%SYS%", this};
      m_Fbranches.emplace(string_var, var);
      ATH_CHECK (m_Fbranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }

    for (const std::string &string_var: m_intVariables) {
      CP::SysWriteDecorHandle<int> var { string_var+"_%SYS%", this};
      m_Ibranches.emplace(string_var, var);
      ATH_CHECK (m_Ibranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }
    
    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode PhotonVarsbbyyAlg::execute()
  {
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      // In case of special Higgs sample, run only on NOSYS
      if (!m_doSystematics && sys.name()!="") continue;

      // container we read in
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));
      
      const xAOD::PhotonContainer *photons = nullptr;
      ANA_CHECK (m_bbyyPhotonHandle.retrieve (photons, sys));

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

      for (const std::string &string_var: m_floatVariables) {
        m_Fbranches.at(string_var).set(*event, -99., sys);
      }

      for (const std::string &string_var: m_intVariables) {
          m_Ibranches.at(string_var).set(*event, -99, sys);
      }

      // photon sector
      std::vector<const xAOD::Photon*> sel_photons = {ph1, ph2};
      for(unsigned int i=0; i<2; i++){
        const xAOD::Photon* ph = sel_photons[i];
        if(!ph) break;
        std::string prefix = "Photon"+std::to_string(i+1);
        TLorentzVector y = ph->p4();
        m_Fbranches.at(prefix+"_pt").set(*event, y.Pt(), sys);
        m_Fbranches.at(prefix+"_eta").set(*event, y.Eta(), sys);
        m_Fbranches.at(prefix+"_phi").set(*event, y.Phi(), sys);
        m_Fbranches.at(prefix+"_E").set(*event, y.E(), sys);
        m_Ibranches.at(prefix+"_isEMTight").set(*event, m_isEMTight.get(*ph, sys), sys);
        if(m_isMC){
          m_Fbranches.at(prefix+"_effSF").set(*event, m_ph_SF.get(*ph, sys), sys);
        }
      }
      
      TLorentzVector H_yy(0.,0.,0.,0.);
      double dRyy = -99.;

      if (ph1 && ph2) {
        // Build the H(yy) candidate
        H_yy = ph1->p4() + ph2->p4();

        m_Fbranches.at("myy").set(*event, H_yy.M(), sys);
        m_Fbranches.at("pTyy").set(*event, H_yy.Pt(), sys);
        m_Fbranches.at("Etayy").set(*event, H_yy.Eta(), sys);
        m_Fbranches.at("Phiyy").set(*event, H_yy.Phi(), sys);
        
        dRyy = ph1->p4().DeltaR(ph2->p4());
        m_Fbranches.at("dRyy").set(*event, dRyy, sys);

        m_Fbranches.at("Photon1_ptOvermyy").set(*event, ph1->pt()/H_yy.M(), sys);
        m_Fbranches.at("Photon2_ptOvermyy").set(*event, ph2->pt()/H_yy.M(), sys);

        if (m_do_HHbbyy_Hyy_Analysis)
        {
          // yAbs_yy
          float yAbs_yy = std::fabs(H_yy.Rapidity());
          m_Fbranches.at("yAbs_yy").set(*event, yAbs_yy, sys);
          
          // Dy_y_y
          float Dy_y_y = std::fabs(ph1->rapidity() - ph2->rapidity());
          m_Fbranches.at("Dy_y_y").set(*event, Dy_y_y, sys);
          
          // pTt_yy
          TLorentzVector ph1_tlv = ph1->p4();
          TLorentzVector ph2_tlv = ph2->p4();
          float pTt_yy =  std::fabs(ph1_tlv.Px() * ph2_tlv.Py() - ph2_tlv.Px() * ph1_tlv.Py()) / (ph1_tlv - ph2_tlv).Pt() * 2.0;
          
          m_Fbranches.at("pTt_yy").set(*event, pTt_yy, sys);
          
          // phiStar_yy
          float phiStar_yy = tan((TMath::Pi() - fabs(ph1_tlv.DeltaPhi(ph2_tlv))) / 2.0) *
            sqrt(1.0 - pow(tanh((ph1_tlv.Eta() - ph2_tlv.Eta()) / 2.0), 2.0));
          m_Fbranches.at("phiStar_yy").set(*event, phiStar_yy, sys);
          
        }
      }

      m_Ibranches.at("nPhotons").set(*event, photons->size(), sys);
    }
    
    return StatusCode::SUCCESS;
  }
}
