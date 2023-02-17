/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Victor Ruelas

//
// includes
//
#include "TruthParticleInformationAlg.h"
#include <algorithm>

//
// method implementations
//
namespace HH4B
{
  static const int H_ID = 25;
  static const int S_ID = 35;

  TruthParticleInformationAlg ::TruthParticleInformationAlg(
      const std::string &name, ISvcLocator *pSvcLocator)
      : AthAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode TruthParticleInformationAlg ::initialize()
  {
    ATH_MSG_DEBUG("Initialising " << name());

    if (!m_truthParticleInfoInKey.empty())
      ATH_CHECK(m_truthParticleInfoInKey.initialize());

    if (!m_truthParticleInfoOutKey.empty())
      ATH_CHECK(m_truthParticleInfoOutKey.initialize());

    ATH_CHECK(m_EventInfoKey.initialize());

    for (auto const &truth_H1 : m_truthH1Vars)
    {
      m_selectionTruthH1Decorators.emplace_back(truth_H1);
    }

    for (auto const &truth_H2 : m_truthH2Vars)
    {
      m_selectionTruthH2Decorators.emplace_back(truth_H2);
    }

    for (auto const &truth_b_fromH1 : m_truthBFromH1Vars)
    {
      m_selectionTruthBFromH1Decorators.emplace_back(truth_b_fromH1);
    }

    for (auto const &truth_b_fromH2 : m_truthBFromH2Vars)
    {
      m_selectionTruthBFromH2Decorators.emplace_back(truth_b_fromH2);
    }

    return StatusCode::SUCCESS;
  }

