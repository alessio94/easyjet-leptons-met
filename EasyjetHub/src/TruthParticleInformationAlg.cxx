/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Victor Ruelas

//
// includes
//
#include "TruthParticleInformationAlg.h"
#include <algorithm>

#include "TruthUtils/HepMCHelpers.h"

//
// method implementations
//
namespace Easyjet
{
  const std::unordered_map<std::string, std::vector<int>> decayProducts_IDs{
    {"bbbb", {MC::BQUARK, -MC::BQUARK}},
    {"bbtt", {MC::BQUARK, -MC::BQUARK, MC::TAU, -MC::TAU}},
    {"bbyy", {MC::BQUARK, -MC::BQUARK, MC::PHOTON}},
    {"bbWW", {MC::BQUARK, -MC::BQUARK, MC::WPLUSBOSON, -MC::WPLUSBOSON}},
    {"bbZZ", {MC::BQUARK, -MC::BQUARK, MC::Z0BOSON}},
    {"mmtt", {MC::MUON, -MC::MUON, MC::TAU, -MC::TAU}},
    {"eett", {MC::ELECTRON, -MC::ELECTRON, MC::TAU, -MC::TAU}},
    {"tttt", {MC::TAU, -MC::TAU}},

  };

  TruthParticleInformationAlg ::TruthParticleInformationAlg(
      const std::string &name, ISvcLocator *pSvcLocator)
      : AthAlgorithm(name, pSvcLocator) { }

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

    for (unsigned int h = 0; h < m_nHiggses; h++)
    {
      // decorator will show up as "truth_Hx_pdgId", where x is the x higgs
      m_truthHiggsesPdgIdDecorators.emplace_back(
          "truth_H" + std::to_string(h + 1) + "_" + "pdgId");
      m_truthChildrenPdgIdFromHiggsesDecorators.emplace_back(
          "truth_children_fromH" + std::to_string(h + 1) + "_" + "pdgId");
      m_truthInitialChildrenPdgIdFromHiggsesDecorators.emplace_back(
          "truth_initial_children_fromH" + std::to_string(h + 1) + "_" + "pdgId");

      m_truthHiggsesKinDecorators.emplace_back(
          std::vector<SG::AuxElement::Decorator<float>>());
      m_truthChildrenKinFromHiggsesDecorators.emplace_back(
          std::vector<SG::AuxElement::Decorator<std::vector<float>>>());
      m_truthInitialChildrenKinFromHiggsesDecorators.emplace_back(
          std::vector<SG::AuxElement::Decorator<std::vector<float>>>());
      for (const std::string &var : m_kinVars)
      {
        m_truthHiggsesKinDecorators[h].emplace_back(
            "truth_H" + std::to_string(h + 1) + "_" + var);
        m_truthChildrenKinFromHiggsesDecorators[h].emplace_back(
            "truth_children_fromH" + std::to_string(h + 1) + "_" + var);
        m_truthInitialChildrenKinFromHiggsesDecorators[h].emplace_back(
            "truth_initial_children_fromH" + std::to_string(h + 1) + "_" + var);
      }
    }

    for (const auto& decayMode : m_decayModes)
    {
      if(decayProducts_IDs.find(decayMode)==decayProducts_IDs.end())
        ATH_MSG_ERROR("Decay mode "<<decayMode<<" is not supported");
    }

