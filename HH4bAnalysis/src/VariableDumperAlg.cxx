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
      ATH_MSG_ERROR("No input collection provided for small R jets!");
      return StatusCode::FAILURE;
    }
    if (m_Reco10JetsKey.empty())
    {
      ATH_MSG_ERROR("No input collection provided for large R jets!");
      return StatusCode::FAILURE;
    }
    if (m_MuonsKey.empty())
    {
      ATH_MSG_ERROR("No input collection provided for muons!");
      return StatusCode::FAILURE;
    }

    ATH_CHECK(m_EventInfoKey.initialize());
    ATH_CHECK(m_Reco4JetsKey.initialize());
    ATH_CHECK(m_Reco10JetsKey.initialize());
    ATH_CHECK(m_MuonsKey.initialize());

    ATH_MSG_INFO("Will search \"" << m_EventInfoKey.key() << "\" for event info");
    ATH_MSG_INFO("Will search \"" << m_Reco4JetsKey.key() << "\" for small R jets info");
    ATH_MSG_INFO("Will search \"" << m_Reco10JetsKey.key() << "\" for large R jets info");
    ATH_MSG_INFO("Will search \"" << m_MuonsKey.key() << "\" for muons info");

    ATH_CHECK(m_btagSelTool.retrieve());

    ATH_MSG_DEBUG("Booking trees.");
    ATH_CHECK(bookEventInfoTree());
    ATH_CHECK(bookReco4JetsTree());
    ATH_CHECK(bookReco10JetsTree());
    ATH_CHECK(bookMuonsTree());

    return StatusCode::SUCCESS;
  }

  StatusCode VariableDumperAlg ::
      execute()
  {
    ATH_MSG_DEBUG("Executing " << name());

    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    SG::ReadHandle<xAOD::JetContainer> reco4Jets(m_Reco4JetsKey);
    SG::ReadHandle<xAOD::JetContainer> reco10Jets(m_Reco10JetsKey);
    SG::ReadHandle<xAOD::MuonContainer> muons(m_MuonsKey);
    ATH_CHECK(eventInfo.isValid());
    ATH_CHECK(reco4Jets.isValid());
    ATH_CHECK(reco10Jets.isValid());
    ATH_CHECK(muons.isValid());

    ATH_CHECK(fillEventInfoTree(*eventInfo));
    ATH_CHECK(fillReco4JetsTree(*reco4Jets));
    ATH_CHECK(fillReco10JetsTree(*reco10Jets));
    ATH_CHECK(fillMuonsTree(*muons));

    return StatusCode::SUCCESS;
  }

  StatusCode VariableDumperAlg ::
      bookEventInfoTree()
  {
    ATH_CHECK(book(TTree("EventInfo", "EvenInfo tree")));

    TTree *eventInfoTree = tree("EventInfo");
    eventInfoTree->Branch("RunNumber", &m_runNumber);
    eventInfoTree->Branch("EventNumber", &m_eventNumber);

    return StatusCode::SUCCESS;
  }

  StatusCode VariableDumperAlg ::
      bookReco4JetsTree()
  {
    ATH_CHECK(book(TTree("Reco4Jets", "Reconstructed small R jets tree")));

    TTree *reco4JetsTree = tree("Reco4Jets");
    reco4JetsTree->Branch("JetEta", &m_reco4JetEta);
    reco4JetsTree->Branch("JetPhi", &m_reco4JetPhi);
    reco4JetsTree->Branch("JetPt", &m_reco4JetPt);
    reco4JetsTree->Branch("JetE", &m_reco4JetE);
    reco4JetsTree->Branch("JetM", &m_reco4JetM);
    reco4JetsTree->Branch("BTagJetEta", &m_reco4BTagJetEta);
    reco4JetsTree->Branch("BTagJetPhi", &m_reco4BTagJetPhi);
    reco4JetsTree->Branch("BTagJetPt", &m_reco4BTagJetPt);
    reco4JetsTree->Branch("BTagJetE", &m_reco4BTagJetE);
    reco4JetsTree->Branch("BTagJetM", &m_reco4BTagJetM);

    return StatusCode::SUCCESS;
  }

  StatusCode VariableDumperAlg ::
      bookReco10JetsTree()
  {
    ATH_CHECK(book(TTree("Reco10Jets", "Reconstructed large R jets tree")));

    TTree *reco10JetsTree = tree("Reco10Jets");
    reco10JetsTree->Branch("JetEta", &m_reco10JetEta);
    reco10JetsTree->Branch("JetPhi", &m_reco10JetPhi);
    reco10JetsTree->Branch("JetPt", &m_reco10JetPt);
    reco10JetsTree->Branch("JetE", &m_reco10JetE);
    reco10JetsTree->Branch("JetM", &m_reco10JetM);

    return StatusCode::SUCCESS;
  }

  StatusCode VariableDumperAlg ::
      bookMuonsTree()
  {
    ATH_CHECK(book(TTree("Muons", "Muons tree")));

    TTree *muonsTree = tree("Muons");
    muonsTree->Branch("MuonEta", &m_muonEta);
    muonsTree->Branch("MuonPhi", &m_muonPhi);
    muonsTree->Branch("MuonPt", &m_muonPt);
    muonsTree->Branch("MuonE", &m_muonE);
    muonsTree->Branch("MuonM", &m_muonM);

    return StatusCode::SUCCESS;
  }

  StatusCode VariableDumperAlg ::
      fillEventInfoTree(const xAOD::EventInfo &eventInfo)
  {
    ATH_MSG_DEBUG("Filling EventInfo tree.");
    m_runNumber = eventInfo.runNumber();
    m_eventNumber = eventInfo.eventNumber();

    tree("EventInfo")->Fill();

    return StatusCode::SUCCESS;
  }

  StatusCode VariableDumperAlg ::
      fillReco4JetsTree(const xAOD::JetContainer &jets)
  {
    ATH_MSG_DEBUG("Filling small R jets tree.");

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

    tree("Reco4Jets")->Fill();

    return StatusCode::SUCCESS;
  }

  StatusCode VariableDumperAlg ::
      fillReco10JetsTree(const xAOD::JetContainer &jets)
  {
    ATH_MSG_DEBUG("Filling large R jets tree.");

    // Being lazy here and not checking for pointer validity!
    for (const xAOD::Jet *jet : jets)
    {
      m_reco10JetEta.push_back(jet->eta());
      m_reco10JetPhi.push_back(jet->phi());
      m_reco10JetPt.push_back(jet->pt());
      m_reco10JetE.push_back(jet->e());
      m_reco10JetM.push_back(jet->m());
    }

    tree("Reco10Jets")->Fill();

    return StatusCode::SUCCESS;
  }

  StatusCode VariableDumperAlg ::
      fillMuonsTree(const xAOD::MuonContainer &muons)
  {
    ATH_MSG_DEBUG("Filling muons tree.");

    // Being lazy here and not checking for pointer validity!
    for (const xAOD::Muon *jet : muons)
    {
      m_muonEta.push_back(jet->eta());
      m_muonPhi.push_back(jet->phi());
      m_muonPt.push_back(jet->pt());
      m_muonE.push_back(jet->e());
      m_muonM.push_back(jet->m());
    }

    tree("Muons")->Fill();

    return StatusCode::SUCCESS;
  }

}
