/*
 Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef EXANALYSIS_FINALMJJEXALG
#define EXANALYSIS_FINALMJJEXALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>

namespace EXANA
{
  /// \brief An algorithm for counting containers
  class MjjExampleAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    MjjExampleAlg(const std::string &name, ISvcLocator *pSvcLocator);
    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::JetContainer>
      m_JetHandle{ this, "Jets", "ExampleJets_%SYS%", "Jet container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    /// \brief Setup sys-aware output decorations
    CP::SysWriteDecorHandle<float> m_mjj {"example_mjj_%SYS%", this};
  };
}

#endif
