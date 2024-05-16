/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VBSHIGGSANALYSIS_SEMILEPSELECTORALG_H
#define VBSHIGGSANALYSIS_SEMILEPSELECTORALG_H

#include <memory>

#include "AnaAlgorithm/AnaAlgorithm.h"
#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AsgDataHandles/ReadDecorHandle.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysFilterReporterParams.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMissingET/MissingETContainer.h>

#include <EasyjetHub/CutManager.h>


namespace VBSHIGGS{
  
  /// \brief An algorithm for counting containers
  class SemiLepSelectorAlg final : public EL::AnaAlgorithm{
    public:
      SemiLepSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

      /// \brief Initialisation method, for setting up tools and other persistent
      /// configs
      StatusCode initialize() override;
      /// \brief Execute method, for actions to be taken in the event loop
      StatusCode execute() override;
      /// \brief This is the mirror of initialize() and is called after all events are processed.
      StatusCode finalize() override; ///I added this to write the cutflow histogram.

      CP::SysListHandle m_systematicsList {this};

      CP::SysReadHandle<xAOD::JetContainer> m_jetHandle{ this, "jets", "",   "Jet container to read" };
      CP::SysReadDecorHandle<char>  m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};
      CP::SysReadHandle<xAOD::EventInfo> m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };
      CP::SysReadHandle<xAOD::ElectronContainer> m_electronHandle{ this, "electrons", "",   "Electron container to read" };
      CP::SysReadHandle<xAOD::MuonContainer> m_muonHandle{ this, "muons", "",   "Muon container to read" }; 
      CP::SysReadHandle<xAOD::MissingETContainer> m_metHandle{ this, "met", "AnalysisMET",   "MET container to read" };

      Gaudi::Property<std::vector<std::string>> m_inputCutList{this, "cutList", {}};
      Gaudi::Property<bool> m_saveCutFlow{this, "saveCutFlow", false};
      Gaudi::Property<bool> m_bypass{ this, "bypass", false, "Run selector algorithm in pass-through mode" };

  };
}
#endif