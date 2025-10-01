/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EASYJET_TRUTHLEPTONORDERINGALG
#define EASYJET_TRUTHLEPTONORDERINGALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteHandleKey.h>
#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>

#include <AthContainers/ConstDataVector.h>
#include <xAODTruth/TruthEvent.h>
#include <xAODTruth/TruthEventContainer.h>
#include <xAODTruth/TruthParticle.h>
#include <xAODTruth/TruthParticleContainer.h> 

namespace Easyjet
{

  /// \brief An algorithm for counting containers
  class TruthLeptonOrderingAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    TruthLeptonOrderingAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};
    Gaudi::Property<int>   m_leptonAmount       {this, "leptonAmount", -1, "Number of truth leptons to consider for isTruthLeptonXX decoration"};

    SG::ReadHandleKey<xAOD::TruthParticleContainer>
    m_inTruthElectronHandleKey{ this, "containerInTruthElectronKey", "",   "Truth electron container to read" };

    SG::ReadHandleKey<xAOD::TruthParticleContainer>
    m_inTruthMuonHandleKey{ this, "containerInTruthMuonKey", "",   "Truth muon container to read" };


    SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_isSelectedElectronKey{this, "isTruthElectronDecoration", "isTruthElectron", "Decoration for selected truth electrons"};
    SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_isSelectedMuonKey{this, "isTruthMuonDecoration", "isTruthMuon", "Decoration for selected truth muons"};

    /// \brief Setup  output decorations
    std::unordered_map<std::string, SG::WriteDecorHandleKey<xAOD::TruthParticleContainer>> m_leadBranchesEleKey;
    std::unordered_map<std::string, SG::WriteDecorHandleKey<xAOD::TruthParticleContainer>> m_leadBranchesMuKey;
  };
}

#endif