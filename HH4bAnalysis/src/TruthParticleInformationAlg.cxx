/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Victor Ruelas

//
// includes
//
#include "TruthParticleInformationAlg.h"

//
// method implementations
//
namespace HH4B
{
  static const int H_ID = 25;
  static const int S_ID = 35;
  static const int X_ID = 36;

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

    for (auto const &truth_SH : truth_SH_map)
    {
      m_selectionTruthSHAccessors.emplace_back(truth_SH.first);
    }

    for (auto const &truth_b : truth_b_map)
    {
      m_selectionTruthBAccessors.emplace_back(truth_b.first);
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

    m_isMC = eventInfo->eventType(xAOD::EventInfo::IS_SIMULATION);
    if (m_isMC)
    {
      ATH_CHECK(recordTruthParticleInformation(*truthInformationParticles));
      ATH_CHECK(decorateEventInfoWithTruthParticleInfo(*eventInfo));
    }
    else
    {
      ATH_MSG_WARNING("Running on data, not recording truth information!");
    }

    return StatusCode::SUCCESS;
  }

  StatusCode TruthParticleInformationAlg ::recordTruthParticleInformation(
      const xAOD::TruthParticleContainer &truthInformationParticles)
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
    auto p_truthparticles = std::make_unique<CDV_TruthPart>(SG::VIEW_ELEMENTS);

    for (auto *ptcl : truthInformationParticles)
    {
      // Keep only the particles involved in the decay
      if (fabs(ptcl->pdgId() != X_ID) && fabs(ptcl->pdgId() != S_ID) &&
          fabs(ptcl->pdgId() != H_ID))
        continue;

      // Require that they have exactly 2 children and exactly one parent
      if (ptcl->nParents() != 1 && ptcl->nChildren() != 2)
        continue;

      p_truthparticles->push_back(ptcl);

      // Truth information needed only for X, S or H
      if (fabs(ptcl->pdgId() == S_ID) || fabs(ptcl->pdgId() == H_ID))
      {
        if (ptcl->parent(0)->pdgId() == X_ID)
        {
          if (msgLvl(MSG::VERBOSE))
          {
            ATH_MSG_VERBOSE("Information about truth X ID: "
                            << ptcl->parent(0)->pdgId()
                            << ", status: " << ptcl->parent(0)->status()
                            << ", pt: " << ptcl->parent(0)->pt()
                            << ", eta: " << ptcl->parent(0)->eta()
                            << ", phi: " << ptcl->parent(0)->phi()
                            << ", m: " << ptcl->parent(0)->m());
          }

          // Save kinematics of S and H
          if (ptcl->pdgId() == S_ID)
          {
            if (msgLvl(MSG::VERBOSE))
            {
              ATH_MSG_VERBOSE(
                  "Truth particle about S ID: "
                  << ptcl->pdgId() << ", status: " << ptcl->status()
                  << ", pt: " << ptcl->pt() << ", eta: " << ptcl->eta()
                  << ", phi: " << ptcl->phi() << ", mass: " << ptcl->m());
            }
            truth_SH_map["truth_S_pt"] = ptcl->pt();
            truth_SH_map["truth_S_eta"] = ptcl->eta();
            truth_SH_map["truth_S_phi"] = ptcl->phi();
            truth_SH_map["truth_S_m"] = ptcl->m();
          }
          if (ptcl->pdgId() == H_ID)
          {
            if (msgLvl(MSG::VERBOSE))
            {
              ATH_MSG_VERBOSE(
                  "Truth particle about H ID: "
                  << ptcl->pdgId() << ", status: " << ptcl->status()
                  << ", pt: " << ptcl->pt() << ", eta: " << ptcl->eta()
                  << ", phi: " << ptcl->phi() << ", mass: " << ptcl->m());
            }
            truth_SH_map["truth_H_pt"] = ptcl->pt();
            truth_SH_map["truth_H_eta"] = ptcl->eta();
            truth_SH_map["truth_H_phi"] = ptcl->phi();
            truth_SH_map["truth_H_m"] = ptcl->m();
          }

          // Truth information on the b-quarks from S and H for each X
          int nchild = ptcl->nChildren();
          for (int ic = 0; ic < nchild; ++ic)
          {
            if (msgLvl(MSG::VERBOSE))
            {
              ATH_MSG_VERBOSE("Information about b-quarks ID: "
                              << ptcl->child(ic)->pdgId()
                              << ", status: " << ptcl->child(ic)->status()
                              << ", pt: " << ptcl->child(ic)->pt()
                              << ", eta: " << ptcl->child(ic)->eta()
                              << ", phi: " << ptcl->child(ic)->phi()
                              << ", m: " << ptcl->child(ic)->m());
            }
            if (ptcl->pdgId() == H_ID)
            {

              truth_b_map["truth_b_fromH_pt"].push_back(ptcl->child(ic)->pt());
              truth_b_map["truth_b_fromH_eta"].push_back(
                  ptcl->child(ic)->eta());
              truth_b_map["truth_b_fromH_phi"].push_back(
                  ptcl->child(ic)->phi());
              truth_b_map["truth_b_fromH_m"].push_back(ptcl->child(ic)->m());
            }
            if (ptcl->pdgId() == S_ID)
            {
              truth_b_map["truth_b_fromS_pt"].push_back(ptcl->child(ic)->pt());
              truth_b_map["truth_b_fromS_eta"].push_back(
                  ptcl->child(ic)->eta());
              truth_b_map["truth_b_fromS_phi"].push_back(
                  ptcl->child(ic)->phi());
              truth_b_map["truth_b_fromS_m"].push_back(ptcl->child(ic)->m());
            }
          }
        }
      }
    }

    SG::WriteHandle<ConstDataVector<xAOD::TruthParticleContainer>>
        xshParticles(m_truthParticleInfoOutKey);
    ATH_CHECK(xshParticles.record(std::move(p_truthparticles)));

    return StatusCode::SUCCESS;
  }

  StatusCode
  TruthParticleInformationAlg ::decorateEventInfoWithTruthParticleInfo(
      const xAOD::EventInfo &eventInfo)
  {
    int i = 0;
    for (auto const &truth_SH : truth_SH_map)
    {
      m_selectionTruthSHAccessors[i](eventInfo) = truth_SH.second;
      ++i;
    }

    int j = 0;
    for (auto const &truth_b : truth_b_map)
    {
      m_selectionTruthBAccessors[j](eventInfo) = truth_b.second;
      ++j;
    }

    return StatusCode::SUCCESS;
  }

}
