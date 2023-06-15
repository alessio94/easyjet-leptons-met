/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Abraham Tishelman-Charny

#include "PhotonSelectorAlg.h"
#include "AthContainers/AuxElement.h"
#include <AthContainers/ConstDataVector.h>
#include <xAODEgamma/PhotonContainer.h>
#include "FourMomUtils/xAODP4Helpers.h"
#include <xAODEgamma/Photon.h>

namespace Easyjet
{
  PhotonSelectorAlg::PhotonSelectorAlg(const std::string &name,
                                       ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("LeadPho_ptOverMyy_min", m_LeadPho_ptOverMyy_min);
    declareProperty("SubleadPho_ptOverMyy_min", m_SubleadPho_ptOverMyy_min);
    declareProperty("etaBounds", m_etaBounds);
    declareProperty("minimumAmount", m_minimumAmount);
    declareProperty("truncateAtAmount", m_truncateAtAmount);
    declareProperty("pTsort", m_pTsort);
  }

  StatusCode PhotonSelectorAlg::initialize()
  {
    ATH_CHECK(m_containerInKey.initialize());
    ATH_CHECK(m_EventInfoKey.initialize());
    ATH_CHECK(m_containerOutKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode PhotonSelectorAlg::execute()
  {
    // container we read in
    SG::ReadHandle<xAOD::PhotonContainer> inContainer(m_containerInKey);
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    ATH_CHECK(inContainer.isValid());
    ATH_CHECK(eventInfo.isValid());

    // make some accessors and decorators

    SG::AuxElement::Decorator<unsigned int> nSelectedParticles_dec(
        m_containerOutKey.key() + "_n");

    // fill workContainer with "views" of the inContainer
    // see TJ's tutorial for this

    auto workContainer =
        std::make_unique<ConstDataVector<xAOD::PhotonContainer> >(
            SG::VIEW_ELEMENTS);

    float this_photon_eta_abs;
    // loop over photons - must have at least 2 for diphoton object
    for (const xAOD::Photon *photon : *inContainer)
    {
      this_photon_eta_abs = std::abs(photon->eta());
      // cuts
      if ((this_photon_eta_abs > m_etaBounds[0] &&
           this_photon_eta_abs < m_etaBounds[1]) ||
          (this_photon_eta_abs > m_etaBounds[2]))
        continue;

      // If cuts are passed, save the object
      workContainer->push_back(photon);
    }

    int nPhotons = workContainer->size();

    float LeadPho_ptOverMyy = -99;
    float SubleadPho_ptOverMyy = -99;
    float diphoton_mass = -99;

    // Add lead/subleading photon requirements

    if (nPhotons >= 2)
    {

      const xAOD::Photon *Leading_Photon = *inContainer->begin();
      auto secondPhotonIterator = std::next(inContainer->begin(), 1);
      const xAOD::Photon *Subleading_Photon = *secondPhotonIterator;

      // Create TLorentzVector objects from the photon four-momenta
      TLorentzVector leadingPhotonP4 = Leading_Photon->p4();
      TLorentzVector subleadingPhotonP4 = Subleading_Photon->p4();

      // Calculate the invariant mass
      diphoton_mass = (leadingPhotonP4 + subleadingPhotonP4).M();

      LeadPho_ptOverMyy = Leading_Photon->pt() / diphoton_mass;
      SubleadPho_ptOverMyy = Subleading_Photon->pt() / diphoton_mass;

      if ((LeadPho_ptOverMyy < m_LeadPho_ptOverMyy_min) ||
          (SubleadPho_ptOverMyy < m_SubleadPho_ptOverMyy_min))
      {
        workContainer->clear();
        nPhotons = 0; // event failed, artifically set this with nPhotons = 0.
                      // may want to revisit.
      }
    }

    // decorate nr of selected particles to the eventinfo
    nSelectedParticles_dec(*eventInfo) = nPhotons;

    // if we have less than the requested nr, empty the workcontainer to write
    // defaults/return empty container
    if (nPhotons < m_minimumAmount)
    {
      workContainer->clear();
      nPhotons = 0;
    }

    // sort and truncate
    int nKeep;
    if (nPhotons < m_truncateAtAmount)
    {
      nKeep = nPhotons;
    }
    else
    {
      nKeep = m_truncateAtAmount;
    }

    if (m_pTsort)
    {
      // if we give -1, sort the whole container
      if (m_truncateAtAmount == -1)
      {
        nKeep = nPhotons;
      }
      std::partial_sort(
          workContainer->begin(), // Iterator from which to start sorting
          workContainer->begin() + nKeep, // Use begin + N to sort first N
          workContainer->end(), // Iterator marking the end of range to sort
          [](const xAOD::IParticle *left, const xAOD::IParticle *right)
      { return left->pt() > right->pt(); }); // lambda function here just
                                             // handy, could also be another
                                             // function that returns bool

      // keep only the requested amount
      workContainer->erase(workContainer->begin() + nKeep,
                           workContainer->end());
    }

    // write to eventstore
    SG::WriteHandle<ConstDataVector<xAOD::PhotonContainer> > Writer(
        m_containerOutKey);
    ATH_CHECK(Writer.record(std::move(workContainer)));

    return StatusCode::SUCCESS;
  }
}
