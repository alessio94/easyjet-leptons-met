/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HH4BANALYSIS_FINALVARSBOOSTEDALG
#define HH4BANALYSIS_FINALVARSBOOSTEDALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>

namespace HH4B
{

  /// \brief An algorithm for counting containers
  class BaselineVarsBoostedAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    BaselineVarsBoostedAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::JetContainer>
      m_LargeRJetHandle{ this, "jets", "boostedAnalysisLargeRJets_%SYS%", "Large-R jets container to read" };
    CP::SysReadHandle<xAOD::JetContainer>
      m_RNNjetBoostedHandle{ this, "RNNJets_boosted", "RNNJets_boosted_%SYS%", "input RNN jets container for boosted 20 GeV" };
      
    CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    CP::SysReadDecorHandle<int> m_R10TruthLabel{"R10TruthLabel_R22v1", this};
    CP::SysReadDecorHandle<float> m_GN2Xv01_phbb = {"GN2Xv01_phbb", this};
    CP::SysReadDecorHandle<float> m_GN2Xv01_phcc = {"GN2Xv01_phcc", this};
    CP::SysReadDecorHandle<float> m_GN2Xv01_pqcd = {"GN2Xv01_pqcd", this};
    CP::SysReadDecorHandle<float> m_GN2Xv01_ptop = {"GN2Xv01_ptop", this};
    CP::SysReadDecorHandle<float> m_Tau2_wta = {"Tau2_wta", this};
    CP::SysReadDecorHandle<float> m_Tau3_wta = {"Tau3_wta", this};

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    Gaudi::Property<std::vector<std::string>> m_Fvars
      {this, "floatVariableList", {}, "Name list of floating variables"};

    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_Fdecos;

    float calculateGN2Xv01_disc(float phbb_score, float pqcd_score, float phcc_score, float ptop_score, float f_Hcc=0.02, float f_top=0.25) const {
      return std::log(phbb_score / (f_Hcc * phcc_score + f_top * ptop_score + (1.0f - f_top - f_Hcc) * pqcd_score));
    }

    Gaudi::Property<bool> m_UseVBFRNN { this, "UseVBFRNN", false, "Add VBF-RNN jets in ntuples or not" };
  };
}

#endif
