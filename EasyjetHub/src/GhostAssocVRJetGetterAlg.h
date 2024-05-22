/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef EASYJET_GHOSTASSOCVRJETGETTERALG
#define EASYJET_GHOSTASSOCVRJETGETTERALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <xAODJet/JetContainer.h>
#include <AthContainers/ConstDataVector.h>

namespace Easyjet
{

  /// \brief An algorithm for counting containers
  class GhostAssocVRJetGetterAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    GhostAssocVRJetGetterAlg(const std::string &name,
                             ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    SG::ReadHandleKey<xAOD::JetContainer> m_containerInKey{
        this, "containerInKey", "", "containerName to read"};
    SG::WriteHandleKey<ConstDataVector<xAOD::JetContainer>> m_containerOutKey{
        this, "containerOutKey", "", "containerName to write"};

    // ghost associated VR track jets are only on the untrimmed 1.0 jets
    Gaudi::Property<unsigned int> m_whichJet {this, "whichJet", -1, "Index of jets to be trimmed"};
  };
}

#endif
