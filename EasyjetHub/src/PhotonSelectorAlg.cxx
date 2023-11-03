/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Abraham Tishelman-Charny

#include "PhotonSelectorAlg.h"
#include "AthContainers/AuxElement.h"
#include <xAODEgamma/PhotonContainer.h>
#include "FourMomUtils/xAODP4Helpers.h"

namespace Easyjet
{
  PhotonSelectorAlg::PhotonSelectorAlg(const std::string &name,
                                       ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator) { }

  StatusCode PhotonSelectorAlg::initialize()
  {

    // Read syst-aware input/output handles
    ATH_CHECK (m_inHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK (m_outHandle.initialize(m_systematicsList));

    // Intialise syst-aware input/output decorators    
    ATH_CHECK (m_nSelPart.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_isLoose.initialize(m_systematicsList, m_inHandle));
    ATH_CHECK (m_isClean.initialize(m_systematicsList, m_inHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());    

    return StatusCode::SUCCESS;
  }

  StatusCode PhotonSelectorAlg::execute()
  {

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector()) {

      // Retrive inputs
      const xAOD::PhotonContainer *inContainer = nullptr;
      ANA_CHECK (m_inHandle.retrieve (inContainer, sys));      

      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      // Setup output 
      auto workContainer =
        std::make_unique<ConstDataVector<xAOD::PhotonContainer> >(
            SG::VIEW_ELEMENTS);

      for (const xAOD::Photon *photon : *inContainer)
      {
        // From DF, Quality
        if(!photon->isGoodOQ(xAOD::EgammaParameters::BADCLUSPHOTON))
          continue ; 
        // From DF, Identification
        if(!m_isLoose.get(*photon, sys))
          continue ;
        
        // E-gamma cleaning
        if(!m_isClean.get(*photon, sys))
          continue ;
        
        if (photon->pt() < m_minPt)
          continue;
        
        float this_photon_eta_abs = std::abs(photon->eta());
        if((this_photon_eta_abs > m_minEtaVeto &&
            this_photon_eta_abs < m_maxEtaVeto) ||
            (this_photon_eta_abs > m_maxEta ))
          continue ;
        
        workContainer->push_back(photon);  
      }

      int nPhotons = workContainer->size();
      m_nSelPart.set(*event, nPhotons, sys);

      // if we have less than the requested number, empty the workcontainer to write
      // defaults/return empty container
      if (nPhotons < m_minimumAmount)
      {
        workContainer->clear();
        nPhotons = 0;
      }
      
      // sort and truncate
      int nKeep;
      if (nPhotons < m_truncateAtAmount) nKeep = nPhotons;
      else nKeep = m_truncateAtAmount;

      if (m_pTsort){
        // if we give -1, sort the whole container
        if (m_truncateAtAmount == -1) nKeep = nPhotons;
        
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

      // Write to eventstore
      ATH_CHECK(m_outHandle.record(std::move(workContainer), sys));   
    }

    return StatusCode::SUCCESS;
  }
}
