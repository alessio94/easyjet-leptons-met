///////////////////////// -*- C++ -*- /////////////////////////////
/// @author Victor Ruelas

//
// includes
//

// Class definition
#include "VariablePlotterAlg.h"
// ROOT histograms
#include <TH1D.h>
// Get some units defined
#include "GaudiKernel/SystemOfUnits.h"

//
// method implementations
//

namespace HH4B
{
  // Better units for plotting
  using Gaudi::Units::GeV;
  // Multiplication is faster than division,
  // so optimise by multiplying by reciprocals.
  static const float invGeV = 1. / GeV;

  VariablePlotterAlg ::
      VariablePlotterAlg(const std::string &name, ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode VariablePlotterAlg ::
      initialize()
  {
    ATH_MSG_DEBUG("Initialising " << name());

    if (m_JetsKey.empty())
    {
      ATH_MSG_ERROR("No input collection provided for Jets!");
      return StatusCode::FAILURE;
    }

    ATH_CHECK(m_JetsKey.initialize());

    ATH_MSG_INFO("Will search \"" << m_JetsKey.key() << "\" for jets info");

    ATH_MSG_DEBUG("Booking histograms.");
    ATH_CHECK(bookHistograms());

    return StatusCode::SUCCESS;
  }

  StatusCode VariablePlotterAlg ::
      execute()
  {
    ATH_MSG_DEBUG("Executing " << name());

    SG::ReadHandle<xAOD::JetContainer> jets(m_JetsKey);
    ATH_CHECK(jets.isValid());

    ATH_CHECK(fillVariableHistogram(*jets));

    return StatusCode::SUCCESS;
  }

  StatusCode VariablePlotterAlg ::
      bookHistograms()
  {
    ATH_CHECK(book(TH1D("JetPt", "Jet pT [GeV]", m_jetPtHist.nbinsx, m_jetPtHist.xmin, m_jetPtHist.xmax)));
    ATH_CHECK(book(TH1D("JetEta", "Jet eta", m_jetEtaHist.nbinsx, m_jetEtaHist.xmin, m_jetEtaHist.xmax)));
    ATH_CHECK(book(TH1D("JetPhi", "Jet phi", m_jetPhiHist.nbinsx, m_jetPhiHist.xmin, m_jetPhiHist.xmax)));

    return StatusCode::SUCCESS;
  }

  StatusCode VariablePlotterAlg ::
      fillVariableHistogram(const xAOD::JetContainer &jets)
  {
    ATH_MSG_DEBUG("Filling Variable histograms.");

    // Being lazy here and not checking for pointer validity!
    for (const xAOD::Jet *jet : jets)
    {
      hist("JetPt")->Fill(jet->pt() * invGeV);
      hist("JetEta")->Fill(jet->eta());
      hist("JetPhi")->Fill(jet->phi());
    }

    return StatusCode::SUCCESS;
  }

}
