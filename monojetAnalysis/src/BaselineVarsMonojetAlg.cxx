/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "BaselineVarsMonojetAlg.h"

#include "AthContainers/AuxElement.h"
#include <AthContainers/ConstDataVector.h>
#include "xAODBTagging/BTaggingUtilities.h"
#include "xAODBTagging/BTaggingContainer.h"
#include "xAODBTagging/BTaggingAuxContainer.h"


#include "TLorentzVector.h"

namespace MONOJET
{
  BaselineVarsMonojetAlg::BaselineVarsMonojetAlg(const std::string& name,
    ISvcLocator* pSvcLocator)
    : AthHistogramAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode BaselineVarsMonojetAlg::initialize()
  {
    // Read syst-aware input handles
    ATH_CHECK(m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK(m_largejetHandle.initialize(m_systematicsList));
    ATH_CHECK(m_metHandle.initialize(m_systematicsList));
    ATH_CHECK(m_eventHandle.initialize(m_systematicsList));



    if (!m_isBtag.empty()) {
      ATH_CHECK(m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    if (!m_PCBT.empty()) {
      ATH_CHECK (m_PCBT.initialize(m_systematicsList, m_jetHandle));
    }

    ATH_CHECK(m_METSig.initialize(m_systematicsList, m_metHandle));

    if (m_isMC) {
      ATH_CHECK(m_truthFlav.initialize(m_systematicsList, m_jetHandle));
    }

    // Intialise syst-aware output decorators
    for (const std::string& var : m_floatVariables) {
      CP::SysWriteDecorHandle<float> whandle{ var + "_%SYS%", this };
      m_Fbranches.emplace(var, whandle);
      ATH_CHECK(m_Fbranches.at(var).initialize(m_systematicsList, m_eventHandle));
    }

    for (const std::string& var : m_intVariables) {
      ATH_MSG_DEBUG("initializing integer variable: " << var);
      CP::SysWriteDecorHandle<int> whandle{ var + "_%SYS%", this };
      m_Ibranches.emplace(var, whandle);
      ATH_CHECK(m_Ibranches.at(var).initialize(m_systematicsList, m_eventHandle));
    };

    ATH_CHECK (m_GN2Xv01_phbb.initialize(m_systematicsList, m_largejetHandle));
    ATH_CHECK (m_GN2Xv01_phcc.initialize(m_systematicsList, m_largejetHandle));
    ATH_CHECK (m_GN2Xv01_pqcd.initialize(m_systematicsList, m_largejetHandle));
    ATH_CHECK (m_GN2Xv01_ptop.initialize(m_systematicsList, m_largejetHandle));

    //jet substruture
    for(auto jss : m_JSS_list){
      m_JSS.emplace(jss, CP::SysReadDecorHandle<float>(jss, this));
      ATH_CHECK(m_JSS.at(jss).initialize(m_systematicsList, m_largejetHandle));
    }

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK(m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsMonojetAlg::execute()
  {

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {

      // Retrieve inputs
      const xAOD::EventInfo* event = nullptr;
      ANA_CHECK(m_eventHandle.retrieve(event, sys));

      const xAOD::JetContainer* jets = nullptr;
      ANA_CHECK(m_jetHandle.retrieve(jets, sys));

      const xAOD::JetContainer *largeJets  = nullptr;
      ANA_CHECK (m_largejetHandle.retrieve (largeJets , sys));

      const xAOD::MissingETContainer* metCont = nullptr;
      ANA_CHECK(m_metHandle.retrieve(metCont, sys));
      const xAOD::MissingET* met = (*metCont)["Final"];
      if (!met) {
        ATH_MSG_ERROR("Could not retrieve MET");
        return StatusCode::FAILURE;
      }

      for (const std::string& string_var : m_floatVariables) {
        m_Fbranches.at(string_var).set(*event, -99., sys);
      }

      for (const auto& var : m_intVariables) {
        m_Ibranches.at(var).set(*event, -99, sys);
      }

      TLorentzVector met_vector (0,0,0,0);
      met_vector.SetPtEtaPhiM(met->met(),
                              0,
                              met->phi(),
                               0);

      // Count jets
      int n_jets = jets->size();
      int nCentralJets = 0;
      int nForwardJets = 0;
      int n_largeJets = largeJets->size();
      std::vector<int> nBJets(m_btagWPs.size(), 0); 

      float sum_pT_CentralJets = 0.;
      float sum_pT_ForwardJets = 0.;

      float DeltaPhi_MET_jets = 1000.;
      float DeltaPhi_MET_largeJets = 1000.;

      for(const xAOD::Jet* jet : *jets) {
          // count central jets vs forward
          if (std::abs(jet->eta())< m_max_eta_central_jet){ 
            nCentralJets++;
            sum_pT_CentralJets += jet->pt();
          }
          else { 
            nForwardJets++;
            sum_pT_ForwardJets += jet->pt();
          }

          // count bjets
          if (!m_PCBT.empty()) {
              int pcbt = m_PCBT.get(*jet, sys);

              for (size_t i = 0; i < m_btagWPs.size(); ++i) {
                  if (pcbt > static_cast<int>(i)) {
                      nBJets[i]++;
                  }
              }
          }

          //get DeltaPhi min	
	        TLorentzVector smallJet_vector(0,0,0,0);
	        smallJet_vector.SetPtEtaPhiE(jet->pt(),
                               jet->eta(),
                               jet->phi(),
                               jet->e());
          float a = abs(met_vector.DeltaPhi(smallJet_vector));
          if(a < DeltaPhi_MET_jets) DeltaPhi_MET_jets = a;
      }

      
      float sum_pT_LargeRJets = 0.;
      for (int i = 0; i < n_largeJets; i++){
	      sum_pT_LargeRJets += largeJets->at(i)->pt();
	
	      TLorentzVector largeJet_vector(0,0,0,0);
	      largeJet_vector.SetPtEtaPhiE(largeJets->at(i)->pt(),
                               largeJets->at(i)->eta(),
                               largeJets->at(i)->phi(),
                               largeJets->at(i)->e());
        
        float b = abs(met_vector.DeltaPhi(largeJet_vector));
        if(b < DeltaPhi_MET_largeJets) DeltaPhi_MET_largeJets = b;

      }

      // write to ttree

      m_Ibranches.at("nCentralJets").set(*event, nCentralJets, sys);
      m_Ibranches.at("nForwardJets").set(*event, nForwardJets, sys);
      m_Ibranches.at("nLargeRJets").set(*event, n_largeJets, sys);

      for (size_t i = 0; i < m_btagWPs.size(); ++i) {
          const std::string& wp = m_btagWPs[i];
          m_Ibranches.at("nBJets" + wp).set(*event, nBJets[i], sys);
      }


      m_Fbranches.at("sum_pT_CentralJets").set(*event,sum_pT_CentralJets,sys);
      m_Fbranches.at("sum_pT_ForwardJets").set(*event,sum_pT_ForwardJets,sys);
      m_Fbranches.at("sum_pT_LargeRJets").set(*event,sum_pT_LargeRJets,sys);

      m_Fbranches.at("DeltaPhi_MET_jets").set(*event,DeltaPhi_MET_jets,sys);
      m_Fbranches.at("DeltaPhi_MET_largeJets").set(*event,DeltaPhi_MET_largeJets,sys);

      //MET Significance
      float METSig = m_METSig.get(*met, sys);
      m_Fbranches.at("METSig").set(*event, METSig, sys);


      //jet sector
      for (int i = 0; i < std::min(n_jets, (int)m_Small_R_jet_amount); i++) {

        const xAOD::Jet* jet = jets->at(i);
        std::string prefix = "Jet"+std::to_string(i+1);

        m_Fbranches.at(prefix+"_pt").set(*event, jet->pt(), sys);
        m_Fbranches.at(prefix+ "_eta").set(*event, jet->eta(), sys);
        m_Fbranches.at(prefix+ "_phi").set(*event, jet->phi(), sys);
        m_Fbranches.at(prefix+ "_E").set(*event, jet->e(), sys);

        if(!m_PCBT.empty())
          m_Ibranches.at(prefix+"_pcbt").set(*event,m_PCBT.get(*jet,sys),sys);
      }

      // large jet sector
      for (int i=0; i<std::min(n_largeJets,(int)m_Large_R_jet_amount); i++){

        const xAOD::Jet* largeJet = largeJets->at(i);
        std::string prefix = "LargeRJet"+std::to_string(i+1)+"_";

        m_Fbranches.at(prefix+"pt").set(*event, largeJet->pt(), sys);
        m_Fbranches.at(prefix+"eta").set(*event, largeJet->eta(), sys);
        m_Fbranches.at(prefix+"phi").set(*event, largeJet->phi(), sys);
        m_Fbranches.at(prefix+"E").set(*event, largeJet->e(), sys);
        m_Fbranches.at(prefix+"m").set(*event, largeJet->m(), sys);

        // tagging
        float phbb = m_GN2Xv01_phbb.get(*largeJet, sys);
        float phcc = m_GN2Xv01_phcc.get(*largeJet, sys);
        float pqcd = m_GN2Xv01_pqcd.get(*largeJet, sys);
        float ptop = m_GN2Xv01_ptop.get(*largeJet, sys);
        
        m_Fbranches.at(prefix+"GN2Xv01_phbb").set(*event, phbb, sys);
        m_Fbranches.at(prefix+"GN2Xv01_phcc").set(*event, phcc, sys);
        m_Fbranches.at(prefix+"GN2Xv01_pqcd").set(*event, pqcd, sys);
        m_Fbranches.at(prefix+"GN2Xv01_ptop").set(*event, ptop, sys);

        // jet substrucure
        for(const auto & jss : m_JSS_list){
          float jss_var = m_JSS.at(jss).get(*largeJet, sys);
          m_Fbranches.at(prefix+jss).set(*event, jss_var, sys);
        }        
      }

    }
    return StatusCode::SUCCESS;
  }
}
