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
  constexpr int H_ID = 25;
  constexpr int S_ID = 35;
  constexpr int b_ID = 5;
  constexpr int b_bar_ID = -5;

  TruthParticleInformationAlg ::TruthParticleInformationAlg(
      const std::string &name, ISvcLocator *pSvcLocator)
      : AthAlgorithm(name, pSvcLocator)
  {
    declareProperty("nHiggses", m_nHiggses = 2, "Number of Higgses to record");
  }

  StatusCode TruthParticleInformationAlg ::initialize()
  {
    ATH_MSG_DEBUG("Initialising " << name());

    if (!m_truthParticleBSMInKey.empty())
      ATH_CHECK(m_truthParticleBSMInKey.initialize());

    if (!m_truthParticleSMInKey.empty())
      ATH_CHECK(m_truthParticleSMInKey.initialize());

    if (!m_truthParticleInfoOutKey.empty())
      ATH_CHECK(m_truthParticleInfoOutKey.initialize());

    ATH_CHECK(m_EventInfoKey.initialize());

    for (int h = 0; h < m_nHiggses; h++)
    {
      // decorator will show up as "truth_Hx_pdgId", where x is the x higgs
      m_truthHiggsesPdgIdDecorators.emplace_back(
          "truth_H" + std::to_string(h + 1) + "_" + "pdgId");
      m_truthHiggsesKinDecorators.emplace_back(
          std::vector<SG::AuxElement::Decorator<float>>());
      m_truthbbKinFromHiggsesDecorators.emplace_back(
          std::vector<SG::AuxElement::Decorator<std::vector<float>>>());
      for (const std::string &var : m_kinVars)
      {
        m_truthHiggsesKinDecorators[h].emplace_back(
            "truth_H" + std::to_string(h + 1) + "_" + var);
        m_truthbbKinFromHiggsesDecorators[h].emplace_back(
            "truth_bb_fromH" + std::to_string(h + 1) + "_" + var);
      }
    }

    return StatusCode::SUCCESS;
  }

  StatusCode TruthParticleInformationAlg ::execute()
  {
    ATH_MSG_DEBUG("Executing " << name());

    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    ATH_CHECK(eventInfo.isValid());
    SG::ReadHandle<xAOD::TruthParticleContainer> truthSMParticles(
        m_truthParticleSMInKey);
    ATH_CHECK(truthSMParticles.isValid());
    SG::ReadHandle<xAOD::TruthParticleContainer> truthBSMParticles(
        m_truthParticleBSMInKey);
    ATH_CHECK(truthBSMParticles.isValid());

    bool isMC = eventInfo->eventType(xAOD::EventInfo::IS_SIMULATION);
    if (!isMC)
    {
      ATH_MSG_ERROR(
          "No truth particle information available in data, cannot build "
          << std::string(m_nHiggses, 'H') << " decay path!");
      return StatusCode::FAILURE;
    }
    ATH_CHECK(recordTruthParticleInformation(*truthBSMParticles,
                                             *truthSMParticles, *eventInfo));
    return StatusCode::SUCCESS;
  }

  StatusCode TruthParticleInformationAlg ::recordTruthParticleInformation(
      const xAOD::TruthParticleContainer &truthBSMParticles,
      const xAOD::TruthParticleContainer &truthSMParticles,
      const xAOD::EventInfo &eventInfo) const
  {
    ATH_MSG_DEBUG("Saving truth particles as \""
                  << m_truthParticleInfoOutKey.key() << "\".");

    // Check that BSM container is not empty, otherwise use SM container
    auto truthParticlesContaienr =
        !truthBSMParticles.empty() ? truthBSMParticles : truthSMParticles;

    /*
      Find and record truth particle information
    */
    std::vector<TruthScalar> higgses =
        getFinalHiggses(truthParticlesContaienr);
    /*
      Decorate truth particle information on EventInfo with defaults if higgses
      empty
    */
    decorateTruthParticleInformation(eventInfo, higgses);
    /*
      Write container with truth particles that will be available in other
      algorithms
    */
    auto higgsesTruthParticles =
        std::make_unique<ConstDataVector<xAOD::TruthParticleContainer>>(
            SG::VIEW_ELEMENTS);
    for (TruthScalar h : higgses)
    {
      debugPrintParticleKinematics(h);
      higgsesTruthParticles->push_back(h);
    }
    SG::WriteHandle<ConstDataVector<xAOD::TruthParticleContainer>> writeHandle(
        m_truthParticleInfoOutKey);
    ATH_CHECK(writeHandle.record(std::move(higgsesTruthParticles)));

    return StatusCode::SUCCESS;
  }

  void TruthParticleInformationAlg ::decorateTruthParticleInformation(
      const xAOD::EventInfo &eventInfo, std::vector<TruthScalar> higgses) const
  {
    if (higgses.size() == 0)
    {
      // Default values
      higgses = std::vector<TruthScalar>(m_nHiggses, TruthScalar());
    }
    for (int h = 0; h < m_nHiggses; h++)
    {
      m_truthHiggsesPdgIdDecorators[h](eventInfo) = higgses[h].pdgId();
      for (size_t i = 0; i < m_kinVars.size(); i++)
      {
        m_truthHiggsesKinDecorators[h][i](eventInfo) = higgses[h].p4(i);
        m_truthbbKinFromHiggsesDecorators[h][i](eventInfo) =
            higgses[h].bb_p4(i);
      }
    }
  }

  void TruthParticleInformationAlg::verbosePrintParticleAndChildren(
      const xAOD::TruthParticle *p, int counter = 1) const
  {
    // Failsafe to prevent infinite recursion if children go indefinitely
    if (counter == 100)
      return;
    ATH_MSG_VERBOSE("Particle " << p->index() << " pdgID " << p->pdgId()
                                << ", barcode " << p->barcode()
                                << ", children " << p->nChildren());
    for (size_t i = 0; i < p->nChildren(); i++)
    {
      verbosePrintParticleAndChildren(p->child(i), counter + 1);
    };
  }

  void TruthParticleInformationAlg::debugPrintParticleKinematics(
      const xAOD::TruthParticle *p) const
  {
    ATH_MSG_DEBUG("Particle " << p->pdgId() << ", pt " << p->pt() << ", phi "
                              << p->phi() << ", eta " << p->eta() << ", mass "
                              << p->m());
  }

  const xAOD::TruthParticle *
  TruthParticleInformationAlg ::getFinalParticleOfType(
      const xAOD::TruthParticle *p, const std::vector<int> ids) const
  {
    for (size_t i = 0; i < p->nChildren(); i++)
    {
      if (std::find(ids.begin(), ids.end(), p->child(i)->pdgId()) != ids.end())
      {
        return getFinalParticleOfType(p->child(i), ids);
      }
    }
    return p;
  }

  std::vector<const xAOD::TruthParticle *>
  TruthParticleInformationAlg ::getFinalbb(const xAOD::TruthParticle *h) const
  {
    std::vector<const xAOD::TruthParticle *> bb;
    const xAOD::TruthParticle *tmp(nullptr);
    for (size_t i = 0; i < h->nChildren(); i++)
    {
      if (msgLvl(MSG::VERBOSE))
      {
        verbosePrintParticleAndChildren(h->child(i));
      }
      const xAOD::TruthParticle *final_b =
          getFinalParticleOfType(h->child(i), {b_ID, b_bar_ID});
      if (!tmp || (final_b->barcode() != tmp->barcode()))
      {
        tmp = final_b;
        bb.push_back(final_b);
      }
    }
    return bb;
  }

  std::vector<TruthScalar> TruthParticleInformationAlg ::getFinalHiggses(
      const xAOD::TruthParticleContainer &truthParticlesContaienr) const
  {
    std::vector<TruthScalar> higgses;
    const xAOD::TruthParticle *tmp(nullptr);
    for (const xAOD::TruthParticle *tp : truthParticlesContaienr)
    {
      if (msgLvl(MSG::VERBOSE))
      {
        verbosePrintParticleAndChildren(tp);
      }
      if ((tp->pdgId() == H_ID || tp->pdgId() == S_ID))
      {
        const xAOD::TruthParticle *final_h =
            getFinalParticleOfType(tp, {H_ID, S_ID});
        if (!tmp || (final_h->barcode() != tmp->barcode()))
        {
          TruthScalar h = final_h;
          h.bb(getFinalbb(final_h));
          higgses.push_back(h);
          tmp = final_h;
        }
      }
    }
    return higgses;
  }
}
