/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EASYJET_TAUSELECTORALG
#define EASYJET_TAUSELECTORALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysListHandle.h>

#include <AthContainers/ConstDataVector.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODTau/TauJetContainer.h>

namespace Easyjet
{

  /// \brief An algorithm for counting containers
  class TauSelectorAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    TauSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

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

    CP::SysReadHandle<xAOD::TauJetContainer>
    m_inHandle{ this, "containerInKey", "",   "Tau container to read" };

    Gaudi::Property<std::string> m_IDTauDecorName
      { this, "idTauDecorKey", "isIDTau", "Decoration for ID taus" };
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_IDTauDecorKey;

    Gaudi::Property<std::string> m_antiTauDecorName
      { this, "antiTauDecorKey", "isAntiTau", "Decoration for anti-taus" };
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_antiTauDecorKey;

    Gaudi::Property<std::string> m_ORTauDecorName
      { this, "ORDecorKey", "passesOR", "Decoration for ID taus" };
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_ORTauDecorKey;

    /// \brief Setup syst-aware output container handles
    CP::SysWriteHandle<ConstDataVector<xAOD::TauJetContainer>>
    m_outHandle{ this, "containerOutKey", "",   "Tau container to write" };

    /// \brief Setup sys-aware output decorations
    CP::SysWriteDecorHandle<int>
      m_nSelPart{ this, "decorOutName", "nTaus_%SYS%",
	  "Name out output decorator for number of selected taus" };

    float m_minPt;
    float m_minEtaVeto;
    float m_maxEtaVeto;
    float m_maxEta;
    int m_minimumAmount;
    int m_truncateAtAmount;
    bool m_pTsort;
    bool m_checkOR;
  };
}

#endif
