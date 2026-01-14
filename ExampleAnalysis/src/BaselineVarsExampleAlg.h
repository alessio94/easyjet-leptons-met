/*
 Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef EXANALYSIS_FINALVARSEXALG
#define EXANALYSIS_FINALVARSEXALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/PhotonContainer.h>

namespace EXANA
{
  /// \brief An algorithm for counting containers
  class BaselineVarsExampleAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    BaselineVarsExampleAlg(const std::string &name, ISvcLocator *pSvcLocator);
    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    CP::SysReadHandle<xAOD::JetContainer>
      m_JetHandle{ this, "Jets", "ExampleJets_%SYS%", "Jet container to read" };

    //CP::SysReadHandle<xAOD::JetContainer>
    //  m_LargeRJetHandle{ this, "LargeJets", "ExampleLargeRJets_%SYS%", "Large-R jets container to read" };

    CP::SysReadHandle<xAOD::PhotonContainer>
      m_photonHandle{ this, "photons", "ExamplePhotons_%SYS%", "Photon container to read" };

    Gaudi::Property<std::vector<std::string>> m_floatVariables
          {this, "floatVariableList", {}, "Name list of floating variables"};

    Gaudi::Property<std::vector<std::string>> m_intVariables
          {this, "intVariableList", {}, "Name list of integer variables"};

    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_Fbranches;
    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;
  };
}

#endif
