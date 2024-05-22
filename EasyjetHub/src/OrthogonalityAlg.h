/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EASYJET_ORTHOGONALITYALG
#define EASYJET_ORTHOGONALITYALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysListHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/PhotonContainer.h>


namespace Easyjet
{

  /// \brief An algorithm for counting containers
  class OrthogonalityAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    OrthogonalityAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

    CP::SysReadHandle<xAOD::PhotonContainer>
    m_photonHandle{ this, "photons", "",   "Photons container to read" };

    CP::SysReadHandle<xAOD::JetContainer>
    m_jetHandle{ this, "bjets", "",   "Jet container to read" };

    std::unordered_map<std::string, CP::SysWriteDecorHandle<bool> > m_Bbranches;
    std::vector<std::string> m_Bvarnames{
      "orth_pass_bbyy", "orth_pass_bbbb", "orth_pass_bbtt"
    };

  };
}

#endif
