/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HHBBLLANALYSIS_HHBBLLSELECTORALG
#define HHBBLLANALYSIS_HHBBLLSELECTORALG

#include <memory>

#include "AnaAlgorithm/AnaAlgorithm.h"
#include <FourMomUtils/xAODP4Helpers.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/ISysHandleBase.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMissingET/MissingETContainer.h>

#include <SystematicsHandles/SysFilterReporterParams.h>

namespace HHBBLL
{

  /// \brief An algorithm for counting containers
  class HHbbllSelectorAlg final : public EL::AnaAlgorithm
  {
    /// \brief The standard constructor
public:
    HHbbllSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// \brief Finalisation method, for cleanup, final print out etc
    StatusCode finalize() override;

private:

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::JetContainer>
      m_jetHandle{ this, "jets", "",   "Jet container to read" };
    
    CP::SysReadHandle<xAOD::ElectronContainer>
    m_electronHandle{ this, "electrons", "",   "Electron container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
    m_muonHandle{ this, "muons", "",   "Muon container to read" };

    CP::SysReadHandle<xAOD::MissingETContainer>
    m_metHandle{ this, "met", "AnalysisMET",   "MET container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

    /// \brief Setup sys-aware output decorations
    CP::SysWriteDecorHandle<bool> m_pass_sr {"pass_bbll_sr_%SYS%", this};

    CP::SysFilterReporterParams m_filterParams {this, "HHbbll selection"};

    /// \brief Steerable properties
    CP::SysReadDecorHandle<char> 
    m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};
    /// \brief Internal variables

  };
}

#endif
