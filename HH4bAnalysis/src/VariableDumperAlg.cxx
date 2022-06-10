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

    // m_systematicsList.addHandle(m_eventInfoHandle);
    m_systematicsList.addHandle(m_muonHandle);
    m_systematicsList.addHandle(m_jetsmallRHandle);
    m_systematicsList.addHandle(m_jetlargeRHandle);

    ATH_CHECK(m_EventInfoKey.initialize());
    ATH_CHECK(m_systematicsList.initialize());

    ATH_MSG_INFO("Will search \"" << m_EventInfoKey.key() << "\" for event info");

    return StatusCode::SUCCESS;
  }

  StatusCode VariableDumperAlg ::
      execute()
  {
    ATH_MSG_DEBUG("Executing " << name());

    for (const auto &sys : m_systematicsList.systematicsVector())
    {
      std::string sysname;
      ATH_CHECK(m_systematicsList.service().makeSystematicsName(sysname, "%SYS%", sys));
      // ATH_MSG_INFO("Will apply sysname \"" << sysname << "\" for event");
      SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
      ATH_CHECK(eventInfo.isValid());

      const xAOD::MuonContainer *muons(nullptr);
      ATH_CHECK(m_muonHandle.retrieve(muons, sys));

      const xAOD::JetContainer *AntiKt4RecoJets(nullptr);
      ANA_CHECK(m_jetsmallRHandle.retrieve(AntiKt4RecoJets, sys));

      const xAOD::JetContainer *AntiKt10RecoJets(nullptr);
      ANA_CHECK(m_jetlargeRHandle.retrieve(AntiKt10RecoJets, sys));
    }

    return StatusCode::SUCCESS;
  }
}
