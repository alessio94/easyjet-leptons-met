///////////////////////// -*- C++ -*- /////////////////////////////
// PileupPlotterAlg.h
// Header file for algorithm class PileupPlotterAlg
//
// This is an algorithm that will run in the event loop.
// The framework will call the "execute" method on each event.
// It can in turn call tools that typically do specialised tasks
//
// Author: T.J.Khoo<khoo@cern.ch>
///////////////////////////////////////////////////////////////////

// Always protect against multiple includes!
#ifndef HH4BANALYSIS_PILEUPPLOTTERALG
#define HH4BANALYSIS_PILEUPPLOTTERALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <AsgTools/ToolHandle.h>

#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/VertexContainer.h"

// A header from this package's installed Library
#include "HH4bAnalysis/HistSpec.h"

namespace MSA
{

  /// \brief Forward declare the tool interface for templating, as
  /// we don't need any information on the details.
  class IVertexCounter;

  /// \brief An algorithm for plotting pileup quantities
  class PileupPlotterAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
  public:
    PileupPlotterAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

  private:
    // Call in initialisation to add histograms to the THistSvc
    StatusCode bookHistograms();

    // Call in execute to fill histograms
    // Keep the "fill" method simple -- we can retrieve in execute.
    // Pass objects by reference, not by pointer unless explicit memory
    // management is unavoidable.
    // Be aware of constness to prevent unexpected alterations of data
    StatusCode fillMuHistograms(const xAOD::EventInfo &);
    StatusCode fillNVtxHistograms(const xAOD::VertexContainer &);

    // Member variables for configuration
    // We use a special templated class to more easily define the properties needed
    // to define a single histogram.
    HistSpec1D m_nVtxSpec{this, "NVtxHist", 10, 0., 20., "Histogram: Number of primary vertices"};
    // Average is over one lumi-block (~5 min)
    HistSpec1D m_avgMuSpec{this, "AvgMuHist", 10, 0., 200., "Histogram: Average interactions/bunch crossing"};
    // The estimated value for the exact bunch crossing, taking into account bunch train structure
    HistSpec1D m_actualMuSpec{this, "ActualMuHist", 10, 0., 200., "Histogram: Actual interactions/bunch crossing"};

    /// \brief A tool used to count vertices
    ToolHandle<MSA::IVertexCounter> m_vertexCounter{this, "VertexCounter", "", "A tool for counting vertices"};
  };
}

#endif
