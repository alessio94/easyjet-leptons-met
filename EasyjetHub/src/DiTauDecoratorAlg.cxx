/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "DiTauDecoratorAlg.h"
#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>


namespace Easyjet
{
  DiTauDecoratorAlg::DiTauDecoratorAlg(const std::string &name,
				       ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator) { }

  StatusCode DiTauDecoratorAlg::initialize()
  {
    ATH_CHECK (m_diTausInKey.initialize());

    m_scoreDecorKey = m_diTausInKey.key() + ".omni_score";
    ATH_CHECK(m_scoreDecorKey.initialize());

    ATH_CHECK (m_diTauIDVarCalculator.retrieve());
    ATH_CHECK (m_diTauOnnxDiscriminantTool.retrieve());

    return StatusCode::SUCCESS;
  }

  StatusCode DiTauDecoratorAlg::execute(const EventContext& ctx) const
  {
    // input handles
    SG::ReadHandle<xAOD::DiTauJetContainer> diTausIn(m_diTausInKey,ctx);
    ATH_CHECK (diTausIn.isValid());

    // The diTauOnnxDiscriminantTool tool decorates using for now some SG::Decorator
    // The decoration is then not available to be dumped in case the di-tau container is empty
    // Properly declaring the WriteDecorHandle here
    // The filling is performed in the diTauOnnxDiscriminantTool itself
    SG::WriteDecorHandle<xAOD::DiTauJetContainer, float> scoreDecorHandle(m_scoreDecorKey);

    for(const xAOD::DiTauJet* ditau : *diTausIn) {
      ATH_CHECK(m_diTauIDVarCalculator->execute(*ditau));
      ATH_CHECK(m_diTauOnnxDiscriminantTool->execute(*ditau));
    }
    
    return StatusCode::SUCCESS;
  }

}
