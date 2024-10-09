/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef TTHHANALYSIS_JETPAIRINGALG
#define TTHHANALYSIS_JETPAIRINGRALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysListHandle.h>

#include "xAODJet/JetContainer.h"

namespace ttHH
{

  enum PairingStrategy {
      ChiSquare,
      MinDeltaR,
  };

  /// \brief An algorithm for counting containers
  class JetPairingAlgttHH final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    JetPairingAlgttHH(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::JetContainer>
    m_bjetHandle{ this, "bjets", "ttHHAnalysisJets_BTag_%SYS%", "Jet container to read" };

    CP::SysReadHandle<xAOD::JetContainer>
    m_jetHandle{ this, "jets", "ttHHAnalysisJets_%SYS%", "Jet container to read" };

    CP::SysReadDecorHandle<char> 
    m_isBtag {this, "bTagWPDecorName", "", "Name of input decorator for b-tagging"};

    /// \brief Setup syst-aware output container handles
    CP::SysWriteHandle<ConstDataVector<xAOD::JetContainer>>
    m_outHandle{ this, "pairedOutBJets", "pairedttHHAnalysisJets_%SYS%", "Jet container to write" };

    std::vector<const xAOD::Jet*> bJetChiSquarePairing(const ConstDataVector<xAOD::JetContainer>& Jets, float target_mass_1, float target_mass_2);

    std::tuple<const xAOD::Jet*, const xAOD::Jet*, const xAOD::Jet*, const xAOD::Jet*, float> minChiSquared(const ConstDataVector<xAOD::JetContainer>& Jets, const std::vector<size_t>& indexes, float target_mass_1, float target_mass_2);

    std::string m_pairingStrategyName;

    std::optional<PairingStrategy> m_pairingStrategy;

    Gaudi::Property<float> m_targetMass1 {this, "targetMass1", 125e3, "First target mass of boson to be used in chi square pairing"};
    Gaudi::Property<float> m_targetMass2 {this, "targetMass2", 125e3, "Second target mass of boson to be used in chi square pairing"};
  };
}

#endif
