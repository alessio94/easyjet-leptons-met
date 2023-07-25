/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Carl Gwilliam

#include "TauDecoratorAlg.h"

namespace Easyjet
{
  TauDecoratorAlg ::TauDecoratorAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator)
  {
  
  }

  StatusCode TauDecoratorAlg ::initialize()
  {
    ATH_CHECK (m_tausInKey.initialize());


    return StatusCode::SUCCESS;
  }

  StatusCode TauDecoratorAlg ::execute(const EventContext& ctx) const
  {

    SG::ReadHandle<xAOD::TauJetContainer> tausIn(m_tausInKey,ctx);
    ATH_CHECK (tausIn.isValid());


    SG::AuxElement::Decorator<float> dec("nProng");

    for(const xAOD::TauJet* tau : *tausIn) {
      dec(*tau) = tau->nTracks();
    }

    return StatusCode::SUCCESS;
  }
}
