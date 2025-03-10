/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author 

#include "BaselineVarsZbbyCalibAlg.h"

namespace XBBCALIB
{
  BaselineVarsZbbyCalibAlg::BaselineVarsZbbyCalibAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  { }

  StatusCode BaselineVarsZbbyCalibAlg::initialize()
  {
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_lrjetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK (m_photonHandle.initialize(m_systematicsList));

    ATH_CHECK(m_GN2Xv01_phbb.initialize(m_systematicsList, m_lrjetHandle));
    ATH_CHECK(m_GN2Xv01_phcc.initialize(m_systematicsList, m_lrjetHandle));
    ATH_CHECK(m_GN2Xv01_pqcd.initialize(m_systematicsList, m_lrjetHandle));
    ATH_CHECK(m_GN2Xv01_ptop.initialize(m_systematicsList, m_lrjetHandle));


    for(const auto& wp: m_GN2X_wps)
      m_GN2X_wp_Handles.emplace_back("xbb_select_GN2Xv01_" + wp, this);

    for(auto& handle : m_GN2X_wp_Handles)
      ATH_CHECK(handle.initialize(m_systematicsList, m_lrjetHandle));


    // Intialise syst-aware output decorators
    for (const std::string &var : m_floatVariables) {
      CP::SysWriteDecorHandle<float> whandle{var+"_%SYS%", this};
      // std::cout<< "In Base Vars Zbb y: "<<var <<std::endl;
      m_Fbranches.emplace(var, whandle);
      ATH_CHECK (m_Fbranches.at(var).initialize(m_systematicsList, m_eventHandle));
    }

    for (const std::string &var : m_intVariables){
      ATH_MSG_DEBUG("initializing integer variable: " << var);
      CP::SysWriteDecorHandle<int> whandle{var+"_%SYS%", this};
      m_Ibranches.emplace(var, whandle);
      ATH_CHECK(m_Ibranches.at(var).initialize(m_systematicsList, m_eventHandle));
    };

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsZbbyCalibAlg::execute()
  {
    //Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *lrjets = nullptr;
      ANA_CHECK (m_lrjetHandle.retrieve (lrjets, sys));

      const xAOD::PhotonContainer *photons = nullptr;
      ANA_CHECK (m_photonHandle.retrieve(photons, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      for (const std::string &string_var: m_floatVariables) {
        m_Fbranches.at(string_var).set(*event, -99., sys);
      }

      for (const auto& var: m_intVariables) {
        m_Ibranches.at(var).set(*event, -99, sys);
      }
      // Calculate vars

      // selected Probe Jet ;
      if (lrjets->size() >= 1)
      {
        const xAOD::Jet* largeJet = lrjets->at(0);
        m_Fbranches.at("Zcand_pt").set(*event, largeJet->pt(), sys);  
        m_Fbranches.at("Zcand_eta").set(*event, largeJet->eta(), sys);  
        m_Fbranches.at("Zcand_phi").set(*event, largeJet->phi(), sys);  
        m_Fbranches.at("Zcand_m").set(*event, largeJet->m(), sys);  
    
    //construct GN2X score
        float GN2Xv01_phbb = m_GN2Xv01_phbb.get(*largeJet, sys);
        float GN2Xv01_phcc = m_GN2Xv01_phcc.get(*largeJet, sys);
        float GN2Xv01_pqcd = m_GN2Xv01_pqcd.get(*largeJet, sys);
        float GN2Xv01_ptop = m_GN2Xv01_ptop.get(*largeJet, sys);
        // float wtag_score = m_WTag_score.get(*largeJet, sys);
        m_Fbranches.at("Zcand_GN2Xv01_phbb").set(*event, GN2Xv01_phbb, sys);
        m_Fbranches.at("Zcand_GN2Xv01_pqcd").set(*event, GN2Xv01_phcc, sys);
        m_Fbranches.at("Zcand_GN2Xv01_phcc").set(*event, GN2Xv01_pqcd, sys);
        m_Fbranches.at("Zcand_GN2Xv01_ptop").set(*event, GN2Xv01_ptop, sys);
        for(unsigned int wp=0; wp<m_GN2X_wps.size(); wp++)
          {
	    int pass_GN2X = m_GN2X_wp_Handles.at(wp).get(*largeJet, sys);
            m_Ibranches.at("Zcand_Pass_GN2X_"+m_GN2X_wps[wp]).set(*event, pass_GN2X, sys);
          }

       
      }
      m_Ibranches.at("lrjets_n").set(*event, lrjets->size(), sys);
      // This should be ok, as exactly one photon req.
      if (photons->size() >= 1)
      {
        m_Fbranches.at("photon_pt").set(*event, photons->at(0)->pt(), sys);  
        m_Fbranches.at("photon_eta").set(*event, photons->at(0)->eta(), sys);  
        m_Fbranches.at("photon_phi").set(*event, photons->at(0)->phi(), sys);  
        m_Fbranches.at("photon_m").set(*event, photons->at(0)->m(), sys);  
      }
      m_Ibranches.at("photons_n").set(*event, photons->size(), sys);


      // Calculate vars
      

    }
    return StatusCode::SUCCESS;
  }

}



