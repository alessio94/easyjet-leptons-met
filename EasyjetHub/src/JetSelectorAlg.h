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

    CP::SysReadDecorHandle<int> m_PCBT {this, "PCBTDecorName", "", "Name of pseudo-continuous b-tagging decorator"};

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


    Gaudi::Property<float> m_minPt            {this, "minPt", 25e3, "Minimum pT of jets"};
    Gaudi::Property<float> m_maxEta           {this, "maxEta", 4.4, "Maximum eta of jets"}; // default is central jets
    Gaudi::Property<int>   m_minimumAmount    {this, "minimumAmount", -1, "Minimum number of jets to consider"}; // -1 means ignores this
    Gaudi::Property<int>   m_maximumAmount    {this, "maximumAmount", -1, "Maximum number of jets to consider"};
    Gaudi::Property<bool>  m_pTsort           {this, "pTsort", true, "Sort jets by pT"};
    Gaudi::Property<bool>  m_PCBTsort         {this, "PCBTsort", false, "Sort jets by PCBT scores"};
    Gaudi::Property<int>   m_truncateAtAmount {this, "truncateAtAmount", -1, "Remove extra jets after pT sorting"}; // -1 means keep them all
    Gaudi::Property<bool>  m_removeRelativeDeltaRToVRJet {this, "removeRelativeDeltaRToVRJet", false, "Remove events in which VR jets overlaps"};
    Gaudi::Property<bool>  m_checkOR          {this, "checkOR", true, "Check the Overlap Removal"};

  };
}

#endif
