/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Hsuan-Chu Lien

#include "WTaggingDecoratorAlg.h"

namespace Easyjet
{
  WTaggingDecoratorAlg ::WTaggingDecoratorAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
      : AthAlgorithm(name, pSvcLocator)
  {
  
  }

  StatusCode WTaggingDecoratorAlg ::initialize()
  {
    ATH_CHECK (m_jetsInKey.initialize());

    if(m_taggerType.value() == "JSSWTopTaggerANN"){

      m_ANNWtagger.setTypeAndName(m_taggerType.value()+"/MyTagger");
      ATH_CHECK( m_ANNWtagger.setProperty( "ContainerName", m_jetsInKey.key()) );
      ATH_CHECK( m_ANNWtagger.setProperty( "ConfigFile", m_configFile.value()) );
      ATH_CHECK( m_ANNWtagger.setProperty( "CalibArea", m_calibArea.value()) );
      ATH_CHECK( m_ANNWtagger.setProperty("IsMC", m_isMC.value()) );
      ATH_CHECK( m_ANNWtagger.retrieve() );

    }

    else if(m_taggerType.value() == "JSSWTopTaggerDNN"){

      m_DNNWtagger.setTypeAndName(m_taggerType.value()+"/MyTagger");
      ATH_CHECK( m_DNNWtagger.setProperty( "ContainerName", m_jetsInKey.key()) );
      ATH_CHECK( m_DNNWtagger.setProperty( "ConfigFile", m_configFile.value()) );
      ATH_CHECK( m_DNNWtagger.setProperty( "CalibArea", m_calibArea.value()) ); 
      ATH_CHECK( m_DNNWtagger.setProperty("IsMC", m_isMC.value()) );
      ATH_CHECK( m_DNNWtagger.retrieve() );

    }

    else{
      ATH_MSG_ERROR("W tagger type is incorrect.");
      return StatusCode::FAILURE;
    }
    
    return StatusCode::SUCCESS;
  }

  StatusCode WTaggingDecoratorAlg ::execute() 
  {

    SG::ReadHandle<xAOD::JetContainer> LRjets(m_jetsInKey);
    ATH_CHECK (LRjets.isValid());
    ATH_MSG_DEBUG("Number of LR jets: " << LRjets->size());
    if(m_taggerType.value() == "JSSWTopTaggerANN") ATH_CHECK( m_ANNWtagger->decorate( *LRjets ) );
    else ATH_CHECK( m_DNNWtagger->decorate( *LRjets ) );

    return StatusCode::SUCCESS;
    
  }
}
