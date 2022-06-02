///////////////////////// -*- C++ -*- /////////////////////////////
/// @author Victor Ruelas

//
// includes
//

// Class definition
#include "VariableDumperAlg.h"

//
// method implementations
//

namespace HH4B
{
  VariableDumperAlg ::
      VariableDumperAlg(const std::string &name, ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode VariableDumperAlg ::
      initialize()
  {
    ATH_MSG_DEBUG("Initialising " << name());

    if (m_EventInfoKey.empty())
    {
      ATH_MSG_ERROR("No input collection provided for EventInfo!");
      return StatusCode::FAILURE;
    }
    if (m_Reco4JetsKey.empty())
    {
      ATH_MSG_ERROR("No input collection provided for Jets!");
      return StatusCode::FAILURE;
    }

    ATH_CHECK(m_EventInfoKey.initialize());
    ATH_CHECK(m_Reco4JetsKey.initialize());

    ATH_MSG_INFO("Will search \"" << m_EventInfoKey.key() << "\" for event info");
    ATH_MSG_INFO("Will search \"" << m_Reco4JetsKey.key() << "\" for jets info");

    ATH_CHECK(m_btagSelTool.retrieve());

    ATH_MSG_DEBUG("Booking tree.");
    ATH_CHECK(bookTTree());

    return StatusCode::SUCCESS;
  }

  StatusCode VariableDumperAlg ::
      execute()
  {
    ATH_MSG_DEBUG("Executing " << name());

    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    SG::ReadHandle<xAOD::JetContainer> jets(m_Reco4JetsKey);
    ATH_CHECK(eventInfo.isValid());
    ATH_CHECK(jets.isValid());

    ATH_CHECK(fillVariableTTree(*eventInfo, *jets));

    return StatusCode::SUCCESS;
  }

  StatusCode VariableDumperAlg ::
      bookTTree()
  {
    ATH_CHECK(book(TTree("Variables", "EvenInfo and jet variables tree")));

    TTree *mytree = tree("Variables");
    mytree->Branch("RunNumber", &m_runNumber);
    mytree->Branch("EventNumber", &m_eventNumber);
    mytree->Branch("Reco4JetEta", &m_reco4JetEta);
    mytree->Branch("Reco4JetPhi", &m_reco4JetPhi);
    mytree->Branch("Reco4JetPt", &m_reco4JetPt);
    mytree->Branch("Reco4JetE", &m_reco4JetE);
    mytree->Branch("Reco4JetM", &m_reco4JetM);
    mytree->Branch("Reco4BTagJetEta", &m_reco4BTagJetEta);
    mytree->Branch("Reco4BTagJetPhi", &m_reco4BTagJetPhi);
    mytree->Branch("Reco4BTagJetPt", &m_reco4BTagJetPt);
    mytree->Branch("Reco4BTagJetE", &m_reco4BTagJetE);
    mytree->Branch("Reco4BTagJetM", &m_reco4BTagJetM);

    return StatusCode::SUCCESS;
  }

  StatusCode VariableDumperAlg ::
      fillVariableTTree(const xAOD::EventInfo &eventInfo, const xAOD::JetContainer &jets)
  {
    ATH_MSG_DEBUG("Filling EventInfo and jet variables.");
    m_runNumber = eventInfo.runNumber();
    m_eventNumber = eventInfo.eventNumber();

    // Being lazy here and not checking for pointer validity!
    for (const xAOD::Jet *jet : jets)
    {
      m_reco4JetEta.push_back(jet->eta());
      m_reco4JetPhi.push_back(jet->phi());
      m_reco4JetPt.push_back(jet->pt());
      m_reco4JetE.push_back(jet->e());
      m_reco4JetM.push_back(jet->m());
      if (m_btagSelTool->accept(*jet))
      {
        m_reco4BTagJetEta.push_back(jet->eta());
        m_reco4BTagJetPhi.push_back(jet->phi());
        m_reco4BTagJetPt.push_back(jet->pt());
        m_reco4BTagJetE.push_back(jet->e());
        m_reco4BTagJetM.push_back(jet->m());
      }
    }

    tree("Variables")->Fill();

    return StatusCode::SUCCESS;
  }

}
