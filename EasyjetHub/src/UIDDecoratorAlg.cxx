/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "UIDDecoratorAlg.h"

#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>


namespace Easyjet
{
  UIDDecoratorAlg::UIDDecoratorAlg(
    const std::string &name, ISvcLocator *pSvcLocator)
    : AthAlgorithm(name, pSvcLocator) { }

  StatusCode UIDDecoratorAlg::initialize()
  {
    ATH_MSG_DEBUG("Initialising " << name());

    ATH_CHECK(m_truthParticleInKey.initialize());

    m_barcodeKey = m_truthParticleInKey.key() + "." + m_barcodeKey.key();
    m_uidKey = m_truthParticleInKey.key() + "." + m_uidKey.key();
    ATH_CHECK(m_barcodeKey.initialize());
    ATH_CHECK(m_uidKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode UIDDecoratorAlg::execute()
  {
    SG::ReadHandle<xAOD::TruthParticleContainer> truthParticleContainer(
        m_truthParticleInKey);
    ATH_CHECK(truthParticleContainer.isValid());

    SG::ReadDecorHandle<xAOD::TruthParticleContainer, int> barcode(m_barcodeKey);
    SG::WriteDecorHandle<xAOD::TruthParticleContainer, int> uid(m_uidKey);

    for (const xAOD::TruthParticle *tp : *truthParticleContainer){
      uid(*tp) = barcode(*tp);
    }

    return StatusCode::SUCCESS;
  }

}