    for (const std::string &var : m_kinVars)
    {
      m_truthHHKinDecorators.emplace_back(
          "truth_HH_" + var);
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
    auto truthParticlesContainer =
        !truthBSMParticles.empty() ? truthBSMParticles : truthSMParticles;

    /*
      Find and record truth particle information
    */
    std::vector<TruthScalar> higgses =
        getFinalHiggses(truthParticlesContainer);
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
      const xAOD::EventInfo &eventInfo, std::vector<TruthScalar>& higgses) const
  {
    if (higgses.size() < m_nHiggses)
    {
      // Default values
      higgses.resize(m_nHiggses, TruthScalar());
    }
    for (unsigned int h = 0; h < m_nHiggses; h++)
    {
      m_truthHiggsesPdgIdDecorators[h](eventInfo) = higgses[h].pdgId();
      m_truthChildrenPdgIdFromHiggsesDecorators[h](eventInfo) = higgses[h].children_pdgId();
      m_truthInitialChildrenPdgIdFromHiggsesDecorators[h](eventInfo) = higgses[h].initial_children_pdgId();

      for (size_t i = 0; i < m_kinVars.size(); i++)
      {
        m_truthHiggsesKinDecorators[h][i](eventInfo) = higgses[h].p4(i);
        m_truthChildrenKinFromHiggsesDecorators[h][i](eventInfo) =
            higgses[h].children_p4(i);
        m_truthInitialChildrenKinFromHiggsesDecorators[h][i](eventInfo) =
            higgses[h].initial_children_p4(i);
      }
    }

    // Reconstruct the HH system
    // Assume exactly two Higgs for now & check for nullptrs 
    if (higgses.size() < 2 || higgses[0] == nullptr || higgses[1] == nullptr) 
    {
      for (size_t i = 0; i < m_kinVars.size(); i++)
      {
        m_truthHHKinDecorators[i](eventInfo) = -999;
      }
    } else 
    {
      std::array<float, 4> coords = calcHHKinematics(higgses[0], higgses[1]);
      for (size_t i = 0; i < m_kinVars.size(); i++)
      {
        m_truthHHKinDecorators[i](eventInfo) = coords[i];
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
      const xAOD::TruthParticle *p, const std::unordered_set<int> ids) const
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
  TruthParticleInformationAlg ::getFinalChildren(const xAOD::TruthParticle *h) const
  {
    std::vector<const xAOD::TruthParticle *> children;
    const xAOD::TruthParticle *tmp(nullptr);
    for (size_t i = 0; i < h->nChildren(); i++)
    {
      if (msgLvl(MSG::VERBOSE))
      {
        verbosePrintParticleAndChildren(h->child(i));
      }

      std::unordered_set<int> childrenPdgIds;
      for (const auto& decayMode : m_decayModes)
      {
	for (const auto id : decayProducts_IDs.at(decayMode))
	{
	  childrenPdgIds.emplace(id);
	}
      }

      const xAOD::TruthParticle *final_child =
          getFinalParticleOfType(h->child(i), childrenPdgIds);
      if (!tmp || (final_child->barcode() != tmp->barcode()))
      {
        tmp = final_child;
        children.push_back(final_child);
      }
    }
    return children;
  }

  std::vector<const xAOD::TruthParticle *>
  TruthParticleInformationAlg ::getInitialChildren(const xAOD::TruthParticle *h) const
  {
    std::vector<const xAOD::TruthParticle *> initial_children;
    for (size_t i = 0; i < h->nChildren(); i++)
    {
      if (msgLvl(MSG::VERBOSE))
      {
        verbosePrintParticleAndChildren(h->child(i));
      }

      initial_children.push_back(h->child(i));
    }
    return initial_children;

  }

  std::vector<TruthScalar> TruthParticleInformationAlg ::getFinalHiggses(
      const xAOD::TruthParticleContainer &truthParticlesContainer) const
  {
    std::vector<TruthScalar> higgses;
    const xAOD::TruthParticle *tmp(nullptr);
    for (const xAOD::TruthParticle *tp : truthParticlesContainer)
    {
      if (msgLvl(MSG::VERBOSE))
      {
        verbosePrintParticleAndChildren(tp);
      }
      if ((tp->pdgId() == MC::HIGGSBOSON || tp->pdgId() == MC::SBOSONBSM))
      {
        const xAOD::TruthParticle *final_h =
	  getFinalParticleOfType(tp, {MC::HIGGSBOSON, MC::SBOSONBSM});
        if (!tmp || (final_h->barcode() != tmp->barcode()))
        {
          TruthScalar h = final_h;
          h.children(getFinalChildren(final_h));
          h.initial_children(getInitialChildren(final_h));
          higgses.push_back(h);
          tmp = final_h;
        }
      }
    }
    return higgses;
  }


   std::array<float, 4> TruthParticleInformationAlg::calcHHKinematics(
       const xAOD::TruthParticle *p1, const xAOD::TruthParticle *p2) const
   {
     
    ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double>> h1;
    ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double>> h2;
    h1.SetCoordinates(p1->pt(), p1->eta(), p1->phi(), p1->m());
    h2.SetCoordinates(p2->pt(), p2->eta(), p2->phi(), p2->m());
    auto hh = h1 + h2;

    ATH_MSG_DEBUG("Particle " << p1->pdgId() << ", pt " << p1->pt() << ", phi "
                                << p1->phi() << ", eta " << p1->eta() << ", mass "
                                << p1->m());
    ATH_MSG_DEBUG("Particle " << p2->pdgId() << ", pt " << p2->pt() << ", phi "
                                << p2->phi() << ", eta " << p2->eta() << ", mass "
                                << p2->m());
    ATH_MSG_DEBUG("Particle " << " pt " << hh.pt() << ", phi "
                                << hh.phi() << ", eta " << hh.eta() << ", mass "
                                << hh.M());

    std::array<float, 4> coords;
    hh.GetCoordinates(coords.begin());

    return coords;
   }


}
