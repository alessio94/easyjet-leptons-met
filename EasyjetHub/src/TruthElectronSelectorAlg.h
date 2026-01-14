/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EASYJET_TRUTHELECTRONSELECTORALG
#define EASYJET_TRUTHELECTRONSELECTORALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>

#include <AthContainers/ConstDataVector.h>
#include <xAODTruth/TruthEvent.h>
#include <xAODTruth/TruthEventContainer.h>
#include <xAODTruth/TruthParticle.h>
#include <xAODTruth/TruthParticleContainer.h> 

namespace Easyjet
{

  /// \brief An algorithm for counting containers
  class TruthElectronSelectorAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    TruthElectronSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:

    SG::ReadHandleKey<xAOD::TruthEventContainer>
      m_eventHandleKey{ this, "event", "TruthEvents", "TruthEvents container to read" };

    SG::ReadHandleKey<xAOD::TruthParticleContainer>
      m_inHandleKey{ this, "containerInKey", "",   "Electron container to read" };

    /// \brief Setup output container handles
    SG::WriteHandleKey<ConstDataVector<xAOD::TruthParticleContainer>>
      m_outHandleKey{ this, "containerOutKey", "",   "Electron container to write" };

    /// \brief Setup output decorations
    SG::WriteDecorHandleKey<xAOD::TruthEventContainer> m_nSelPartKey {this, "decorOutName", "nElectrons", 
        "Name out output decorator for number of selected electrons"};

    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_isSelectedElectronKey {
        this, "decoration", "isTruthElectron", "decoration for per-object if electron is selected"
    };

    Gaudi::Property<float> m_minPt            {this, "minPt", 7e3, "Minimum pT of electrons"};
    Gaudi::Property<float> m_maxEta           {this, "maxEta", 100., "Maximum eta of electrons"};
    Gaudi::Property<int>   m_minimumAmount    {this, "minimumAmount", -1, "Minimum number of electrons to consider"}; // -1 means ignores this
    Gaudi::Property<bool>  m_pTsort           {this, "pTsort", true, "Sort electrons by pT"};
    Gaudi::Property<int>   m_truncateAtAmount {this, "truncateAtAmount", -1, "Remove extra electrons after pT sorting"}; // -1 means keep them all

  };
}

#endif