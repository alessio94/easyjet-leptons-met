/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Victor Ruelas

//
// includes
//
#include "VariableDumperAlg.h"

//
// method implementations
//
namespace HH4B
{
  VariableDumperAlg ::VariableDumperAlg(const std::string &name,
                                        ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode VariableDumperAlg ::initialize()
  {
    ATH_MSG_DEBUG("Initialising " << name());

    ATH_CHECK(m_systematicsList.addHandle(m_electronHandle));
    ATH_CHECK(m_systematicsList.addHandle(m_photonHandle));
    ATH_CHECK(m_systematicsList.addHandle(m_muonHandle));
    ATH_CHECK(m_systematicsList.addHandle(m_jetsmallRHandle));
    ATH_CHECK(m_systematicsList.addHandle(m_VRtrackjetHandle));
    if (!m_jetlargeRHandle.empty())
    {
      ATH_CHECK(m_systematicsList.addHandle(m_jetlargeRHandle));
    }

    ATH_CHECK(m_systematicsList.initialize());

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
      ATH_MSG_VERBOSE("Will apply sysname \"" << sysname << "\" for event");

      // get the xAOD objects
      const xAOD::ElectronContainer *electrons(nullptr);
      ATH_CHECK(m_electronHandle.retrieve(electrons, sys));
      const xAOD::PhotonContainer *photons(nullptr);
      ATH_CHECK(m_photonHandle.retrieve(photons, sys));
      const xAOD::MuonContainer *muons(nullptr);
      ATH_CHECK(m_muonHandle.retrieve(muons, sys));
      const xAOD::JetContainer *antiKt4RecoJets(nullptr);
      ATH_CHECK(m_jetsmallRHandle.retrieve(antiKt4RecoJets, sys));
      const xAOD::JetContainer *antiKt10RecoJets(nullptr);
      if (!m_jetlargeRHandle.empty())
      {
        ATH_CHECK(m_jetlargeRHandle.retrieve(antiKt10RecoJets, sys));
      }
      const xAOD::JetContainer *VRTrackJets(nullptr);
      if (!m_VRtrackjetHandle.empty())
      {
        ATH_CHECK(m_VRtrackjetHandle.retrieve(VRTrackJets, sys));
      }
    }

    return StatusCode::SUCCESS;
  }
}
