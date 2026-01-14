/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!

#ifndef SELECTIONFLAGSZllyCaLIBALG_H
#define SELECTIONFLAGSZllyCaLIBALG_H

#include <memory>

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <AsgDataHandles/ReadDecorHandleKey.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysFilterReporterParams.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/PhotonContainer.h>
#include <EasyjetHub/CutManager.h>
#include "TriggerMatchingTool/IMatchingTool.h"
#include "XbbCalibEnums.h"

namespace XBBCALIB
{
  /// \brief An algorithm for counting containers
  class ZllyCalibSelectorAlg final : public AthHistogramAlgorithm {

    public:
      ZllyCalibSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

      /// \brief Initialisation method, for setting up tools and other persistent
      /// configs
      StatusCode initialize() override;
      /// \brief Execute method, for actions to be taken in the event loop
      StatusCode execute() override;
      /// \brief This is the mirror of initialize() and is called after all events are processed.
      StatusCode finalize() override; ///I added this to write the cutflow histogram.

    private :
      // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
      // "someInfo"};
      const std::vector<std::string> m_STANDARD_CUTS{
        "PASS_EXACTLY_ONE_PHOTON",
        "PASS_TWO_SF_LEPTONS",
      };
      StatusCode initialiseCutflow();

      CutManager m_ZllyCalibCuts; 
      Gaudi::Property<bool> m_bypass
        { this, "bypass", false, "Run selector algorithm in pass-through mode" };

      /// \brief Setup syst-aware input container handles
      CP::SysListHandle m_systematicsList {this};

      CP::SysReadHandle<xAOD::JetContainer>
      m_jetHandle{ this, "jets", "XbbCalibJets_%SYS%",   "Jet container to read" };

      CP::SysReadHandle<xAOD::JetContainer>
      m_lrjetHandle{ this, "lrjets", "XbbCalibLRJets_%SYS%",   "Large-R jet container to read" };

      CP::SysReadHandle<xAOD::ElectronContainer>
      m_electronHandle{ this, "electrons", "XbbCalibElectrons_%SYS%", "Electron container to read" };

      CP::SysReadHandle<xAOD::MuonContainer>
      m_muonHandle{ this, "muons", "XbbCalibMuons_%SYS%", "Muon container to read" };

      CP::SysReadHandle<xAOD::PhotonContainer>
      m_photonHandle{ this, "photons", "XbbCalibPhotons_%SYS%", "Photon container to read" };

      CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

      CP::SysFilterReporterParams m_filterParams {this, "XbbCalib selection"};

      Gaudi::Property<std::vector<std::string>> m_inputCutList{this, "cutList", {}};
      std::vector<XBBCALIB::Booleans> m_inputCutKeys;
      Gaudi::Property<bool> m_saveCutFlow{this, "saveCutFlow", false};

      long long int m_total_events{0};

      std::unordered_map<XBBCALIB::Booleans, CP::SysWriteDecorHandle<bool> > m_Bbranches;
      std::unordered_map<XBBCALIB::Booleans, bool> m_bools;
      CP::SysWriteDecorHandle<bool> m_passallcuts {"PassAllCuts_%SYS%", this};
      std::unordered_map<XBBCALIB::Booleans, std::string> m_boolnames{
      // {XBBCALIB::PASS_TRIGGER, "PASS_TRIGGER"},
      {XBBCALIB::PASS_EXACTLY_ONE_PHOTON, "PASS_EXACTLY_ONE_PHOTON"},
      {XBBCALIB::PASS_TWO_SF_LEPTONS, "PASS_TWO_SF_LEPTONS"},
      };

  };

}

#endif // SELECTIONFLAGSZllyCaLIBALG_H
