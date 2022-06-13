///////////////////////// -*- C++ -*- /////////////////////////////
// VariableDumperAlg.h
//
// This is an algorithm that will dump variables into a tree.
//
// Author: Victor Ruelas<victor.hugo.ruelas.rivera@cern.ch>
///////////////////////////////////////////////////////////////////

// Always protect against multiple includes!
#ifndef HH4BANALYSIS_VARIABLEDUMPERALG
#define HH4BANALYSIS_VARIABLEDUMPERALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODEgamma/PhotonContainer.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>

#include <AsgTools/ToolHandle.h>
#include "FTagAnalysisInterfaces/IBTaggingSelectionTool.h"

namespace HH4B
{

  /// \brief An algorithm for plotting dilepton masses
  class VariableDumperAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
  public:
    VariableDumperAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

  private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue", "someInfo"};
    ToolHandle<IBTaggingSelectionTool> m_btagSelTool{this, "BTaggingSelectionTool", {}, "Tool to select b-jets"};

    CP::SysListHandle m_systematicsList{this};

    // Member variables for configuration
    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{this, "EventInfoKey", "", "EventInfo container to dump"};

    CP::SysReadHandle<xAOD::ElectronContainer> m_electronHandle{
        this, "electrons", "AnalysisElectrons_%SYS%", "the electron collection to run on"};

    CP::SysReadHandle<xAOD::PhotonContainer> m_photonHandle{
        this, "photons", "AnalysisPhotons_%SYS%", "the photon collection to run on"};

    CP::SysReadHandle<xAOD::MuonContainer> m_muonHandle{
        this, "muons", "AnalysisMuons_%SYS%", "the muon collection to run on"};

    CP::SysReadHandle<xAOD::JetContainer> m_jetsmallRHandle{this, "jetsmallR", "AnalysisJetsBTAG_%SYS%", "the small-R jet collection to run on"};

    CP::SysReadHandle<xAOD::JetContainer> m_jetlargeRHandle{this, "jetlargeR", "AnalysisLargeRRecoJets_%SYS%", "the large-R jet collection to run on"};

    // output variables for the current event
    unsigned int m_runNumber = 0;
    unsigned long long m_eventNumber = 0;
    float m_mcEventWeight = 0;
  };
}

#endif
