/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EASYJET_LEPTONORDERINGALG
#define EASYJET_LEPTONORDERINGALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysListHandle.h>

#include <AthContainers/ConstDataVector.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>

namespace Easyjet
{

  /// \brief An algorithm for counting containers
  class LeptonOrderingAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    LeptonOrderingAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};
    Gaudi::Property<int>   m_leptonAmount       {this, "leptonAmount", -1, "Number of leptons to consider for isLeptonXX decoration"};

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

    CP::SysReadHandle<xAOD::ElectronContainer>
    m_inEleHandle{ this, "containerInEleKey", "",   "Electron container to read" };
    CP::SysReadHandle<xAOD::MuonContainer>
    m_inMuHandle{ this, "containerInMuKey", "",   "Muon container to read" };

    CP::SysReadDecorHandle<bool> m_isSelectedElectron{"isAnalysisElectron_%SYS%", this};
    CP::SysReadDecorHandle<bool> m_isSelectedMuon{"isAnalysisMuon_%SYS%", this};

    /// \brief Setup sys-aware output decorations
    CP::SysWriteDecorHandleArray<bool> m_leadBranchesEle{{}, this};
    CP::SysWriteDecorHandleArray<bool> m_leadBranchesMu{{}, this};
  };
}

#endif
