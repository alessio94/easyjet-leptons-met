/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef EASYJET_JETSELECTORALG
#define EASYJET_JETSELECTORALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysListHandle.h>

#include <AthContainers/ConstDataVector.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>

namespace Easyjet
{

  /// \brief An algorithm for counting containers
  class JetSelectorAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    JetSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

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

    CP::SysReadHandle<xAOD::JetContainer>
    m_inHandle{ this, "containerInKey", "",   "Jet container to read" };

    // \brief Setup syst-aware input decorations
    CP::SysReadDecorHandle<char> m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};
    Gaudi::Property<std::string> m_ORJetDecorName
      { this, "ORDecorKey", "passesOR", "Decoration for jets OR" };
    SG::ReadDecorHandleKey<xAOD::JetContainer> m_ORJetDecorKey;

    CP::SysReadDecorHandle<float> m_relativeDeltaRToVRJet {"relativeDeltaRToVRJet", this};

    /// \brief Setup syst-aware output container handles
    CP::SysWriteHandle<ConstDataVector<xAOD::JetContainer>>
    m_outHandle{ this, "containerOutKey", "",   "Jet container to write" };

    /// \brief Setup sys-aware output decorations
    CP::SysWriteDecorHandle<int> m_nSelPart {this, "decorOutName", "nJets_%SYS%", 
        "Name of output decorator for number of selected jets"};

    float m_minPt;
    float m_maxEta;
    int m_minimumAmount;
    int m_maximumAmount;
    int m_truncateAtAmount;
    bool m_pTsort;
    bool m_removeRelativeDeltaRToVRJet;
    bool m_checkOR;
  };
}

#endif
