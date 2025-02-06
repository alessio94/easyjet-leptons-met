/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Abraham Tishelman-Charny

#include "PhotonSelectorAlg.h"
#include <AsgDataHandles/ReadHandle.h>
#include "egammaUtils/egPhotonWrtPoint.h"

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

    for (int i = 0; i < m_photonAmount; i++){
      std::string index = std::to_string(i + 1);
      CP::SysWriteDecorHandle<bool> whandle{"isPhoton" + index + "_%SYS%", this};
      m_leadBranches.emplace("isPhoton" + index, whandle);
      ATH_CHECK(m_leadBranches.at("isPhoton" + index).initialize(m_systematicsList, m_inHandle));
    }

    ATH_CHECK (m_isSelectedPhoton.initialize(m_systematicsList, m_inHandle));

    // Select flags
    for(const auto& wp : m_photonWPs){
      m_select_in.emplace_back("baselineSelection_"+wp+"_%SYS%", this);
      m_select_out.emplace_back("baselineSelection_"+wp+"_%SYS%", this);
    }

    for(auto& handle : m_select_in)
      ATH_CHECK(handle.initialize(m_systematicsList, m_inHandle, SG::AllowEmpty));
    for(auto& handle : m_select_out)
      ATH_CHECK(handle.initialize(m_systematicsList, m_outHandle, SG::AllowEmpty));

    // Intialise syst-aware input/output decorators
    ATH_CHECK (m_nSelPart.initialize(m_systematicsList, m_eventHandle));

    // Initialise syst list (must come after all syst-aware inputs and outputs)
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
        std::make_unique<ConstDataVector<xAOD::PhotonContainer> >(SG::VIEW_ELEMENTS);

      for (const xAOD::Photon *photon : *inContainer)
      {

	// selected photons for systematics
        m_isSelectedPhoton.set(*photon, false, sys);
        
        if (photon->pt() < m_minPt)
          continue;

        if(std::abs(photon->eta()) > m_maxEta)
          continue ;

        // For some reason this decoration needs to be explicitly copied
        for(unsigned int i=0; i<m_photonWPs.size(); i++)
          m_select_out[i].set(*photon, m_select_in[i].get(*photon,sys), sys);

        m_isSelectedPhoton.set(*photon, true, sys);
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

      //lead/sublead photon
      if(m_photonAmount > 0){
        int nPhoton = 0;
        for (const xAOD::Photon *photon : *workContainer) {
          nPhoton++;
          m_leadBranches.at("isPhoton"+std::to_string(nPhoton)).set(*photon, true, sys);
          if ( nPhoton == m_photonAmount ) break;
        }
      }

      // Write to eventstore
      ATH_CHECK(m_outHandle.record(std::move(workContainer), sys));   
    }

    return StatusCode::SUCCESS;
  }
}
