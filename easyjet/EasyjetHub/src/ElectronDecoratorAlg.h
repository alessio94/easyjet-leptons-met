/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

  ElectronDecoratorAlg:
  An alg that copies electron information to aux decorations so can be
  output branch.
*/

// Always protect against multiple includes!
#ifndef EASYJET_ELECTRONDECORATORALG
#define EASYJET_ELECTRONDECORATORALG

#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <xAODEgamma/ElectronContainer.h>
#include <xAODTracking/VertexContainer.h>

namespace Easyjet
{

  /// \brief An algorithm for counting containers
  class ElectronDecoratorAlg final : public AthReentrantAlgorithm
  {
    /// \brief The standard constructor
  public:
    ElectronDecoratorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute(const EventContext& ctx) const override;
    /// We use default finalize() -- this is for cleanup, and we don't do any
    


  private:

    void assignHelixArray(const xAOD::TrackParticle* track, double helix[5], const xAOD::Vertex* pvtx) const;

    std::pair<float, float> position(const double* helix) const;

    // Members for configurable properties
    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    Gaudi::Property<bool> m_doRetrieveTracks
      {this, "doRetrieveTrack", false, "retrive track information for electrons?" };

    // Electrons
    SG::ReadHandleKey<xAOD::ElectronContainer> m_electronsInKey{
      this, "electronsIn", "", "reco electron container"
    };

    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackParticleKey
      {this, "tracksIn", "InDetTrackParticles", "input track particle container"};

    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_gsfTrackParticleKey
      {this, "gsfTracksIn", "GSFTrackParticles", "input track particle container"};

    SG::ReadHandleKey<xAOD::VertexContainer> m_VertexContainerKey
      {this, "vertexIn", "PrimaryVertices", "input primary vertex container"};

    SG::WriteDecorHandleKey<xAOD::ElectronContainer> m_closestSiTrackDecorKey;
    SG::WriteDecorHandleKey<xAOD::ElectronContainer> m_bestmatchedElTrackDecorKey;

    SG::WriteDecorHandleKey<xAOD::ElectronContainer> m_mllConvDecorKey;
    SG::WriteDecorHandleKey<xAOD::ElectronContainer> m_mllConvAtConvVDecorKey;
    SG::WriteDecorHandleKey<xAOD::ElectronContainer> m_radiusConvDecorKey;
    SG::WriteDecorHandleKey<xAOD::ElectronContainer> m_separationMinDCTDecorKey;

    const float PTTOCURVATURE = -0.301; // ATLAS B=2T in MeV/mm
    const float m_e = 0.511;

  };

}

#endif
