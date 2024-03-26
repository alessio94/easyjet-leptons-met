/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EASYJET_ELECTRONSELECTORALG
#define EASYJET_ELECTRONSELECTORALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
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

    CP::SysReadDecorHandle<char> m_passesOR{"passesOR_%SYS%", this};

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    Gaudi::Property<std::string> m_eleWPName
      { this, "ele_WP", "","Electron ID + Iso working point" };
    bool m_isoIncluded = true;
    CP::SysReadDecorHandle<float> m_ele_recoSF{"", this};
    CP::SysReadDecorHandle<float> m_ele_idSF{"", this};
    CP::SysReadDecorHandle<float> m_ele_isoSF{"", this};
    CP::SysWriteDecorHandle<float> m_ele_SF{"", this};

    CP::SysReadDecorHandle<char> m_select_in{"", this};
    CP::SysWriteDecorHandle<char> m_select_out{"", this};

    /// \brief Setup syst-aware output container handles
    CP::SysWriteHandle<ConstDataVector<xAOD::ElectronContainer>>
    m_outHandle{ this, "containerOutKey", "",   "Electron container to write" };

    /// \brief Setup sys-aware output decorations
    CP::SysWriteDecorHandle<int> m_nSelPart {this, "decorOutName", "nElectrons_%SYS%", 
        "Name out output decorator for number of selected electrons"};

    Gaudi::Property<float> m_minPt            {this, "minPt", 7e3, "Minimum pT of electrons"};
    Gaudi::Property<float> m_minEtaVeto       {this, "minEtaVeto", 1.37, "Minimum eta veto of EMCal"};
    Gaudi::Property<float> m_maxEtaVeto       {this, "maxEtaVeto", 1.52, "Maximum eta veto of EMCal"};
    Gaudi::Property<float> m_maxEta           {this, "maxEta", 2.47, "Maximum eta of electrons"};
    Gaudi::Property<int>   m_minimumAmount    {this, "minimumAmount", -1, "Minimum number of electrons to consider"}; // -1 means ignores this
    Gaudi::Property<bool>  m_pTsort           {this, "pTsort", true, "Sort electrons by pT"};
    Gaudi::Property<int>   m_truncateAtAmount {this, "truncateAtAmount", -1, "Remove extra electrons after pT sorting"}; // -1 means keep them all
    Gaudi::Property<bool>  m_checkOR          {this, "checkOR", true, "Check the Overlap Removal"};
  };
}

#endif