  StatusCode TruthParticleInformationAlg ::execute()
  {
    ATH_MSG_DEBUG("Executing " << name());

    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    ATH_CHECK(eventInfo.isValid());
    SG::ReadHandle<xAOD::TruthParticleContainer> truthInformationParticles(
        m_truthParticleInfoInKey);
    ATH_CHECK(truthInformationParticles.isValid());

    if (eventInfo->eventType(xAOD::EventInfo::IS_SIMULATION))
    {
      ATH_CHECK(recordTruthParticleInformation(*truthInformationParticles,
                                               *eventInfo));
    }
    else
    {
      ATH_MSG_ERROR("Running on data, can't record truth information!");
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

  StatusCode TruthParticleInformationAlg ::recordTruthParticleInformation(
      const xAOD::TruthParticleContainer &truthInformationParticles,
      const xAOD::EventInfo &eventInfo) const
  {
    ATH_MSG_DEBUG("Saving truth particles as \""
                  << m_truthParticleInfoOutKey.key() << "\".");

    // Typedef for readability later
    // All xAOD::BlahContainers inherit from DataVector, which you
    // can think of as a vector of pointers that owns the pointers
    // (and will delete them when it is destructed).
    // DataVector only receives a non-const object, so we have a
    // different container type to which we pass const objects,
    // ConstDataVector (templated to the owning container type).
    typedef ConstDataVector<xAOD::TruthParticleContainer> CDV_TruthPart;

    // Now we can make another container to hold just the items we care about
    //
    // Here, we need to hand the memory
    // over to the store, so we need to create a new object on the heap.
    // Use std::unique_ptr to avoid memory leaks!
    //
    // Passing SG::VIEW_ELEMENTS to the constructor means that this
    // container will not own its contents (and therefore need to
    // manage the corresponding memory). The default is OWN_ELEMENTS.
    auto hhTruthParticles = std::make_unique<CDV_TruthPart>(SG::VIEW_ELEMENTS);

    for (const xAOD::TruthParticle *tp : truthInformationParticles)
    {
      if ((tp->pdgId() == H_ID || tp->pdgId() == S_ID) && tp->nChildren() == 2)
      {
        hhTruthParticles->push_back(tp);
      }
    }

    if (hhTruthParticles->size() < 2)
    {
      ATH_MSG_DEBUG(
          "Only 1 H truth particle in the event. Skipping the event "
          << eventInfo.eventNumber());
      if (m_filter) {
        setFilterPassed(false);
      }
      return StatusCode::SUCCESS;
    }

    if (hhTruthParticles->size() > 2)
    {
      ATH_MSG_DEBUG("More than 2 H truth particles in event "
                    << eventInfo.eventNumber());
    }

    std::map<std::string, std::vector<float>> truth_b_fromH1;
    std::map<std::string, std::vector<float>> truth_b_fromH2;
    std::map<std::string, float> truth_H1;
    std::map<std::string, float> truth_H2;

    auto h1 = (*hhTruthParticles.get())[0];
    truth_H1["truth_H1_pt"] = h1->pt();
    truth_H1["truth_H1_eta"] = h1->eta();
    truth_H1["truth_H1_phi"] = h1->phi();
    truth_H1["truth_H1_m"] = h1->m();
    for (size_t ic = 0; ic < h1->nChildren(); ++ic)
    {
      auto nchild = h1->child(ic);
      if (nchild == nullptr)
      {
        ATH_MSG_ERROR("children of H1 does not exist (nullptr).");
        continue;
      }
      if (msgLvl(MSG::VERBOSE))
      {
        ATH_MSG_VERBOSE("Information about b-quarks coming from H1. ID: "
                        << nchild->pdgId() << ", status: " << nchild->status()
                        << ", pt: " << nchild->pt() << ", eta: "
                        << nchild->eta() << ", phi: " << nchild->phi()
                        << ", m: " << nchild->m());
      }
      truth_b_fromH1["truth_b_fromH1_pt"].push_back(nchild->pt());
      truth_b_fromH1["truth_b_fromH1_eta"].push_back(nchild->eta());
      truth_b_fromH1["truth_b_fromH1_phi"].push_back(nchild->phi());
      truth_b_fromH1["truth_b_fromH1_m"].push_back(nchild->m());
    }

    auto h2 = (*hhTruthParticles.get())[1];
    truth_H2["truth_H2_pt"] = h2->pt();
    truth_H2["truth_H2_eta"] = h2->eta();
    truth_H2["truth_H2_phi"] = h2->phi();
    truth_H2["truth_H2_m"] = h2->m();
    for (size_t ic = 0; ic < h2->nChildren(); ++ic)
    {
      auto nchild = h2->child(ic);
      if (nchild == nullptr)
      {
        ATH_MSG_ERROR("children of H2 does not exist (nullptr).");
        continue;
      }
      if (msgLvl(MSG::VERBOSE))
      {
        ATH_MSG_VERBOSE("Information about b-quarks coming from H2. ID: "
                        << nchild->pdgId() << ", status: " << nchild->status()
                        << ", pt: " << nchild->pt() << ", eta: "
                        << nchild->eta() << ", phi: " << nchild->phi()
                        << ", m: " << nchild->m());
      }
      truth_b_fromH2["truth_b_fromH2_pt"].push_back(nchild->pt());
      truth_b_fromH2["truth_b_fromH2_eta"].push_back(nchild->eta());
      truth_b_fromH2["truth_b_fromH2_phi"].push_back(nchild->phi());
      truth_b_fromH2["truth_b_fromH2_m"].push_back(nchild->m());
    }

    // Add decorators
    m_truth_H1_pdgId(eventInfo) = h1->pdgId();
    m_truth_H2_pdgId(eventInfo) = h2->pdgId();

    for (size_t i = 0; i < m_truthH1Vars.size(); i++)
    {
      m_selectionTruthH1Decorators[i](eventInfo) = truth_H1[m_truthH1Vars[i]];
    }

    for (size_t i = 0; i < m_truthH2Vars.size(); i++)
    {
      m_selectionTruthH2Decorators[i](eventInfo) = truth_H2[m_truthH2Vars[i]];
    }

    for (size_t i = 0; i < m_truthBFromH1Vars.size(); i++)
    {
      m_selectionTruthBFromH2Decorators[i](eventInfo) =
          truth_b_fromH1[m_truthBFromH1Vars[i]];
    }

    for (size_t i = 0; i < m_truthBFromH2Vars.size(); i++)
    {
      m_selectionTruthBFromH1Decorators[i](eventInfo) =
          truth_b_fromH2[m_truthBFromH2Vars[i]];
    }

    SG::WriteHandle<ConstDataVector<xAOD::TruthParticleContainer>> writeHandle(
        m_truthParticleInfoOutKey);
    ATH_CHECK(writeHandle.record(std::move(hhTruthParticles)));

    return StatusCode::SUCCESS;
  }
}
