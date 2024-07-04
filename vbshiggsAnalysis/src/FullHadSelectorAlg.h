/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VBSHIGGSANALYSIS_FULLHADSELECTORALG_H
#define VBSHIGGSANALYSIS_FULLHADSELECTORALG_H

#include <memory>

#include "AnaAlgorithm/AnaAlgorithm.h"

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>

#include <EasyjetHub/CutManager.h>


namespace VBSHIGGS{
  
  /// \brief An algorithm for counting containers
  class FullHadSelectorAlg final : public EL::AnaAlgorithm{
    public:
      FullHadSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

      /// \brief Initialisation method, for setting up tools and other persistent
      /// configs
      StatusCode initialize() override;
      /// \brief Execute method, for actions to be taken in the event loop
      StatusCode execute() override;
      /// \brief This is the mirror of initialize() and is called after all events are processed.
      StatusCode finalize() override; ///I added this to write the cutflow histogram.

      CP::SysListHandle m_systematicsList {this};
      CP::SysReadHandle<xAOD::JetContainer> m_jetHandle{ this, "jets", "vbshiggsAnalysisJets_%SYS%", "Jet container to read" };
      CP::SysReadHandle<xAOD::ElectronContainer> m_electronHandle{ this, "electrons", "vbshiggsAnalysisElectrons_%SYS%", "Electron container to read" };
      CP::SysReadHandle<xAOD::MuonContainer> m_muonHandle{ this, "muons", "vbshiggsAnalysisMuons_%SYS%", "Muon container to read" };

      Gaudi::Property<std::vector<std::string>> m_inputCutList{this, "cutList", {}};
      Gaudi::Property<bool> m_saveCutFlow{this, "saveCutFlow", false};
      Gaudi::Property<bool> m_bypass{ this, "bypass", false, "Run selector algorithm in pass-through mode" };
  };
}
#endif
