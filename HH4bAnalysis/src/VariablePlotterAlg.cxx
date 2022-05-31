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
      : AthHistogramAlgorithm(name, pSvcLocator), m_btagSelTool("BTaggingSelectionTool", this)

  {

    // Property name, member variable, documentation string
    // declareProperty("NTrk_Min", m_minNtrks, "Minimum NTrks for vertex counting");

    // declareProperty("TaggerName", m_BtagTagger);
    // declareProperty("BtaggingOperatingPoint", m_BtagWP);
    // // declareProperty("JetAuthor", jetcollBTag);
    // //  declareProperty("MinPt", m_BtagMinPt);
    // declareProperty("FlvTagCutDefinitionsFileName", m_bTaggingCalibrationFilePath);

    declareProperty("FlvTagCutDefinitionsFileName", m_CutFileName = "", "name of the files containing official cut definitions (uses PathResolver)");
    declareProperty("TaggerName", m_taggerName = "", "tagging algorithm name");
    declareProperty("OperatingPoint", m_OP = "", "operating point");
    declareProperty("JetAuthor", m_jetAuthor = "", "jet collection");
  }

  StatusCode VariablePlotterAlg ::
      initialize()
  {
    ATH_MSG_DEBUG("Initialising " << name());

    ATH_MSG_DEBUG("Booking histograms.");
    ATH_CHECK(bookHistograms());

    // ATH_CHECK(m_btagSelTool.retrieve());

    // ATH_CHECK(m_btagSelTool.setProperty("FlvTagCutDefinitionsFileName", "xAODBTaggingEfficiency/13TeV/2021-22-13TeV-MC16-CDI-2021-12-02_v2.root ")); // this is the CDI file
    // ATH_CHECK(m_btagSelTool.setProperty("TaggerName", "DL1r"));
    // ATH_CHECK(m_btagSelTool.setProperty("OperatingPoint", "FixedCutBEff_77"));
    // ATH_CHECK(m_btagSelTool.setProperty("JetAuthor", "AntiKt4EMPLowJets"));
    if ATH_CHECK(m_btagSelTool.initialize());

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
