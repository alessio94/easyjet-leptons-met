///////////////////////// -*- C++ -*- /////////////////////////////
// DileptonFinder.h
// Header file for algorithm class DileptonFinder
//
// This algorithm does two things:
// * First, it checks if there are two leptons in the specified
//   collection. If not, it sets the filter condition to failed.
// * If there are at least two leptons, it records a VIEW CONTAINER
//   holding the leading lepton pair.
//
// Author: T.J.Khoo<khoo@cern.ch>
///////////////////////////////////////////////////////////////////

// Always protect against multiple includes!
#ifndef PLOTTING_DILEPTONFINDERALG
#define PLOTTING_DILEPTONFINDERALG

#include <AthenaBaseComps/AthAlgorithm.h>
#include <xAODBase/IParticleContainer.h>
#include "AthContainers/ConstDataVector.h"

namespace MSA
{

  /// \brief An algorithm for finding a dilepton pair.
  class DileptonFinderAlg final : public AthAlgorithm
  {
    /// \brief The standard constructor
  public:
    DileptonFinderAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent configs
    ///        This is called before the event loop starts
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    ///        This is called by the framework every event
    ///        As for tools, it should be const, but in the early days
    ///        we didn't know to do this stuff...
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

  private:
    StatusCode recordLeadingLeptonPair(const xAOD::IParticleContainer &);

    // Member variables for configuration
    SG::ReadHandleKey<xAOD::IParticleContainer> m_leptonsInKey{this, "LeptonsInKey", "", "Lepton container to read"};
    SG::WriteHandleKey<ConstDataVector<xAOD::IParticleContainer>> m_leptonsOutKey{this, "LeptonsOutKey", "", "Lepton pair container to write"};
  };
}

#endif
