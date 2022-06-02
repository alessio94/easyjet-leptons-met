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
    StatusCode bookEventInfoTree();
    StatusCode bookReco4JetsTree();
    StatusCode bookReco10JetsTree();
    StatusCode bookMuonsTree();

    // Call in execute to fill the EventInfo and jet variables
    StatusCode fillEventInfoTree(const xAOD::EventInfo &);
    StatusCode fillReco4JetsTree(const xAOD::JetContainer &);
    StatusCode fillReco10JetsTree(const xAOD::JetContainer &);
    StatusCode fillMuonsTree(const xAOD::MuonContainer &);

    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue", "someInfo"};
    ToolHandle<IBTaggingSelectionTool> m_btagSelTool{this, "BTaggingSelectionTool", {}, "Tool to select b-jets"};

    // Member variables for configuration
    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{this, "EventInfoKey", "", "EventInfo container to dump"};
    SG::ReadHandleKey<xAOD::JetContainer> m_Reco4JetsKey{this, "Reco4JetsKey", "", "Reconstructed small R jets container to dump"};
    SG::ReadHandleKey<xAOD::JetContainer> m_Reco10JetsKey{this, "Reco10JetsKey", "", "Reconstructed large R jets container to dump"};
    SG::ReadHandleKey<xAOD::MuonContainer> m_MuonsKey{this, "MuonsKey", "", "Muons container to dump"};

    // output variables for the current event
    unsigned int m_runNumber = 0;
    unsigned long long m_eventNumber = 0;
    // Small R jet variables
    std::vector<float> m_reco4JetEta;
    std::vector<float> m_reco4JetPhi;
    std::vector<float> m_reco4JetPt;
    std::vector<float> m_reco4JetE;
    std::vector<float> m_reco4JetM;
    // b-tagged jet variables
    std::vector<float> m_reco4BTagJetEta;
    std::vector<float> m_reco4BTagJetPhi;
    std::vector<float> m_reco4BTagJetPt;
    std::vector<float> m_reco4BTagJetE;
    std::vector<float> m_reco4BTagJetM;

    // Large R jet variables
    std::vector<float> m_reco10JetEta;
    std::vector<float> m_reco10JetPhi;
    std::vector<float> m_reco10JetPt;
    std::vector<float> m_reco10JetE;
    std::vector<float> m_reco10JetM;

    // Muon variables
    std::vector<float> m_muonEta;
    std::vector<float> m_muonPhi;
    std::vector<float> m_muonPt;
    std::vector<float> m_muonE;
    std::vector<float> m_muonM;
  };
}

#endif
