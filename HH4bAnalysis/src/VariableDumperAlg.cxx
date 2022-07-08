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
  VariableDumperAlg ::VariableDumperAlg(const std::string &name,
                                        ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator),
        m_acc_DFCommonJets_eventClean_LooseBad(
            "DFCommonJets_eventClean_LooseBad")
  {
    declareProperty("applyJetCleaning", m_applyJetCleaning);
  }

  StatusCode VariableDumperAlg ::initialize()
  {
    ATH_MSG_DEBUG("Initialising " << name());

    if (m_EventInfoKey.empty())
    {
      ATH_MSG_ERROR("No input collection provided for EventInfo!");
      return StatusCode::FAILURE;
    }

    ATH_CHECK(m_systematicsList.addHandle(m_electronHandle));
    ATH_CHECK(m_systematicsList.addHandle(m_photonHandle));
    ATH_CHECK(m_systematicsList.addHandle(m_muonHandle));
    ATH_CHECK(m_systematicsList.addHandle(m_jetsmallRHandle));
    ATH_CHECK(m_systematicsList.addHandle(m_jetlargeRHandle));

    ATH_CHECK(m_EventInfoKey.initialize());
    ATH_CHECK(m_systematicsList.initialize());

    ATH_MSG_INFO("Will search \"" << m_EventInfoKey.key()
                                  << "\" for event info");

    return StatusCode::SUCCESS;
  }

  StatusCode VariableDumperAlg ::execute()
  {
    ATH_MSG_DEBUG("Executing " << name());

    for (const auto &sys : m_systematicsList.systematicsVector())
    {
      std::string sysname;
      ATH_CHECK(m_systematicsList.service().makeSystematicsName(sysname,
                                                                "%SYS%", sys));
      ATH_MSG_INFO("Will apply sysname \"" << sysname << "\" for event");
      SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
      ATH_CHECK(eventInfo.isValid());
      // jet cleaning
      if (m_applyJetCleaning)
      {
        if (!m_acc_DFCommonJets_eventClean_LooseBad(*eventInfo))
        {
          return StatusCode::SUCCESS; // go to next event
        }
      }

      const xAOD::ElectronContainer *electrons(nullptr);
      ATH_CHECK(m_electronHandle.retrieve(electrons, sys));
      // do something with electrons
      const xAOD::PhotonContainer *photons(nullptr);
      ATH_CHECK(m_photonHandle.retrieve(photons, sys));
      // do something with electrons
      const xAOD::MuonContainer *muons(nullptr);
      ATH_CHECK(m_muonHandle.retrieve(muons, sys));
      // do something with muons
      const xAOD::JetContainer *antiKt4RecoJets(nullptr);
      ANA_CHECK(m_jetsmallRHandle.retrieve(antiKt4RecoJets, sys));
      // do something with antiKt4RecoJets
      const xAOD::JetContainer *antiKt10RecoJets(nullptr);
      ANA_CHECK(m_jetlargeRHandle.retrieve(antiKt10RecoJets, sys));
      // do something with antiKt10RecoJets
    }

    return StatusCode::SUCCESS;
  }
}
