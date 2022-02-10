///////////////////////// -*- C++ -*- /////////////////////////////
// VariablePlotterAlg.h
//
// This is an algorithm that will run in the event loop.
// The framework will call the "execute" method on each event.
// It can in turn call tools that typically do specialised tasks
//
// Author: Victor Ruelas<victor.hugo.ruelas.rivera@cern.ch>
///////////////////////////////////////////////////////////////////

// Always protect against multiple includes!
#ifndef PLOTTING_VARIABLEPLOTTERALG
#define PLOTTING_VARIABLEPLOTTERALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <xAODJet/JetContainer.h>

// A header from this package's installed Library
#include "Plotting/HistSpec.h"

namespace MSA
{

  /// \brief An algorithm for plotting dilepton masses
  class VariablePlotterAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
  public:
    VariablePlotterAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

  private:
    StatusCode bookHistograms();

    // Call in execute to fill histograms
    StatusCode fillVariableHistogram(const xAOD::JetContainer &);

    // Member variables for configuration
    // We use a special templated class to more easily define the properties needed
    // to define a single histogram.
    HistSpec1D m_jetPt{this, "JetPtHist", 100, 0, 500., "Histogram: jet pt [GeV]"};
  };
}

#endif
