/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!

#ifndef SELECTIONFILTERDALITZYYBBALG_H
#define SELECTIONFILTERDALITZYYBBALG_H

#include <AsgDataHandles/ReadHandleKey.h>

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <xAODTruth/TruthParticleContainer.h>


#include <EventBookkeeperTools/FilterReporterParams.h>


namespace HHBBYY
{
  
  /// \brief An algorithm for counting containers
  class bbyyFilterDalitzAlg final : public AthHistogramAlgorithm {

    public:
      bbyyFilterDalitzAlg(const std::string &name, ISvcLocator *pSvcLocator);

      /// \brief Initialisation method, for setting up tools and other persistent
      /// configs
      StatusCode initialize() override;
      /// \brief Execute method, for actions to be taken in the event loop
      StatusCode execute() override;
      /// \brief This is the mirror of initialize() and is called after all events are processed.
      StatusCode finalize() override; ///I added this to write the cutflow histogram.

      bool isDalitz(const xAOD::TruthParticleContainer &truthPtcls);

    private :
      SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleSMInKey{
        this, "TruthParticleSMInKey", "",
        "the truth Standard Model particles container to run on"};

      SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleBSMInKey{
        this, "TruthParticleBSMInKey", "",
        "the truth Beyond Standard Model particles container to run on"};

      FilterReporterParams m_filterParams {this, "nodalitz" ,"no Dalitz selection"};

  };

}

#endif // SELECTIONFILTERDALITZYYBBALG_H
