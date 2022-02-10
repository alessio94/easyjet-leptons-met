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

namespace MSA
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

    ATH_MSG_DEBUG("Booking histograms.");
    ATH_CHECK(bookHistograms());

    return StatusCode::SUCCESS;
  }

  StatusCode VariablePlotterAlg ::
      execute()
  {
    ATH_MSG_DEBUG("Executing " << name());

    const xAOD::JetContainer *jets(nullptr);
    ATH_CHECK(evtStore()->retrieve(jets, "AntiKt4EMPFlowJets"));
    if (jets == nullptr)
    {
      ATH_MSG_ERROR("Got null pointer for JetContainer!");
      return StatusCode::FAILURE;
    }
    ATH_CHECK(fillVariableHistogram(*jets));

    return StatusCode::SUCCESS;
  }

  StatusCode VariablePlotterAlg ::
      bookHistograms()
  {
    ATH_CHECK(book(TH1D("JetPt", "Jet pT [GeV]", m_jetPt.nbinsx, m_jetPt.xmin, m_jetPt.xmax)));

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
    }

    return StatusCode::SUCCESS;
  }

}
