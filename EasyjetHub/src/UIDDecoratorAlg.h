/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// UIDDecroatorAlg.h
//
// Decorate TruthParticleContainer with UID, for old DAOD with barcode
//
// Author: Thomas Strebler
///////////////////////////////////////////////////////////////////

// Always protect against multiple includes!
#ifndef EASYJETHUB_UIDDECORATORALG
#define EASYJETHUB_UIDDECORATORALG

#include <AthenaBaseComps/AthAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>

#include <xAODTruth/TruthParticleContainer.h>

namespace Easyjet
{
  /// \brief An algorithm for dumping variables
  class UIDDecoratorAlg final : public AthAlgorithm
  {
    /// \brief The standard constructor
  public:
    UIDDecoratorAlg(const std::string &name,
		    ISvcLocator *pSvcLocator);

    StatusCode initialize() override;
    StatusCode execute() override;

  private:
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleInKey{
        this, "TruthParticleKey", "", "the truth particle container to run on"};
    SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_barcodeKey{
        this, "BarcodeKey", "barcode", ""};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_uidKey{
        this, "UIDKey", "uid", ""};
  };
}

#endif

