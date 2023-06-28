/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Teng Jian Khoo

#include "BTaggingDecoratorAlg.h"

namespace Easyjet
{
  BTaggingDecoratorAlg ::BTaggingDecoratorAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator),
        m_btagLinkAcc(m_btagLinkName)
  {
  
  }

  StatusCode BTaggingDecoratorAlg ::initialize()
  {
    ATH_CHECK (m_jetsInKey.initialize());
    m_btagLinkAcc = SG::AuxElement::ConstAccessor<ElementLink<xAOD::BTaggingContainer> >(m_btagLinkName);

    // Build ConstAccessor/Decorator pairs for each
    // variable to be copied
    for(const std::string& var : m_floatVars) {
      m_floatHandlers.emplace_back(var,var);
    }

    return StatusCode::SUCCESS;
  }

  StatusCode BTaggingDecoratorAlg ::execute(const EventContext& ctx) const
  {

    SG::ReadHandle<xAOD::JetContainer> jetsIn(m_jetsInKey,ctx);
    ATH_CHECK (jetsIn.isValid());

    for(const xAOD::Jet* jet : *jetsIn) {
      const xAOD::BTagging* btag = *m_btagLinkAcc(*jet);

      // Read the variable from the b-tag object and decorate on the jet
      for(const auto&[cacc,dec] : m_floatHandlers) {
        dec(*jet) = cacc(*btag);
      }
    }

    return StatusCode::SUCCESS;
  }
}
