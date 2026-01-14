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

    ATH_CHECK (m_Wtagger.retrieve());
    
    return StatusCode::SUCCESS;
  }

  StatusCode WTaggingDecoratorAlg ::execute() 
  {

    SG::ReadHandle<xAOD::JetContainer> LRjets(m_jetsInKey);
    ATH_CHECK (LRjets.isValid());
    ATH_MSG_DEBUG("Number of LR jets: " << LRjets->size());
    ATH_CHECK( m_Wtagger->decorate( *LRjets ));
    return StatusCode::SUCCESS;
    
  }
}
