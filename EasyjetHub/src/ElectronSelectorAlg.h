/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HH4BANALYSIS_ELECTRONSELECTORALG
#define HH4BANALYSIS_ELECTRONSELECTORALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysListHandle.h>

#include <AthContainers/ConstDataVector.h>
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

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

    CP::SysReadHandle<xAOD::ElectronContainer>
    m_inHandle{ this, "containerInKey", "",   "Electron container to read" };

    /// \brief Setup syst-aware output container handles
    CP::SysWriteHandle<ConstDataVector<xAOD::ElectronContainer>>
    m_outHandle{ this, "containerOutKey", "",   "Electron container to write" };

    /// \brief Setup sys-aware output decorations
    CP::SysWriteDecorHandle<int> m_nSelPart {this, "decorOutName", "nElecrons_%SYS%", 
        "Name out output decorator for number of selected electrons"};

    float m_minPt;
    float m_minEtaVeto;
    float m_maxEtaVeto;
    float m_maxEta;
    int m_minimumAmount;
    int m_truncateAtAmount;
    bool m_pTsort;
  };
}

#endif
