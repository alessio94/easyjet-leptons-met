/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Kehang Bai

#include "MuonTrackDecoratorAlg.h"
#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>
#include <AthenaKernel/Units.h>
#include <TLorentzVector.h>
#include <cmath>

namespace Easyjet {
MuonTrackDecoratorAlg ::MuonTrackDecoratorAlg(const std::string &name,
                                    ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode MuonTrackDecoratorAlg ::initialize() {
  // Initializa input containers
  ATH_CHECK(m_muonsInKey.initialize());

  // Initialize decorator keys

  // Muon ID track properties
  m_idtrackPtDecorKey = m_muonsInKey.key() + ".idtrack_pt";
  m_idtrackEtaDecorKey = m_muonsInKey.key() + ".idtrack_eta";
  m_idtrackPhiDecorKey = m_muonsInKey.key() + ".idtrack_phi";
  m_idtrackD0DecorKey = m_muonsInKey.key() + ".idtrack_d0";
  m_idtrackZ0DecorKey = m_muonsInKey.key() + ".idtrack_z0";
  m_idtrackChi2OverDoFDecorKey = m_muonsInKey.key() + ".idtrack_chi2OverDoF";
  ATH_CHECK(m_idtrackPtDecorKey.initialize(m_doRetrieveTracks));
  ATH_CHECK(m_idtrackEtaDecorKey.initialize(m_doRetrieveTracks));
  ATH_CHECK(m_idtrackPhiDecorKey.initialize(m_doRetrieveTracks));
  ATH_CHECK(m_idtrackD0DecorKey.initialize(m_doRetrieveTracks));
  ATH_CHECK(m_idtrackZ0DecorKey.initialize(m_doRetrieveTracks));
  ATH_CHECK(m_idtrackChi2OverDoFDecorKey.initialize(m_doRetrieveTracks));
  // Number of hits at each ID layer saved by default
  m_idtrackNumIBLHitsDecorKey = m_muonsInKey.key() + ".idtrack_nIBL";
  m_idtrackNumPixelHitsDecorKey = m_muonsInKey.key() + ".idtrack_nPIX";
  m_idtrackNumPixelSharedHitsDecorKey =
      m_muonsInKey.key() + ".idtrack_nPIX_shared";
  m_idtrackNumSCTHitsDecorKey = m_muonsInKey.key() + ".idtrack_nSCT";
  m_idtrackNumSCTSharedHitsDecorKey =
      m_muonsInKey.key() + ".idtrack_nSCT_shared";
  ATH_CHECK(m_idtrackNumIBLHitsDecorKey.initialize(m_doRetrieveTracks));
  ATH_CHECK(m_idtrackNumPixelHitsDecorKey.initialize(m_doRetrieveTracks));
  ATH_CHECK(m_idtrackNumPixelSharedHitsDecorKey.initialize(m_doRetrieveTracks));
  ATH_CHECK(m_idtrackNumSCTHitsDecorKey.initialize(m_doRetrieveTracks));
  ATH_CHECK(m_idtrackNumSCTSharedHitsDecorKey.initialize(m_doRetrieveTracks));
  // Numer of hits at each ID layer saved only in LLP1
  m_idtrackNumNextToIBLHitsDecorKey =
      m_muonsInKey.key() + ".idtrack_nNextToIBL";
  m_idtrackNumPixelSplitHitsDecorKey =
      m_muonsInKey.key() + ".idtrack_nPIX_split";
  m_idtrackNumTRTHitsDecorKey = m_muonsInKey.key() + ".idtrack_nTRT";
  ATH_CHECK(m_idtrackNumNextToIBLHitsDecorKey.initialize(m_doRetrieveTracks &&
                                                         m_doLLP1));
  ATH_CHECK(m_idtrackNumPixelSplitHitsDecorKey.initialize(m_doRetrieveTracks &&
                                                          m_doLLP1));
  ATH_CHECK(
      m_idtrackNumTRTHitsDecorKey.initialize(m_doRetrieveTracks && m_doLLP1));

  // Muon CB track properties
  m_cbtrackD0DecorKey = m_muonsInKey.key() + ".cbtrack_d0";
  m_cbtrackZ0DecorKey = m_muonsInKey.key() + ".cbtrack_z0";
  m_cbtrackChi2OverDoFDecorKey = m_muonsInKey.key() + ".cbtrack_chi2OverDoF";
  ATH_CHECK(m_cbtrackD0DecorKey.initialize(m_doRetrieveTracks));
  ATH_CHECK(m_cbtrackZ0DecorKey.initialize(m_doRetrieveTracks));
  ATH_CHECK(m_cbtrackChi2OverDoFDecorKey.initialize(m_doRetrieveTracks));

  return StatusCode::SUCCESS;
}

StatusCode MuonTrackDecoratorAlg ::execute(const EventContext &ctx) const {
  // Input handles
  // Muon container
  SG::ReadHandle<xAOD::MuonContainer> muonsIn(m_muonsInKey, ctx);
  ATH_CHECK(muonsIn.isValid());

  if (m_doRetrieveTracks) {

    // Initialize write handles
    SG::WriteDecorHandle<xAOD::MuonContainer, float> idtrackPtDecorHandle(
        m_idtrackPtDecorKey);
    SG::WriteDecorHandle<xAOD::MuonContainer, float> idtrackEtaDecorHandle(
        m_idtrackEtaDecorKey);
    SG::WriteDecorHandle<xAOD::MuonContainer, float> idtrackPhiDecorHandle(
        m_idtrackPhiDecorKey);
    SG::WriteDecorHandle<xAOD::MuonContainer, float> idtrackD0DecorHandle(
        m_idtrackD0DecorKey);
    SG::WriteDecorHandle<xAOD::MuonContainer, float> idtrackZ0DecorHandle(
        m_idtrackZ0DecorKey);
    SG::WriteDecorHandle<xAOD::MuonContainer, float>
        idtrackChi2OverDoFDecorHandle(m_idtrackChi2OverDoFDecorKey);
    SG::WriteDecorHandle<xAOD::MuonContainer, float> cbtrackD0DecorHandle(
        m_cbtrackD0DecorKey);
    SG::WriteDecorHandle<xAOD::MuonContainer, float> cbtrackZ0DecorHandle(
        m_cbtrackZ0DecorKey);
    SG::WriteDecorHandle<xAOD::MuonContainer, float>
        cbtrackChi2OverDoFDecorHandle(m_cbtrackChi2OverDoFDecorKey);

    // Number of hits at each ID layer saved by default
    SG::WriteDecorHandle<xAOD::MuonContainer, float>
        idtrackNumIBLHitsDecorHandle(m_idtrackNumIBLHitsDecorKey);
    SG::WriteDecorHandle<xAOD::MuonContainer, float>
        idtrackNumPixelHitsDecorHandle(m_idtrackNumPixelHitsDecorKey);
    SG::WriteDecorHandle<xAOD::MuonContainer, float>
        idtrackNumPixelSharedHitsDecorHandle(
            m_idtrackNumPixelSharedHitsDecorKey);
    SG::WriteDecorHandle<xAOD::MuonContainer, float>
        idtrackNumSCTHitsDecorHandle(m_idtrackNumSCTHitsDecorKey);
    SG::WriteDecorHandle<xAOD::MuonContainer, float>
        idtrackNumSCTSharedHitsDecorHandle(m_idtrackNumSCTSharedHitsDecorKey);

    // Muon track decorations
    for (const xAOD::Muon *muon : *muonsIn) {

      // ID track information associated to muon
      const xAOD::TrackParticle *muonIDTrackParticle = nullptr;

      // If muon links to a track particle
      if (muon->inDetTrackParticleLink().isValid()) {
        muonIDTrackParticle = *muon->inDetTrackParticleLink();
      }

      // If idtrack exists
      if (muonIDTrackParticle) {
        // ID track properties
        idtrackPtDecorHandle(*muon) = muonIDTrackParticle->pt();
        idtrackEtaDecorHandle(*muon) = muonIDTrackParticle->eta();
        idtrackPhiDecorHandle(*muon) = muonIDTrackParticle->phi();
        idtrackD0DecorHandle(*muon) = muonIDTrackParticle->d0();
        idtrackZ0DecorHandle(*muon) = muonIDTrackParticle->z0();
        idtrackChi2OverDoFDecorHandle(*muon) =
            muonIDTrackParticle->numberDoF() > 0
                ? muonIDTrackParticle->chiSquared() /
                      muonIDTrackParticle->numberDoF()
                : -999;

        // Number of hits at each ID layer
        uint8_t iSummaryValue(0); // Dummy counter to retrieve summary values
        idtrackNumIBLHitsDecorHandle(*muon) =
            muonIDTrackParticle->summaryValue(
                iSummaryValue, xAOD::numberOfInnermostPixelLayerHits)
                ? iSummaryValue
                : -999;
        idtrackNumPixelHitsDecorHandle(*muon) =
            muonIDTrackParticle->summaryValue(iSummaryValue,
                                              xAOD::numberOfPixelHits)
                ? iSummaryValue
                : -999;
        idtrackNumPixelSharedHitsDecorHandle(*muon) =
            muonIDTrackParticle->summaryValue(iSummaryValue,
                                              xAOD::numberOfPixelSharedHits)
                ? iSummaryValue
                : -999;
        idtrackNumSCTHitsDecorHandle(*muon) =
            muonIDTrackParticle->summaryValue(iSummaryValue,
                                              xAOD::numberOfSCTHits)
                ? iSummaryValue
                : -999;
        idtrackNumSCTSharedHitsDecorHandle(*muon) =
            muonIDTrackParticle->summaryValue(iSummaryValue,
                                              xAOD::numberOfSCTSharedHits)
                ? iSummaryValue
                : -999;
      } else {
        // Muon ID track particle does not exist, decorate with default values.
        idtrackNumIBLHitsDecorHandle(*muon) = -999;
        idtrackNumPixelHitsDecorHandle(*muon) = -999;
        idtrackNumPixelSharedHitsDecorHandle(*muon) = -999;
        idtrackNumSCTHitsDecorHandle(*muon) = -999;
        idtrackNumSCTSharedHitsDecorHandle(*muon) = -999;
      }

      // Combined track information associated to muon
      const xAOD::TrackParticle *muonCBTrackParticle = nullptr;

      // If muon links to a combined track particle
      if (muon->combinedTrackParticleLink().isValid()) {
        muonCBTrackParticle = *muon->combinedTrackParticleLink();
      }

      // If CB track exists
      if (muonCBTrackParticle) {
        cbtrackD0DecorHandle(*muon) = muonCBTrackParticle->d0();
        cbtrackZ0DecorHandle(*muon) = muonCBTrackParticle->z0();
        cbtrackChi2OverDoFDecorHandle(*muon) =
            muonCBTrackParticle->numberDoF() > 0
                ? muonCBTrackParticle->chiSquared() /
                      muonCBTrackParticle->numberDoF()
                : -999;
      } else {
        // Muon CB track particle does not exist, decorate with default values.
	cbtrackD0DecorHandle(*muon) = -999;
	cbtrackZ0DecorHandle(*muon) = -999;
	cbtrackChi2OverDoFDecorHandle(*muon) = -999;
      }
    } // end of loop over muons

    // LLP1 only branches
    if (m_doLLP1) {

    // Numer of hits at each ID layer saved only in LLP1
    SG::WriteDecorHandle<xAOD::MuonContainer, float>
        idtrackNumNextToIBLHitsDecorHandle(m_idtrackNumNextToIBLHitsDecorKey);
    SG::WriteDecorHandle<xAOD::MuonContainer, float>
        idtrackNumPixelSplitHitsDecorHandle(m_idtrackNumPixelSplitHitsDecorKey);
    SG::WriteDecorHandle<xAOD::MuonContainer, float>
        idtrackNumTRTHitsDecorHandle(m_idtrackNumTRTHitsDecorKey);

      // Muon track decorations
      for (const xAOD::Muon *muon : *muonsIn) {

        // ID track information associated to muon
        const xAOD::TrackParticle *muonIDTrackParticle = nullptr;

        // If muon links to a track particle
        if (muon->inDetTrackParticleLink().isValid()) {
          muonIDTrackParticle = *muon->inDetTrackParticleLink();
        }

        // If idtrack exists
        if (muonIDTrackParticle) {
          // Dummy counter to retrieve summary values
          uint8_t iSummaryValue(0);
          // Decorate number of hits
          idtrackNumNextToIBLHitsDecorHandle(*muon) =
              muonIDTrackParticle->summaryValue(
                  iSummaryValue, xAOD::numberOfNextToInnermostPixelLayerHits)
                  ? iSummaryValue
                  : -999;
          idtrackNumPixelSplitHitsDecorHandle(*muon) =
              muonIDTrackParticle->summaryValue(iSummaryValue,
                                                xAOD::numberOfPixelSplitHits)
                  ? iSummaryValue
                  : -999;
          idtrackNumTRTHitsDecorHandle(*muon) =
              muonIDTrackParticle->summaryValue(iSummaryValue,
                                                xAOD::numberOfTRTHits)
                  ? iSummaryValue
                  : -999;
        } else {
          // Muon ID track particle does not exist, decorate with default values.
          idtrackNumNextToIBLHitsDecorHandle(*muon) = -999;
	  idtrackNumPixelSplitHitsDecorHandle(*muon) = -999;
	  idtrackNumTRTHitsDecorHandle(*muon) = -999;
	}
      } // end of loop over muons
    } // end of if doLLP1
  } // end of if doRetriveTracks

  return StatusCode::SUCCESS;
}
} // namespace Easyjet
