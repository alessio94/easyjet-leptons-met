/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

  MuonTrackDecoratorAlg:
  An alg that copies muon information to aux decorations so can be
  output branch.
*/

// Always protect against multiple includes!
#ifndef EASYJET_MUONTRACKDECORATORALG
#define EASYJET_MUONTRACKDECORATORALG

#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <xAODMuon/MuonContainer.h>
#include <xAODTracking/TrackParticleContainer.h>

namespace Easyjet {

/// \brief An algorithm for counting containers
class MuonTrackDecoratorAlg final : public AthReentrantAlgorithm {
  /// \brief The standard constructor
public:
  MuonTrackDecoratorAlg(const std::string &name, ISvcLocator *pSvcLocator);

  /// \brief Initialisation method, for setting up tools and other persistent
  /// configs
  StatusCode initialize() override;
  /// \brief Execute method, for actions to be taken in the event loop
  StatusCode execute(const EventContext &ctx) const override;
  /// We use default finalize() -- this is for cleanup, and we don't do any

private:
  // Members for configurable properties
  Gaudi::Property<bool> m_isMC{this, "isMC", false, "Is this simulation?"};

  Gaudi::Property<bool> m_doLLP1{
      this, "doLLP1", false,
      "retrive track information for muons that only exists in LLP1?"};

  // Muons
  SG::ReadHandleKey<xAOD::MuonContainer> m_muonsInKey{this, "muonsIn", "",
                                                      "reco muon container"};

  // Muon InDetTrackParticle properties
  SG::WriteDecorHandleKey<xAOD::MuonContainer> m_idtrackPtDecorKey;
  SG::WriteDecorHandleKey<xAOD::MuonContainer> m_idtrackEtaDecorKey;
  SG::WriteDecorHandleKey<xAOD::MuonContainer> m_idtrackPhiDecorKey;
  SG::WriteDecorHandleKey<xAOD::MuonContainer> m_idtrackD0DecorKey;
  SG::WriteDecorHandleKey<xAOD::MuonContainer> m_idtrackZ0DecorKey;
  SG::WriteDecorHandleKey<xAOD::MuonContainer> m_idtrackChi2OverDoFDecorKey;
  // Number of hits at each ID layer saved by default
  SG::WriteDecorHandleKey<xAOD::MuonContainer> m_idtrackNumIBLHitsDecorKey;
  SG::WriteDecorHandleKey<xAOD::MuonContainer> m_idtrackNumPixelHitsDecorKey;
  SG::WriteDecorHandleKey<xAOD::MuonContainer>
      m_idtrackNumPixelSharedHitsDecorKey;
  SG::WriteDecorHandleKey<xAOD::MuonContainer> m_idtrackNumSCTHitsDecorKey;
  SG::WriteDecorHandleKey<xAOD::MuonContainer>
      m_idtrackNumSCTSharedHitsDecorKey;
  // Numer of hits at each ID layer saved only in LLP1
  SG::WriteDecorHandleKey<xAOD::MuonContainer>
      m_idtrackNumNextToIBLHitsDecorKey;
  SG::WriteDecorHandleKey<xAOD::MuonContainer>
      m_idtrackNumPixelSplitHitsDecorKey;
  SG::WriteDecorHandleKey<xAOD::MuonContainer> m_idtrackNumTRTHitsDecorKey;

  // Muon CombinedTrackParticle properties
  SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cbtrackD0DecorKey;
  SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cbtrackZ0DecorKey;
  SG::WriteDecorHandleKey<xAOD::MuonContainer> m_cbtrackChi2OverDoFDecorKey;
};

} // namespace Easyjet

#endif
