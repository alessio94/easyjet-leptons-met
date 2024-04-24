/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TruthParticleInformationAlg.h
//
// This is an algorithm that will dump variables into a tree.
//
// Author: Victor Ruelas<victor.hugo.ruelas.rivera@cern.ch>
///////////////////////////////////////////////////////////////////

// Always protect against multiple includes!
#ifndef HHANALYSIS_TRUTHPARTICLEINFORMATIONALG
#define HHANALYSIS_TRUTHPARTICLEINFORMATIONALG

#include "AthContainers/ConstDataVector.h"
#include <AthContainers/AuxElement.h>
#include <AthenaBaseComps/AthAlgorithm.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODTruth/TruthParticleContainer.h>

namespace MC
{
  static const int SBOSONBSM = 35;
}

namespace Easyjet
{
  class TruthScalar
  {
private:
    using P4 = ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double>>;
    int m_pdgId;
    P4 m_p4;
    const xAOD::TruthParticle *m_source = nullptr;
    // final children
    std::vector<P4> m_children_p4;
    std::vector<P4> m_initial_children_p4;
    std::vector<const xAOD::TruthParticle *> m_children;
    std::vector<const xAOD::TruthParticle *> m_initial_children;

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

    void children(std::vector<const xAOD::TruthParticle *> children)
    {
      for (const xAOD::TruthParticle *child : children)
      {
        P4 child_p4{child->pt(), child->eta(), child->phi(), child->m()};
        m_children_p4.push_back(child_p4);
      }
      m_children = std::move(children);
    }

    void initial_children(std::vector<const xAOD::TruthParticle *> initial_children)
    {
      for (const xAOD::TruthParticle *initial_child : initial_children)
      {
        P4 initial_child_p4{initial_child->pt(), initial_child->eta(), initial_child->phi(), initial_child->m()};
        m_initial_children_p4.push_back(initial_child_p4);
      }
      m_initial_children = std::move(initial_children);
    }

    std::vector<int> children_pdgId(){
      std::vector<int> pdgId_pair;
      for (const xAOD::TruthParticle *child : m_children)
      {
	      pdgId_pair.push_back(child->pdgId());
      }
      return pdgId_pair;
    }

    std::vector<int> initial_children_pdgId(){
      std::vector<int> pdgId_pair;
      for (const xAOD::TruthParticle *initial_child : m_initial_children)
      {
	      pdgId_pair.push_back(initial_child->pdgId());
      }
      return pdgId_pair;
    }

    std::vector<float> children_p4(int coordIdx)
    {
      std::vector<float> coords_pair;
      for (P4 child_p4 : m_children_p4)
      {
        std::array<float, 4> coords;
        child_p4.GetCoordinates(coords.begin());
        coords_pair.push_back(coords[coordIdx]);
      }
      return coords_pair;
    }

    std::vector<float> initial_children_p4(int coordIdx)
    {
      std::vector<float> coords_pair;
      for (P4 initial_child_p4 : m_initial_children_p4)
      {
        std::array<float, 4> coords;
        initial_child_p4.GetCoordinates(coords.begin());
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
                           const std::unordered_set<int> pdgIds) const;

    std::vector<const xAOD::TruthParticle *>
    getFinalChildren(const xAOD::TruthParticle *p) const;

    std::vector<const xAOD::TruthParticle *>
    getInitialChildren(const xAOD::TruthParticle *p) const;

    std::vector<TruthScalar>
    getFinalHiggses(const xAOD::TruthParticleContainer &container) const;

    void verbosePrintParticleAndChildren(const xAOD::TruthParticle *p,
                                         int counter) const;

    void debugPrintParticleKinematics(const xAOD::TruthParticle *p) const;

    std::array<float, 4> calcHHKinematics(const xAOD::TruthParticle *p1, const xAOD::TruthParticle *p2) const;

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
    Gaudi::Property<std::vector<std::string>> m_decayModes  {this, "decayModes", {""}, "HH decay modes to consider"};

    std::vector<SG::AuxElement::Decorator<int>> m_truthHiggsesPdgIdDecorators;

    std::vector<std::vector<SG::AuxElement::Decorator<float>>>
        m_truthHiggsesKinDecorators;

    std::vector<SG::AuxElement::Decorator<std::vector<int>>>
        m_truthChildrenPdgIdFromHiggsesDecorators;

    std::vector<SG::AuxElement::Decorator<std::vector<int>>>
        m_truthInitialChildrenPdgIdFromHiggsesDecorators;

    std::vector<std::vector<SG::AuxElement::Decorator<std::vector<float>>>>
        m_truthChildrenKinFromHiggsesDecorators;

    std::vector<std::vector<SG::AuxElement::Decorator<std::vector<float>>>>
        m_truthInitialChildrenKinFromHiggsesDecorators;

    std::vector<SG::AuxElement::Decorator<float>> m_truthHHKinDecorators;

    std::array<std::string, 4> m_kinVars{"pt", "eta", "phi", "m"};
  };
}

#endif
