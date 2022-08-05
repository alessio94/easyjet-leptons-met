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
      ATH_CHECK(m_jetsmallRHandle.retrieve(antiKt4RecoJets, sys));
      // do something with antiKt4RecoJets
      for (auto jet : *antiKt4RecoJets)
      {
        const xAOD::BTagging *bjet =
            xAOD::BTaggingUtilities::getBTagging(*jet);

        if (!bjet)
        {
          ATH_MSG_WARNING("btagging information not available");
          continue;
        }

        double DL1dv00_pb = -1;
        double DL1dv00_pc = -1;
        double DL1dv00_pu = -1;
        bjet->pb("DL1dv00", DL1dv00_pb);
        bjet->pc("DL1dv00", DL1dv00_pc);
        bjet->pu("DL1dv00", DL1dv00_pu);
        ATH_MSG_WARNING("DL1dv00_pb \"" << DL1dv00_pb << "\" for jet");
        ATH_MSG_WARNING("DL1dv00_pc \"" << DL1dv00_pc << "\" for jet");
        ATH_MSG_WARNING("DL1dv00_pu \"" << DL1dv00_pu << "\" for jet");
        jet->auxdecor<double>("DL1dv00_pb") = DL1dv00_pb;
      }

      if (!m_jetlargeRHandle.empty())
      {
        const xAOD::JetContainer *antiKt10RecoJets(nullptr);
        ATH_CHECK(m_jetlargeRHandle.retrieve(antiKt10RecoJets, sys));
        // do something with antiKt10RecoJets
      }
    }

    return StatusCode::SUCCESS;
  }
}
