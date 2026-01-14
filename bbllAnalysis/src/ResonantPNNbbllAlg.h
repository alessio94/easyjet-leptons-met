/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HHBBLLANALYSIS__ResonantPNNbbllAlg
#define HHBBLLANALYSIS__ResonantPNNbbllAlg 

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMissingET/MissingETContainer.h>
#include "lwtnn/LightweightGraph.hh"
#include "lwtnn/LightweightNeuralNetwork.hh"
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <iostream>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/member.hpp>
#include "NeutrinoWeightingTool.h"
#include <xAODTruth/TruthParticleContainer.h>
using namespace lwt;
namespace HHBBLL
{

  /// \brief An algorithm for counting containers
  class ResonantPNNbbllAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
  public:
    ResonantPNNbbllAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any
    void LoadPDNN_prepare(size_t total_inputs, const xAOD::EventInfo& event, const xAOD::ElectronContainer& electrons, const xAOD::MuonContainer& muons, const ConstDataVector<xAOD::JetContainer>& bjets, const xAOD::MissingET& met,const CP::SystematicSet& sys);

  private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};
    CP::SysReadDecorHandle<unsigned int> m_year
	{this, "year", "dataTakingYear", ""};
    CP::SysReadDecorHandle<float>
      m_met_sig {this, "METSignificance", "significance_%SYS%", "Met Significance"};

    CP::SysReadHandle<xAOD::JetContainer>
      m_jetHandle{ this, "jets", "bbllAnalysisJets_%SYS%", "Jet container to read" };

    CP::SysReadDecorHandle<char> 
      m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};
    
    CP::SysReadHandle<xAOD::ElectronContainer>
      m_electronHandle{ this, "electrons", "bbllAnalysisElectrons_%SYS%", "Electron container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
      m_muonHandle{ this, "muons", "bbllAnalysisMuons_%SYS%", "Muon container to read" };

    CP::SysReadHandle<xAOD::MissingETContainer>
    m_metHandle{ this, "met", "AnalysisMET_%SYS%", "MET container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    Gaudi::Property <std::vector<double>> m_mX_values {this, "mX_values", {}, "mX values"};

    Gaudi::Property<std::vector<std::string>> m_floatVariables
      { this, "floatPNNVariables", {}, "List of float variables to write out" };

    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>>
      m_Fbranches;

    CP::SysReadDecorHandle<float> m_NW{"NW_neutrinoweight_%SYS%", this};
    CP::SysReadDecorHandle<float> m_mT2bb{"mT2_bb_%SYS%", this};
    Gaudi::Property<bool> m_run_Run2_train
      { this, "run_Run2_train", false, "use either the Run2 or Run3 PNN training." };
    //
    const std::string m_PNN_ScoreLabel_SR1 = "PNN_Score";
    const std::string m_PNN_ScoreLabel_SR2 = "PNN_Score_SR2";
    const std::string m_DNN_ScoreLabel_SR1 = "DNN_Score_SR1";
    std::unique_ptr<lwt::LightweightGraph> m_model_PNN_setA_SR1_Run2;
    std::unique_ptr<lwt::LightweightGraph> m_model_PNN_setB_SR1_Run2;
    std::unique_ptr<lwt::LightweightGraph> m_model_PNN_setC_SR1_Run2;
    std::unique_ptr<lwt::LightweightGraph> m_model_PNN_setA_SR2_Run2;
    std::unique_ptr<lwt::LightweightGraph> m_model_PNN_setB_SR2_Run2;
    std::unique_ptr<lwt::LightweightGraph> m_model_PNN_setC_SR2_Run2;
    std::unique_ptr<lwt::LightweightGraph> m_model_DNN_setA_SR1_Run2;
    std::unique_ptr<lwt::LightweightGraph> m_model_DNN_setB_SR1_Run2;
    std::unique_ptr<lwt::LightweightGraph> m_model_DNN_setC_SR1_Run2;
    std::unique_ptr<lwt::LightweightGraph> m_model_PNN_setA_SR1_Run3;
    std::unique_ptr<lwt::LightweightGraph> m_model_PNN_setB_SR1_Run3;
    std::unique_ptr<lwt::LightweightGraph> m_model_PNN_setC_SR1_Run3;
    std::unique_ptr<lwt::LightweightGraph> m_model_PNN_setA_SR2_Run3;
    std::unique_ptr<lwt::LightweightGraph> m_model_PNN_setB_SR2_Run3;
    std::unique_ptr<lwt::LightweightGraph> m_model_PNN_setC_SR2_Run3;
    std::unique_ptr<lwt::LightweightGraph> m_model_DNN_setA_SR1_Run3;
    std::unique_ptr<lwt::LightweightGraph> m_model_DNN_setB_SR1_Run3;
    std::unique_ptr<lwt::LightweightGraph> m_model_DNN_setC_SR1_Run3;
    std::map<std::string, double> pnn_inputs_SR1;
    std::map<std::string, double> pnn_inputs_SR2;
    std::map<std::string, double> dnn_inputs_SR1;
  };
}
#endif
