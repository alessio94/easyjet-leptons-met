/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HH4BANALYSIS_JETPAIRINGALG
#define HH4BANALYSIS_JETPAIRINGRALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

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

    SG::ReadHandleKey<ConstDataVector<xAOD::JetContainer>> m_containerInKey{
        this, "containerInKey", "", "containerName to read"};
    SG::WriteHandleKey<ConstDataVector<xAOD::JetContainer>> m_containerOutKey{
        this, "containerOutKey", "", "containerName to write"};

    std::string m_pairingStrategy;
  };
}

#endif
