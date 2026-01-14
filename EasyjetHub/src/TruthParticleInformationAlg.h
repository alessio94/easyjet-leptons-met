/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TruthParticleInformationAlg.h
//
// This is an algorithm that will dump variables into a tree.
//
// Author: Victor Ruelas<victor.hugo.ruelas.rivera@cern.ch>
//         Shudong Wang<shudong.wang@cern.ch>
///////////////////////////////////////////////////////////////////

// Always protect against multiple includes!
#ifndef HHANALYSIS_TRUTHPARTICLEINFORMATIONALG
#define HHANALYSIS_TRUTHPARTICLEINFORMATIONALG

#include "AthContainers/ConstDataVector.h"
#include <AthContainers/AuxElement.h>
#include <AthenaBaseComps/AthAlgorithm.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODTruth/TruthParticleContainer.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKeyArray.h>


namespace MC
{
  static const int SBOSONBSM = 35;
  static const int ABOSONBSM = 36;
}

namespace Easyjet
{

  class TruthScalar;
  namespace DecayMode {
    enum DecayMode :int8_t;
  }

  /// \brief An algorithm for dumping variables
  class TruthParticleInformationAlg final : public AthAlgorithm
  {
    /// \brief The standard constructor
public:
    TruthParticleInformationAlg(const std::string &name,
                                ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any


private:
    using P4 = ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double>>;

    StatusCode
    recordTruthParticleInformation(const xAOD::TruthParticleContainer &,
                                   const xAOD::TruthParticleContainer &,
                                   const xAOD::EventInfo &) const;

    void
    decorateTruthParticleInformation(const xAOD::EventInfo &eventInfo,
                                     std::vector<TruthScalar>& higgses) const;


    std::vector<const xAOD::TruthParticle *>
    getFinalChildren(const xAOD::TruthParticle *p) const;

    std::vector<const xAOD::TruthParticle *>
    getInitialChildren(const xAOD::TruthParticle *p) const;

    std::vector<std::vector<const xAOD::TruthParticle *>>
    getFinalGrandchildren(const std::vector<const xAOD::TruthParticle *>& children) const;

    std::vector<std::vector<const xAOD::TruthParticle *>>
    getInitialGrandchildren(const std::vector<const xAOD::TruthParticle *>& children) const;

    std::vector<TruthScalar>
    getFinalHiggses(const xAOD::TruthParticleContainer &container) const;

    void verbosePrintParticleAndChildren(const xAOD::TruthParticle *p,
                                         int counter) const;

    void debugPrintParticleKinematics(const xAOD::TruthParticle *p) const;

    std::array<float, 4> calcHHKinematics(const xAOD::TruthParticle *p1, const xAOD::TruthParticle *p2) const;
    std::array<float, 2> calcHHAverageKinematics(const xAOD::TruthParticle *p1, const xAOD::TruthParticle *p2) const;
    float calcHHCosThetaStar(const xAOD::TruthParticle *h1_in, const xAOD::TruthParticle *h2_in) const;

    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{
      this, "EventInfoKey", "EventInfo", "EventInfo container to dump"};

    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleSMInKey{
      this, "TruthParticleSMInKey", "",
      "the truth Standard Model particles container to run on"};

    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleBSMInKey{
      this, "TruthParticleBSMInKey", "",
      "the truth Beyond Standard Model particles container to run on"};

    SG::WriteHandleKey<ConstDataVector<xAOD::TruthParticleContainer>>
      m_truthParticleInfoOutKey{
        this, "TruthParticleInformationOutKey", "",
        "Truth particle information container to write"};

    Gaudi::Property<unsigned int> m_nHiggses                {this, "nHiggses", 2, "Number of Higgses to record"};
    Gaudi::Property<bool> m_recordGrandchildren{
      this, "recordGrandchildren", false, "Record HH grandchildren or not"};

    Gaudi::Property<std::vector<std::string>> m_decayModes  {this, "decayModes", {""}, "HH decay modes to consider"};
    std::set<int> m_targetPdgIDs;

    SG::WriteDecorHandleKeyArray<xAOD::EventInfo, int>
      m_truthHiggsesPdgIdDecorKeys;

    std::vector<SG::WriteDecorHandleKeyArray<xAOD::EventInfo, float>>
      m_truthHiggsesKinDecorKeys;

    SG::WriteDecorHandleKeyArray<xAOD::EventInfo, int>
      m_truthChildrenPdgIdFromHiggsesDecorKeys;

    SG::WriteDecorHandleKeyArray<xAOD::EventInfo, int>
      m_truthInitialChildrenPdgIdFromHiggsesDecorKeys;

    std::vector<SG::WriteDecorHandleKeyArray<xAOD::EventInfo, float>>
      m_truthChildrenKinFromHiggsesDecorKeys;

    std::vector<SG::WriteDecorHandleKeyArray<xAOD::EventInfo, float>>
      m_truthInitialChildrenKinFromHiggsesDecorKeys;

    SG::WriteDecorHandleKeyArray<xAOD::EventInfo, int>
      m_truthGrandchildrenPdgIdFromHiggsesDecorKeys;

    SG::WriteDecorHandleKeyArray<xAOD::EventInfo, int>
      m_truthInitialGrandchildrenPdgIdFromHiggsesDecorKeys;

    std::vector<SG::WriteDecorHandleKeyArray<xAOD::EventInfo, float>>
      m_truthGrandchildrenKinFromHiggsesDecorKeys;

    std::vector<SG::WriteDecorHandleKeyArray<xAOD::EventInfo, float>>
      m_truthInitialGrandchildrenKinFromHiggsesDecorKeys;

    SG::WriteDecorHandleKeyArray<xAOD::EventInfo, float> m_truthHHKinDecorKeys;
    SG::WriteDecorHandleKeyArray<xAOD::EventInfo, float> m_truthHHAverageKinDecorKeys;
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_absCosThetaStarDecorKey;

    std::array<std::string, 4> m_kinVars{"pt", "eta", "phi", "m"};
    std::array<std::string, 2> m_kinAverageVars{"average_pt", "average_eta"};
    std::string m_absCosThetaStar = "abs_cos_theta_star";
  };
}

#endif
