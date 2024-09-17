/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HH4BANALYSIS_JETPAIRINGALG
#define HH4BANALYSIS_JETPAIRINGRALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>
#include <SystematicsHandles/SysListHandle.h>

#include <xAODJet/JetContainer.h>

namespace HH4B
{

  /// \brief An algorithm for counting containers
  class JetPairingAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    JetPairingAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    CP::SysListHandle m_systematicsList {this};
    CP::SysReadHandle<xAOD::JetContainer>
      m_inJetHandle{this, "containerInKey", "", "Input jet container to read"};
    CP::SysWriteHandle<ConstDataVector<xAOD::JetContainer>>
      m_outJetHandle{this, "containerOutKey", "", "Output jet container to write"};

    Gaudi::Property<std::string> m_pairingStrategy {this, "pairingStrategy", "", "Pairing strategy."};
  };
}

#endif
