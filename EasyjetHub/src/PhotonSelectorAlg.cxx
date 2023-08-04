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

    // Read syst-aware input/output handles
    ATH_CHECK (m_inHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK (m_outHandle.initialize(m_systematicsList));

    // Intialise syst-aware input/output decorators    
    ATH_CHECK (m_nSelPart.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_isLoose.initialize(m_systematicsList, m_inHandle));

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

      // Cut flow
      int passOQ = -1; 
      int passPtCut = -1;
      int passEtaCut = -1;
      int passCleaning = -1;
      float pt_cut = 25000;
      int passPID = -1;

      for (const xAOD::Photon *photon : *inContainer)
	{
	  passPtCut = 0;
	  passEtaCut = 0;
	  passOQ = photon->isGoodOQ(xAOD::EgammaParameters::BADCLUSPHOTON); // From DF
	  passCleaning = 0;
	  passPID = 0;
	  passPID = m_isLoose.get(*photon, sys); // From DF
	  
	  if (!((photon->OQ() & 1073741824) != 0 ||
		(
		 (photon->OQ() & 134217728) != 0 &&
		 (photon->showerShapeValue(xAOD::EgammaParameters::Reta) > 0.98
		  || photon->showerShapeValue(xAOD::EgammaParameters::f1) > 0.4
		  || (photon->OQ() & 67108864) != 0
		  )
		 )
		)) {
	    passCleaning = 1;
	  } else {
	    passCleaning = 0;
	  }
	  
	  if(photon->pt() > pt_cut) passPtCut = 1;
	  
	  if(m_etaBounds[0] == -1 && m_etaBounds[1] == -1 && m_etaBounds[2] == -1){
	    passEtaCut = 1;
	  }
	  else{
	    if((abs(photon->eta()) <= m_etaBounds[0] || abs(photon->eta()) >= m_etaBounds[1]) && abs(photon->eta()) <= m_etaBounds[2]) passEtaCut = 1;
	  }
	  
	  if(   passOQ
		&& passCleaning
		&& passPtCut
		&& passEtaCut
		&& passPID
		){
	    workContainer->push_back(photon);
	  }
	  
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

      // Write to eventstore
      ATH_CHECK(m_outHandle.record(std::move(workContainer), sys));   
    }

    return StatusCode::SUCCESS;
  }
}
