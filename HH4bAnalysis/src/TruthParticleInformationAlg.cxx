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

    for (auto const &truth_H : m_truthHVars)
    {
      m_selectionTruthHDecorators.emplace_back(truth_H);
    }

    for (auto const &truth_S : m_truthSVars)
    {
      m_selectionTruthSDecorators.emplace_back(truth_S);
    }

    for (auto const &truth_b_fromH : m_truthBFromHVars)
    {
      m_selectionTruthBFromHDecorators.emplace_back(truth_b_fromH);
    }

    for (auto const &truth_b_fromS : m_truthBFromSVars)
    {
      m_selectionTruthBFromSDecorators.emplace_back(truth_b_fromS);
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
      ATH_CHECK(recordTruthParticleInformation(*truthInformationParticles,
                                               *eventInfo));
    }
    else
    {
      ATH_MSG_WARNING("Running on data, not recording truth information!");
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
    auto p_truthparticles = std::make_unique<CDV_TruthPart>(SG::VIEW_ELEMENTS);

    std::map<std::string, std::vector<float>> truth_b_fromH;
    std::map<std::string, std::vector<float>> truth_b_fromS;
    std::map<std::string, float> truth_H;
    std::map<std::string, float> truth_S;

    for (auto *ptcl : truthInformationParticles)
    {
      if (msgLvl(MSG::VERBOSE))
      {
        ATH_MSG_VERBOSE("Information about truth particle ID: "
                        << ptcl->pdgId() << ", status: " << ptcl->status()
                        << ", pt: " << ptcl->pt() << ", eta: " << ptcl->eta()
                        << ", phi: " << ptcl->phi() << ", m: " << ptcl->m());
      }

      // Keep only the particles involved in the decay
      if (ptcl->pdgId() != X_ID && ptcl->pdgId() != S_ID &&
          ptcl->pdgId() != H_ID)
        continue;

      // Require that they have exactly 2 children and exactly one parent
      if (ptcl->nParents() != 1 && ptcl->nChildren() != 2)
        continue;

      p_truthparticles->push_back(ptcl);

      // Truth information needed only for X, S or H
      if (ptcl->pdgId() == S_ID || ptcl->pdgId() == H_ID)
      {
        auto parent = ptcl->parent(0);
        if (parent == nullptr)
        {
          ATH_MSG_ERROR("S or H parent does not exist (nullptr).");
          return StatusCode::FAILURE;
        }
        if (parent->pdgId() == X_ID)
        {
          if (msgLvl(MSG::VERBOSE))
          {
            ATH_MSG_VERBOSE(
                "Information about truth X ID: "
                << parent->pdgId() << ", status: " << parent->status()
                << ", pt: " << parent->pt() << ", eta: " << parent->eta()
                << ", phi: " << parent->phi() << ", m: " << parent->m());
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
            truth_S["truth_S_pt"] = ptcl->pt();
            truth_S["truth_S_eta"] = ptcl->eta();
            truth_S["truth_S_phi"] = ptcl->phi();
            truth_S["truth_S_m"] = ptcl->m();
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
            truth_H["truth_H_pt"] = ptcl->pt();
            truth_H["truth_H_eta"] = ptcl->eta();
            truth_H["truth_H_phi"] = ptcl->phi();
            truth_H["truth_H_m"] = ptcl->m();
          }

          // Truth information on the b-quarks from S and H for each X
          int nChildren = ptcl->nChildren();
          for (int ic = 0; ic < nChildren; ++ic)
          {
            auto nchild = ptcl->child(ic);
            if (ptcl->pdgId() == H_ID)
            {
              if (nchild == nullptr)
              {
                ATH_MSG_ERROR("children of H does not exist (nullptr).");
                return StatusCode::FAILURE;
              }
              if (msgLvl(MSG::VERBOSE))
              {
                ATH_MSG_VERBOSE(
                    "Information about b-quarks coming from H,  ID: "
                    << nchild->pdgId() << ", status: " << nchild->status()
                    << ", pt: " << nchild->pt() << ", eta: " << nchild->eta()
                    << ", phi: " << nchild->phi() << ", m: " << nchild->m());
              }
              truth_b_fromH["truth_b_fromH_pt"].push_back(nchild->pt());
              truth_b_fromH["truth_b_fromH_eta"].push_back(nchild->eta());
              truth_b_fromH["truth_b_fromH_phi"].push_back(nchild->phi());
              truth_b_fromH["truth_b_fromH_m"].push_back(nchild->m());
            }
            if (ptcl->pdgId() == S_ID)
            {
              if (nchild == nullptr)
              {
                ATH_MSG_ERROR("children of S does not exist (nullptr).");
                return StatusCode::FAILURE;
              }
              if (msgLvl(MSG::VERBOSE))
              {
                ATH_MSG_VERBOSE(
                    "Information about b-quarks coming from S, ID: "
                    << nchild->pdgId() << ", status: " << nchild->status()
                    << ", pt: " << nchild->pt() << ", eta: " << nchild->eta()
                    << ", phi: " << nchild->phi() << ", m: " << nchild->m());
              }
              truth_b_fromS["truth_b_fromS_pt"].push_back(nchild->pt());
              truth_b_fromS["truth_b_fromS_eta"].push_back(nchild->eta());
              truth_b_fromS["truth_b_fromS_phi"].push_back(nchild->phi());
              truth_b_fromS["truth_b_fromS_m"].push_back(nchild->m());
            }
          }
        }
      }
    }

    for (size_t i = 0; i < m_truthHVars.size(); i++)
    {
      m_selectionTruthHDecorators[i](eventInfo) = truth_H[m_truthHVars[i]];
    }

    for (size_t i = 0; i < m_truthSVars.size(); i++)
    {
      m_selectionTruthSDecorators[i](eventInfo) = truth_S[m_truthSVars[i]];
    }

    for (size_t i = 0; i < m_truthBFromHVars.size(); i++)
    {
      m_selectionTruthBFromSDecorators[i](eventInfo) =
          truth_b_fromH[m_truthBFromHVars[i]];
    }

    for (size_t i = 0; i < m_truthBFromSVars.size(); i++)
    {
      m_selectionTruthBFromHDecorators[i](eventInfo) =
          truth_b_fromS[m_truthBFromSVars[i]];
    }

    SG::WriteHandle<ConstDataVector<xAOD::TruthParticleContainer>>
        xshParticles(m_truthParticleInfoOutKey);
    ATH_CHECK(xshParticles.record(std::move(p_truthparticles)));

    return StatusCode::SUCCESS;
  }
}
