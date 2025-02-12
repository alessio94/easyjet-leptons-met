/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "BosonTaggerAlg.h"

namespace VBSVV4q{

    BosonTaggerAlg::BosonTaggerAlg(const std::string &name,
                                                ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator){

    }

    StatusCode BosonTaggerAlg::initialize(){

      ATH_MSG_INFO("******************************\n");
      ATH_MSG_INFO("       BosonTaggerAlg         \n");
      ATH_MSG_INFO("******************************\n");

      // Read input handles
      ATH_CHECK (m_LargeRJetsHandle.initialize());
      
      // boson tagger
      ATH_CHECK(m_MLTagger.retrieve());

      return StatusCode::SUCCESS;
    }

    StatusCode BosonTaggerAlg::execute(){
              
      // Retrieve inputs
      SG::ReadHandle<xAOD::JetContainer> LargeRJets(m_LargeRJetsHandle);

      // boson tagger
      ATH_CHECK( m_MLTagger->GetImageScore( *LargeRJets ));
      ATH_CHECK( m_MLTagger->GetHLScore( *LargeRJets ));

      return StatusCode::SUCCESS;
    }

}
