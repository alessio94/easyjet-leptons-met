/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "SemiLepSelectorAlg.h"
#include <AthenaKernel/Units.h>

#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>

namespace VBSHIGGS{

  SemiLepSelectorAlg::SemiLepSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator) : EL::AnaAlgorithm(name, pSvcLocator){
    

  }
  StatusCode SemiLepSelectorAlg::initialize(){
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("     SemiLepSelectorAlg      \n");
    ATH_MSG_INFO("*********************************\n");
    return StatusCode::SUCCESS;
  }
  StatusCode SemiLepSelectorAlg::execute(){
    ATH_MSG_INFO("EXECUTE  -----  SemiLepSelectorAlg  -----    \n");

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector()){
      // Retrive inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));
      
    }
    return StatusCode::SUCCESS;
  }
  StatusCode SemiLepSelectorAlg::finalize(){

    return StatusCode::SUCCESS;
  }
  
}//name space