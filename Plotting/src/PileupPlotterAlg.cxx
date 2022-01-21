///////////////////////// -*- C++ -*- /////////////////////////////
/// @author TJ Khoo

//
// includes
//

// Class definition
#include "PileupPlotterAlg.h"

// Tool interface header
#include "Plotting/IVertexCounter.h"
// ROOT histograms
#include <TH1D.h>

//
// method implementations
//

namespace MSA
{
  PileupPlotterAlg ::
      PileupPlotterAlg(const std::string &name, ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode PileupPlotterAlg ::
      initialize()
  {
    ATH_MSG_DEBUG("Initialising " << name());

    ATH_MSG_DEBUG("Attempting to retrieve IVertexCounter \"" << m_vertexCounter.name() << "\"");
    ATH_CHECK(m_vertexCounter.retrieve());

    ATH_MSG_DEBUG("Booking histograms.");
    ATH_CHECK(bookHistograms());

    return StatusCode::SUCCESS;
  }

  StatusCode PileupPlotterAlg ::
      bookHistograms()
  {

    // The "book" method (and "hist") is defined in AthHistogramming.
    // Look this up with LXR (the Athena code browser):
    // https://acode-browser1.usatlas.bnl.gov
    ATH_CHECK(book(TH1D("NVtx", "Number of Primary Vertices",
                        m_nVtxSpec.nbinsx, m_nVtxSpec.xmin, m_nVtxSpec.xmax)));
    ATH_CHECK(book(TH1D("AvgMu", "Average number of interactions per bunch crossing",
                        m_avgMuSpec.nbinsx, m_avgMuSpec.xmin, m_avgMuSpec.xmax)));
    ATH_CHECK(book(TH1D("ActualMu", "Actual number of interactions per bunch crossing",
                        m_actualMuSpec.nbinsx, m_actualMuSpec.xmin, m_actualMuSpec.xmax)));

    return StatusCode::SUCCESS;
  }

  StatusCode PileupPlotterAlg ::
      execute()
  {
    ATH_MSG_DEBUG("Executing " << name());

    const xAOD::EventInfo *eventInfo(nullptr);
    // When there is only one object in StoreGate, we can skip the key
    // Having said that, it is strictly better to specify a known one,
    // as behaviour can be undefined if there is unexpectedly more than one.
    // E.g. sometimes analyses might copy the EventInfo for systematics
    ATH_CHECK(evtStore()->retrieve(eventInfo));
    if (eventInfo == nullptr)
    {
      ATH_MSG_ERROR("Got null pointer for EventInfo!");
      return StatusCode::FAILURE;
    }
    ATH_CHECK(fillMuHistograms(*eventInfo));

    const xAOD::VertexContainer *vertices(nullptr);
    ATH_CHECK(evtStore()->retrieve(vertices, "PrimaryVertices"));
    if (vertices == nullptr)
    {
      ATH_MSG_ERROR("Got null pointer for PrimaryVertices!");
      return StatusCode::FAILURE;
    }
    ATH_CHECK(fillNVtxHistograms(*vertices));

    return StatusCode::SUCCESS;
  }

  StatusCode PileupPlotterAlg ::
      fillMuHistograms(const xAOD::EventInfo &eventInfo)
  {
    ATH_MSG_DEBUG("Filling Mu histograms.");

    // Being lazy here and not checking for pointer validity!
    hist("AvgMu")->Fill(eventInfo.averageInteractionsPerCrossing());
    hist("ActualMu")->Fill(eventInfo.actualInteractionsPerCrossing());

    return StatusCode::SUCCESS;
  }

  StatusCode PileupPlotterAlg ::
      fillNVtxHistograms(const xAOD::VertexContainer &vertices)
  {
    ATH_MSG_DEBUG("Filling NVtx histograms.");

    // Being lazy here and not checking for pointer validity!
    hist("NVtx")->Fill(m_vertexCounter->countVertices(vertices));

    return StatusCode::SUCCESS;
  }

}
