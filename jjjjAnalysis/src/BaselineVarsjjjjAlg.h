/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef JJJJANALYSIS_FINALVARSJJJJALG
#define JJJJANALYSIS_FINALVARSJJJJALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODTruth/TruthParticleContainer.h>
#include "AthContainers/ConstDataVector.h"
//#include <TMatrixD.h>
//#include <TMatrixDEigen.h>
#include <Eigen/Dense>
//#include <TMatrixT.h>
//#include <TMatrixTEigen.h>
//#include <TDecompEigen.h>


typedef TLorentzVector TLV;
typedef std::vector<TLorentzVector> TLVs;

namespace jjjj
{

  /// \brief An algorithm for counting containers
  class BaselineVarsjjjjAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    BaselineVarsjjjjAlg(const std::string &name, ISvcLocator *pSvcLocator);

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

    Eigen::Matrix3d get3DMomentumTensor (TLVs const &jetP4s);
    Eigen::Matrix2d get2DMomentumTensor (TLVs const &jetP4s);
    CP::SysReadHandle<xAOD::JetContainer> m_SmallRJetsHandle{ this, "SmallRJets", "", "Small R Jet container to read"};
    
    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    Gaudi::Property<bool> m_isMC { this, "isMC", false, "Is this simulation?" };
    
    CP::SysReadDecorHandle<int> m_PTLID = {this, "PartonTruthLabelID", "PartonTruthLabelID", "PartonTruthLabelID"};

    Gaudi::Property<std::vector<std::string>> m_floatVariables {this, "floatVariableList", {}, "Name list of floating variables"};

    Gaudi::Property<std::vector<std::string>> m_intVariables {this, "intVariableList", {}, "Name list of integer variables"};

    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_Fbranches;

    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;

  };
}

#endif
