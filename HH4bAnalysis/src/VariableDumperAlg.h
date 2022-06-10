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
    // CP::SysReadHandle<xAOD::EventInfo> m_eventInfoHandle{
    //     this, "EventInfoInKey", "EventInfo_%SYS%", "the event info collection to run on"};
    CP::SysReadHandle<xAOD::MuonContainer> m_muonHandle{
        this, "muons", "AnalysisMuons_%SYS%", "the muon collection to run on"};
    CP::SysReadHandle<xAOD::JetContainer> m_jetsmallRHandle{this, "jetsmallR", "AnalysisJetsBTAG_%SYS%", "the small-R jet collection to run on"};
    CP::SysReadHandle<xAOD::JetContainer> m_jetlargeRHandle{this, "jetlargeR", "AnalysisLargeRRecoJets_%SYS%", "the large-R jet collection to run on"};

    // Member variables for configuration
    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{this, "EventInfoKey", "", "EventInfo container to dump"};

    // output variables for the current event
    unsigned int m_runNumber = 0;
    unsigned long long m_eventNumber = 0;
    float m_mcEventWeight = 0;
  };
}

#endif
