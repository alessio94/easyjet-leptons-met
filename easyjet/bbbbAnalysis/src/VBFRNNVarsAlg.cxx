/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Frederic Renner

#include "VBFRNNVarsAlg.h"
#include "AthContainers/AuxElement.h"

namespace HH4B
{
  VBFRNNVarsAlg ::VBFRNNVarsAlg(const std::string &name,
                                                  ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode VBFRNNVarsAlg ::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("      VBFRNNVarsAlg     \n");
    ATH_MSG_INFO("*********************************\n");

    if(m_UseVBFRNN){
      ATH_CHECK (m_RNNjetBoostedHandle.initialize(m_systematicsList));
    }
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    // make decorators
    for (const std::string &string_var: m_Fvars.value()) {
      CP::SysWriteDecorHandle<float> var {string_var+"_%SYS%", this};
      m_Fdecos.emplace(string_var, var);
      ATH_CHECK (m_Fdecos.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }

    // Initialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode VBFRNNVarsAlg ::execute()
  {
    for (const auto& sys : m_systematicsList.systematicsVector()) {
      // container we read in
      const xAOD::EventInfo *eventInfo = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (eventInfo, sys));

      const xAOD::JetContainer *RNNJets_boosted= nullptr;
      if(m_UseVBFRNN) {
        ANA_CHECK (m_RNNjetBoostedHandle.retrieve (RNNJets_boosted, sys));
      }

      // set defaults
      for (const std::string& var : m_Fvars) {
        m_Fdecos.at(var).set(*eventInfo, -99, sys);
      }

      // RNN jets
      if (m_UseVBFRNN){
        std::string RNNJets_names = "boosted_RNNJets";
        
        for(unsigned int j=0; j<std::min(size_t(2),RNNJets_boosted->size()); j++) {
          const xAOD::Jet* RNNJet = RNNJets_boosted->at(j);
          std::string prefix = "Jet"+std::to_string(j+1);
          
          m_Fdecos.at(RNNJets_names+"_"+prefix+"_m").set(*eventInfo, RNNJet->m(), sys);
          m_Fdecos.at(RNNJets_names+"_"+prefix+"_pt").set(*eventInfo, RNNJet->pt(), sys);
          m_Fdecos.at(RNNJets_names+"_"+prefix+"_eta").set(*eventInfo, RNNJet->eta(), sys);
          m_Fdecos.at(RNNJets_names+"_"+prefix+"_phi").set(*eventInfo, RNNJet->phi(), sys);
        }
      } // end of RNN jets
    }
    return StatusCode::SUCCESS;
  }
}
