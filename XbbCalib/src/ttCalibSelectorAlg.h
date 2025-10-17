/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/// @author Derrick Allen
// Always protect against multiple includes!

#ifndef SELECTIONFLAGSTTCALIBALG_H
#define SELECTIONFLAGSTTCALIBALG_H

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysFilterReporterParams.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODMissingET/MissingETContainer.h>

namespace XBBCALIB
{

  /// \brief An algorithm for counting containers
  class ttCalibSelectorAlg final : public AthHistogramAlgorithm {

    public:
      ttCalibSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

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
      Gaudi::Property<bool> m_bypass
        { this, "bypass", false, "Run selector algorithm in pass-through mode" };

      /// \brief Setup syst-aware input container handles
      CP::SysListHandle m_systematicsList {this};

      CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

      CP::SysReadHandle<xAOD::JetContainer>
      m_jetHandle{ this, "jets", "ttCalibJets_%SYS%",   "Jet container to read" };

      CP::SysReadHandle<xAOD::JetContainer>
      m_lrjetHandle{ this, "lrjets", "ttCalibLRJets_%SYS%",   "Large-R jet container to read" };

      CP::SysReadHandle<xAOD::ElectronContainer>
      m_electronHandle{ this, "electrons", "ttCalibElectrons_%SYS%",   "Electron container to read" };

      CP::SysReadHandle<xAOD::MuonContainer>
      m_muonHandle{ this, "muons", "ttCalibMuons_%SYS%",   "Muon container to read" };

      CP::SysReadHandle<xAOD::MissingETContainer>
      m_metHandle{ this, "met", "AnalysisMET_%SYS%",   "MET container to read" };

      Gaudi::Property<float> m_minMet
      {this, "minMet", 20000, "Minimum MET cut"};

      Gaudi::Property<std::string> m_eleWPName
      { this, "eleWP", "","Electron ID + Iso working point" };
      CP::SysReadDecorHandle<char> m_eleWPDecorHandle{"", this};

      Gaudi::Property<std::string> m_muonWPName
        { this, "muonWP", "","Muon ID + Iso cuts" };
      CP::SysReadDecorHandle<char> m_muonWPDecorHandle{"", this};

      CP::SysFilterReporterParams m_filterParams {this, "ttCalib selection"};

  };

}

#endif // SELECTIONFLAGSXBBCALIBALG_H
