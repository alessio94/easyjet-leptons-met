///////////////////////// -*- C++ -*- /////////////////////////////
/// @author TJ Khoo

//
// includes
//

// Class definition
#include "MllPlotterAlg.h"
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

  MllPlotterAlg ::
      MllPlotterAlg(const std::string &name, ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode MllPlotterAlg ::
      initialize()
  {
    ATH_MSG_DEBUG("Initialising " << name());

    ATH_CHECK(m_leptonPairKey.initialize());

    ATH_CHECK(book(TH1D("Mll", "Dilepton mass",
                        m_mllSpec.nbinsx, m_mllSpec.xmin, m_mllSpec.xmax)));

    return StatusCode::SUCCESS;
  }

  StatusCode MllPlotterAlg ::
      execute()
  {
    ATH_MSG_DEBUG("Executing " << name());

    SG::ReadHandle<xAOD::IParticleContainer> leptonpair(m_leptonPairKey);
    ATH_CHECK(leptonpair.isValid());
    ATH_CHECK(fillMllHistogram(*leptonpair));

    return StatusCode::SUCCESS;
  }

  StatusCode MllPlotterAlg ::
      fillMllHistogram(const xAOD::IParticleContainer &leptonpair)
  {
    ATH_MSG_DEBUG("Filling Mll histograms.");

    ATH_MSG_VERBOSE("Leading lepton has pt " << leptonpair[0]->pt());
    ATH_MSG_VERBOSE("Subleading lepton has pt " << leptonpair[1]->pt());

    // Being lazy here and not checking for pointer validity!
    // xAOD::IParticle gives access to p4(), which returns a
    // TLorentzVector.
    // With this, we can easily do 4-vector computations.
    hist("Mll")->Fill((leptonpair[0]->p4() + leptonpair[1]->p4()).M() * invGeV);

    return StatusCode::SUCCESS;
  }

}
