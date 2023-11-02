/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef TTHHANALYSIS_JETPAIRINGALG
#define TTHHANALYSIS_JETPAIRINGRALG

#include "xAODJet/JetContainer.h"
#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>

namespace ttHH
{

  /// \brief An algorithm for counting containers
  class JetPairingAlgttHH final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    JetPairingAlgttHH(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    std::tuple<std::vector<const xAOD::Jet*>, float> bJetChiSquarePairing(const ConstDataVector<xAOD::JetContainer>& Jets, float target_mass_1, float target_mass_2);

    std::tuple<const xAOD::Jet*, const xAOD::Jet*, const xAOD::Jet*, const xAOD::Jet*, float> minChiSquared(const ConstDataVector<xAOD::JetContainer>& Jets, const std::vector<size_t>& indexes, float target_mass_1, float target_mass_2);

    SG::ReadHandleKey<ConstDataVector<xAOD::JetContainer>> m_containerInKey{
        this, "containerInKey", "", "containerName to read"};
    SG::WriteHandleKey<ConstDataVector<xAOD::JetContainer>> m_containerOutKey{
        this, "containerOutKey", "", "containerName to write"};

    std::string m_pairingStrategy;
  };
}

#endif
