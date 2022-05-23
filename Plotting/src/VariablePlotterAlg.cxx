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
    ATH_CHECK(bookTTree());
    ATH_CHECK(bookHistograms());

    return StatusCode::SUCCESS;
  }

  StatusCode VariablePlotterAlg ::
      execute()
  {
    ATH_MSG_DEBUG("Executing " << name());

    const xAOD::EventInfo *eventInfo(nullptr);
    const xAOD::JetContainer *jets(nullptr);
    ATH_CHECK(evtStore()->retrieve(eventInfo, "EventInfo"));
    ATH_CHECK(evtStore()->retrieve(jets, "AnalysisJets"));
    if (eventInfo == nullptr)
    {
      ATH_MSG_ERROR("Got null pointer for EventInfo!");
      return StatusCode::FAILURE;
    }
    if (jets == nullptr)
    {
      ATH_MSG_ERROR("Got null pointer for JetContainer!");
      return StatusCode::FAILURE;
    }
    ATH_CHECK(fillVariableTTree(*eventInfo, *jets));
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

  StatusCode VariablePlotterAlg ::
      bookTTree()
  {
    ATH_CHECK(book(TTree("analysisvariables", "EvenInfo and jet variables ntuple")));

    TTree *mytree = tree("analysisvariables");
    mytree->Branch("RunNumber", &m_runNumber);
    mytree->Branch("EventNumber", &m_eventNumber);
    mytree->Branch("JetEta", &m_jetEta);
    mytree->Branch("JetPhi", &m_jetPhi);
    mytree->Branch("JetPt", &m_jetPt);
    mytree->Branch("JetE", &m_jetE);

    return StatusCode::SUCCESS;
  }

  StatusCode VariablePlotterAlg ::
      fillVariableTTree(const xAOD::EventInfo &eventInfo, const xAOD::JetContainer &jets)
  {
    ATH_MSG_DEBUG("Filling EventInfo and jet variables.");
    m_runNumber = eventInfo.runNumber();
    m_eventNumber = eventInfo.eventNumber();

    // Being lazy here and not checking for pointer validity!
    for (const xAOD::Jet *jet : jets)
    {
      m_jetEta.push_back(jet->eta());
      m_jetPhi.push_back(jet->phi());
      m_jetPt.push_back(jet->pt());
      m_jetE.push_back(jet->e());
    }

    tree("analysisvariables")->Fill();

    return StatusCode::SUCCESS;
  }

}
