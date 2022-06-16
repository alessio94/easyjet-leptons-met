///////////////////////// -*- C++ -*- /////////////////////////////
/// @author TJ Khoo

//
// includes
//

// Class definition
#include "PileupPlotterAlg.h"

// Tool interface header
#include "HH4bAnalysis/IVertexCounter.h"
// ROOT histograms
#include <TH1D.h>

//
// method implementations
//

namespace HH4B
{
  PileupPlotterAlg ::PileupPlotterAlg(const std::string &name,
                                      ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode PileupPlotterAlg ::initialize()
  {
    ATH_MSG_DEBUG("Initialising " << name());

    ATH_MSG_DEBUG("Attempting to retrieve IVertexCounter \""
                  << m_vertexCounter.name() << "\"");
    ATH_CHECK(m_vertexCounter.retrieve());

    if (m_EventInfoKey.empty())
    {
      ATH_MSG_ERROR("No input collection provided for EventInfo!");
      return StatusCode::FAILURE;
    }
    if (m_VerticesKey.empty())
    {
      ATH_MSG_ERROR("No input collection provided for Vertexing!");
      return StatusCode::FAILURE;
    }

    ATH_CHECK(m_EventInfoKey.initialize());
    ATH_CHECK(m_VerticesKey.initialize());

    ATH_MSG_INFO("Will search \"" << m_EventInfoKey.key()
                                  << "\" for event info");
    ATH_MSG_INFO("Will search \"" << m_VerticesKey.key()
                                  << "\" for vertexing info");

    ATH_MSG_DEBUG("Booking histograms.");
    ATH_CHECK(bookHistograms());

    return StatusCode::SUCCESS;
  }

  StatusCode PileupPlotterAlg ::execute()
  {
    ATH_MSG_DEBUG("Executing " << name());

    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    ATH_CHECK(eventInfo.isValid());
    ATH_CHECK(fillMuHistograms(*eventInfo));

    SG::ReadHandle<xAOD::VertexContainer> vertices(m_VerticesKey);
    ATH_CHECK(vertices.isValid());
    ATH_CHECK(fillNVtxHistograms(*vertices));

    return StatusCode::SUCCESS;
  }

  StatusCode PileupPlotterAlg ::bookHistograms()
  {

    // The "book" method (and "hist") is defined in AthHistogramming.
    // Look this up with LXR (the Athena code browser):
    // https://acode-browser1.usatlas.bnl.gov
    ATH_CHECK(book(TH1D("NVtx", "Number of Primary Vertices",
                        m_nVtxSpec.nbinsx, m_nVtxSpec.xmin, m_nVtxSpec.xmax)));
    ATH_CHECK(
        book(TH1D("AvgMu", "Average number of interactions per bunch crossing",
                  m_avgMuSpec.nbinsx, m_avgMuSpec.xmin, m_avgMuSpec.xmax)));
    ATH_CHECK(book(TH1D(
        "ActualMu", "Actual number of interactions per bunch crossing",
        m_actualMuSpec.nbinsx, m_actualMuSpec.xmin, m_actualMuSpec.xmax)));

    return StatusCode::SUCCESS;
  }

  StatusCode
  PileupPlotterAlg ::fillMuHistograms(const xAOD::EventInfo &eventInfo)
  {
    ATH_MSG_DEBUG("Filling Mu histograms.");

    // Being lazy here and not checking for pointer validity!
    hist("AvgMu")->Fill(eventInfo.averageInteractionsPerCrossing());
    hist("ActualMu")->Fill(eventInfo.actualInteractionsPerCrossing());

    return StatusCode::SUCCESS;
  }

  StatusCode
  PileupPlotterAlg ::fillNVtxHistograms(const xAOD::VertexContainer &vertices)
  {
    ATH_MSG_DEBUG("Filling NVtx histograms.");

    // Being lazy here and not checking for pointer validity!
    hist("NVtx")->Fill(m_vertexCounter->countVertices(vertices));

    return StatusCode::SUCCESS;
  }

}
