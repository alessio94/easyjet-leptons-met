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
    if (m_JetsKey.empty())
    {
      ATH_MSG_ERROR("No input collection provided for Jets!");
      return StatusCode::FAILURE;
    }

    ATH_CHECK(m_EventInfoKey.initialize());
    ATH_CHECK(m_JetsKey.initialize());

    ATH_MSG_INFO("Will search \"" << m_EventInfoKey.key() << "\" for event info");
    ATH_MSG_INFO("Will search \"" << m_JetsKey.key() << "\" for jets info");

    ATH_MSG_DEBUG("Booking tree.");
    ATH_CHECK(bookTTree());

    return StatusCode::SUCCESS;
  }

  StatusCode VariableDumperAlg ::
      execute()
  {
    ATH_MSG_DEBUG("Executing " << name());

    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    SG::ReadHandle<xAOD::JetContainer> jets(m_JetsKey);
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
    mytree->Branch("JetEta", &m_jetEta);
    mytree->Branch("JetPhi", &m_jetPhi);
    mytree->Branch("JetPt", &m_jetPt);
    mytree->Branch("JetE", &m_jetE);

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
      m_jetEta.push_back(jet->eta());
      m_jetPhi.push_back(jet->phi());
      m_jetPt.push_back(jet->pt());
      m_jetE.push_back(jet->e());
    }

    tree("Variables")->Fill();

    return StatusCode::SUCCESS;
  }

}
