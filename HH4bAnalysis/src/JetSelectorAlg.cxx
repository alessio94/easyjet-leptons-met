/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Frederic Renner

#include "JetSelectorAlg.h"
#include "AthContainers/AuxElement.h"
#include <AthContainers/ConstDataVector.h>
#include <xAODJet/JetContainer.h>

namespace HH4B
{
  JetSelectorAlg ::JetSelectorAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("bTagWP", m_bTagWP);
    declareProperty("minPt", m_minPt);
    declareProperty("maxEta", m_maxEta);
    declareProperty("howManyToKeep", m_howManyToKeep);
    declareProperty("pTsort", m_pTsort);
  }

  StatusCode JetSelectorAlg ::initialize()
  {
    ATH_CHECK(m_containerInKey.initialize());
    ATH_CHECK(m_EventInfoKey.initialize());
    ATH_CHECK(m_containerOutKey.initialize());

    // make decorators for the four vectors
    std::vector<std::string> vars{
        "pt", "eta", "phi", "m", "isCentral",
    };
    for (std::string var : vars)
    {
      std::string deco_var = m_containerOutKey.key() + "_" + var;
      SG::AuxElement::Decorator<std::vector<float>> deco(deco_var);
      m_fourVecDecos.emplace(deco_var, deco);
    };
    return StatusCode::SUCCESS;
  }

  StatusCode JetSelectorAlg ::execute()
  {
    // container we read in
    SG::ReadHandle<xAOD::JetContainer> inContainer(m_containerInKey);
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    ATH_CHECK(inContainer.isValid());
    ATH_CHECK(eventInfo.isValid());

    // make some accessors and decorators
    SG::AuxElement::ConstAccessor<char> isBtag("ftag_select_" + m_bTagWP);
    SG::AuxElement::Decorator<unsigned int> nSelectedParticles_dec(
        m_containerOutKey.key() + "_n");
    SG::AuxElement::Decorator<char> isCentral_dec("isCentral");
    SG::AuxElement::Accessor<char> isCentral_acc("isCentral");

    // fill workContainer with "views" of the inContainer
    // see TJ's tutorial for this
    auto workContainer = std::make_unique<ConstDataVector<xAOD::JetContainer>>(
        SG::VIEW_ELEMENTS);

    // check if a btag wp is given
    bool WPgiven = false;
    if (m_bTagWP != "")
    {
      WPgiven = true;
    }
    for (const xAOD::Jet *jet : *inContainer)
    {
      // cuts
      if (jet->pt() < m_minPt && std::abs(jet->eta() > m_maxEta))
      {
        continue;
      }

      // decorate if particles are central
      if (jet->pt() > 25000. && std::abs(jet->eta()) < 2.5)
      {
        isCentral_dec(*jet) = 1;
      }
      else
      {
        isCentral_dec(*jet) = 0;
      }

      // if no btag wp is given take all
      if (WPgiven)
      {
        if (isBtag(*jet))
        {
          workContainer->push_back(jet);
        }
      }
      else
      {
        workContainer->push_back(jet);
      }
    }

    int nParticles = workContainer->size();
    // decorate nr of selected particles to the eventinfo
    nSelectedParticles_dec(*eventInfo) = nParticles;

    // sort and make sure we have at least the amount we want to keep
    if (m_pTsort && nParticles >= m_howManyToKeep)
    {
      // if we give -1, sort the whole container
      if (m_howManyToKeep == -1)
      {
        m_howManyToKeep = nParticles;
      }
      std::partial_sort(
          workContainer->begin(), // Iterator from which to start sorting
          workContainer->begin() +
              m_howManyToKeep,  // Use begin + N to sort first N
          workContainer->end(), // Iterator marking the end of range to sort
          [](const xAOD::IParticle *left, const xAOD::IParticle *right)
          {
            return left->pt() > right->pt();
          }); // lambda function here just handy, could also be another
              // function that returns bool
              // keep only the requested amount
      workContainer->erase(workContainer->begin() + m_howManyToKeep,
                           workContainer->end());
    }

    // decorate eventInfo
    std::vector<float> jet_pt;
    std::vector<float> jet_eta;
    std::vector<float> jet_phi;
    std::vector<float> jet_m;
    std::vector<float> jet_isCentral;

    // set defaults
    if (workContainer->size() == 0)
    {
      jet_pt.push_back(-100);
      jet_eta.push_back(-100);
      jet_phi.push_back(-100);
      jet_m.push_back(-100);
      jet_isCentral.push_back(-100);
    }
    else
    {
      for (const xAOD::Jet *jet : *workContainer)
      {
        jet_pt.push_back(jet->pt());
        jet_eta.push_back(jet->eta());
        jet_phi.push_back(jet->phi());
        jet_m.push_back(jet->m());
        jet_isCentral.push_back(float(isCentral_acc(*jet)));
      }
    }
    // clang-format off
      m_fourVecDecos.at(m_containerOutKey.key() + "_pt")(*eventInfo) = jet_pt;
      m_fourVecDecos.at(m_containerOutKey.key() + "_eta")(*eventInfo) = jet_eta;
      m_fourVecDecos.at(m_containerOutKey.key() + "_phi")(*eventInfo) = jet_phi;
      m_fourVecDecos.at(m_containerOutKey.key() + "_m")(*eventInfo) = jet_m;
      m_fourVecDecos.at(m_containerOutKey.key() + "_isCentral")(*eventInfo) = jet_isCentral;
    // clang-format on

    // write to eventstore
    SG::WriteHandle<ConstDataVector<xAOD::JetContainer>> Writer(
        m_containerOutKey);
    ATH_CHECK(Writer.record(std::move(workContainer)));

    return StatusCode::SUCCESS;
  }
}
