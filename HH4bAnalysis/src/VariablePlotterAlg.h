///////////////////////// -*- C++ -*- /////////////////////////////
// VariablePlotterAlg.h
//
// This is an algorithm that will create histograms of different
// variables.
//
// Author: Victor Ruelas<victor.hugo.ruelas.rivera@cern.ch>
///////////////////////////////////////////////////////////////////

// Always protect against multiple includes!
#ifndef HH4BANALYSIS_VARIABLEPLOTTERALG
#define HH4BANALYSIS_VARIABLEPLOTTERALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <xAODJet/JetContainer.h>

// A header from this package's installed Library
#include "HH4bAnalysis/HistSpec.h"

#include <memory> // for unique_ptr

namespace HH4B
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
    SG::ReadHandleKey<xAOD::JetContainer> m_JetsKey{this, "JetsKey", "", "Jets container to dump"};

    // Member variables for configuration
    // We use a special templated class to more easily define the properties needed
    // to define a single histogram.
    HistSpec1D m_jetPtHist{this, "JetPtHist", 100, 0, 500, "Histogram: jet pt [GeV]"};
    HistSpec1D m_jetEtaHist{this, "JetEtaHist", 100, -6, 6, "Histogram: jet eta"};
    HistSpec1D m_jetPhiHist{this, "JetPhiHist", 100, -4, 4, "Histogram: jet phi"};
  };
}

#endif
