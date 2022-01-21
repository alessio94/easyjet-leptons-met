///////////////////////// -*- C++ -*- /////////////////////////////
// MllPlotterAlg.h
// Header file for algorithm class MllPlotterAlg
//
// This is an algorithm that will run in the event loop.
// The framework will call the "execute" method on each event.
// It can in turn call tools that typically do specialised tasks
//
// Author: T.J.Khoo<khoo@cern.ch>
///////////////////////////////////////////////////////////////////

// Always protect against multiple includes!
#ifndef PLOTTING_MLLPLOTTERALG
#define PLOTTING_MLLPLOTTERALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

// A header from this package's installed Library
#include "Plotting/HistSpec.h"

#include "xAODBase/IParticleContainer.h"

namespace MSA
{

  /// \brief An algorithm for plotting dilepton masses
  class MllPlotterAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
  public:
    MllPlotterAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

  private:
    // Call in execute to fill histograms
    StatusCode fillMllHistogram(const xAOD::IParticleContainer &);

    // Member variables for configuration
    SG::ReadHandleKey<xAOD::IParticleContainer> m_leptonPairKey{this, "LeptonPairKey", "", "Lepton pair container to plot"};
    HistSpec1D m_mllSpec{this, "MllHist", 100, 0., 200., "Histogram: Dilepton mass"};
  };
}

#endif
