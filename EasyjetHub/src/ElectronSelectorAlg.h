/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HH4BANALYSIS_ELECTRONSELECTORALG
#define HH4BANALYSIS_ELECTRONSELECTORALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODEgamma/ElectronContainer.h>

namespace Easyjet
{

  /// \brief An algorithm for counting containers
  class ElectronSelectorAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    ElectronSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    SG::ReadHandleKey<xAOD::ElectronContainer> m_containerInKey{
      this, "containerInKey", "", "containerName to read"
    };
    SG::WriteHandleKey<ConstDataVector<xAOD::ElectronContainer> >
    m_containerOutKey{ this, "containerOutKey", "", "containerName to write" };
    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{
      this, "EventInfoKey", "EventInfo", "EventInfo container to dump"
    };

    float m_minPt;
    float m_maxEta;
    int m_minimumAmount;
    int m_truncateAtAmount;
    bool m_pTsort;
  };
}

#endif
