/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TruthParticleInformationAlg.h
//
// This is an algorithm that will dump variables into a tree.
//
// Author: Victor Ruelas<victor.hugo.ruelas.rivera@cern.ch>
///////////////////////////////////////////////////////////////////

// Always protect against multiple includes!
#ifndef HH4BANALYSIS_TRUTHPARTICLEINFORMATIONALG
#define HH4BANALYSIS_TRUTHPARTICLEINFORMATIONALG

#include "AthContainers/ConstDataVector.h"
#include <AthContainers/AuxElement.h>
#include <AthenaBaseComps/AthAlgorithm.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODTruth/TruthParticleContainer.h>

namespace Easyjet
{
  class TruthScalar
  {
private:
    using P4 = ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double>>;
    int m_pdgId;
    P4 m_p4;
    const xAOD::TruthParticle *m_source = nullptr;
    // final bb children
    std::vector<P4> m_bb_p4;
    std::vector<const xAOD::TruthParticle *> m_bb;

public:
    TruthScalar() { m_pdgId = 0; };
    TruthScalar(const xAOD::TruthParticle *h) : m_source(h)
    {
      m_pdgId = h->pdgId();
      m_p4.SetCoordinates(h->pt(), h->eta(), h->phi(), h->m());
    };

    operator const xAOD::TruthParticle *() { return m_source; };

    const xAOD::TruthParticle *source() { return m_source; };

    int pdgId() { return m_pdgId; };

    float p4(int coordIdx)
    {
      std::array<float, 4> coords;
      m_p4.GetCoordinates(coords.begin());
      return coords[coordIdx];
    }

    void bb(std::vector<const xAOD::TruthParticle *> bb)
    {
      for (const xAOD::TruthParticle *b : bb)
      {
        P4 b_p4{b->pt(), b->eta(), b->phi(), b->m()};
        m_bb_p4.push_back(b_p4);
      }
      m_bb = std::move(bb);
    }

    std::vector<float> bb_p4(int coordIdx)
    {
      std::vector<float> coords_pair;
      for (P4 b_p4 : m_bb_p4)
      {
        std::array<float, 4> coords;
        b_p4.GetCoordinates(coords.begin());
        coords_pair.push_back(coords[coordIdx]);
      }
      return coords_pair;
    }
  };

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

    const xAOD::TruthParticle *
    getFinalParticleOfType(const xAOD::TruthParticle *p,
                           const std::vector<int> pdgIds) const;

    std::vector<const xAOD::TruthParticle *>
    getFinalbb(const xAOD::TruthParticle *p) const;

    std::vector<TruthScalar>
    getFinalHiggses(const xAOD::TruthParticleContainer &container) const;

    void verbosePrintParticleAndChildren(const xAOD::TruthParticle *p,
                                         int counter) const;

    void debugPrintParticleKinematics(const xAOD::TruthParticle *p) const;

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

    unsigned int m_nHiggses;

    std::vector<SG::AuxElement::Decorator<int>> m_truthHiggsesPdgIdDecorators;

    std::vector<std::vector<SG::AuxElement::Decorator<float>>>
        m_truthHiggsesKinDecorators;

    std::vector<std::vector<SG::AuxElement::Decorator<std::vector<float>>>>
        m_truthbbKinFromHiggsesDecorators;

    std::array<std::string, 4> m_kinVars{"pt", "eta", "phi", "m"};
  };
}

#endif
