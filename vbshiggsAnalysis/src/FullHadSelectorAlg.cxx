/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FullHadSelectorAlg.h"
#include <AthenaKernel/Units.h>


namespace VBSHIGGS{

  FullHadSelectorAlg::FullHadSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator) : EL::AnaAlgorithm(name, pSvcLocator){
    

  }
  StatusCode FullHadSelectorAlg::initialize(){
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("     FullHadSelectorAlg      \n");
    ATH_MSG_INFO("*********************************\n");

    return StatusCode::SUCCESS;
  }
  StatusCode FullHadSelectorAlg::execute(){
    ATH_MSG_INFO("EXECUTE  -----  FullHadSelectorAlg  -----    \n");

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector()){
      
    }
    return StatusCode::SUCCESS;
  }
  StatusCode FullHadSelectorAlg::finalize(){

    return StatusCode::SUCCESS;
  }
  
}//name space