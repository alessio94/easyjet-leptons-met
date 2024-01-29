/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Minori Fujimoto

#include "ElectronSelectorAlg.h"
#include "AthContainers/AuxElement.h"
#include <xAODEgamma/ElectronContainer.h>
#include "FourMomUtils/xAODP4Helpers.h"
#include <AsgDataHandles/ReadDecorHandle.h>

namespace Easyjet
{
  ElectronSelectorAlg::ElectronSelectorAlg(const std::string &name,
                                       ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator) { }

  StatusCode ElectronSelectorAlg::initialize()
  {
    // Read syst-aware input/output handles
    ATH_CHECK (m_inHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK (m_outHandle.initialize(m_systematicsList));

    // Intialise syst-aware input/output decorators    
    ATH_CHECK (m_nSelPart.initialize(m_systematicsList, m_eventHandle));

    m_ORElDecorKey = m_inHandle.getNamePattern() + "." + m_ORElDecorName;
    ATH_CHECK (m_ORElDecorKey.initialize());

    if(m_isMC){
      m_ele_recoSF = CP::SysReadDecorHandle<float>("el_reco_effSF_"+m_eleWPName+"_%SYS%", this);
      m_ele_idSF = CP::SysReadDecorHandle<float>("el_id_effSF_"+m_eleWPName+"_%SYS%", this);
      if(m_eleWPName.value().find("NonIso")!=std::string::npos) m_isoIncluded = false;
      else m_ele_isoSF = CP::SysReadDecorHandle<float>("el_isol_effSF_"+m_eleWPName+"_%SYS%", this);
      m_ele_SF = CP::SysWriteDecorHandle<float>("el_effSF_"+m_eleWPName+"_%SYS%", this);
    }

    ATH_CHECK (m_ele_recoSF.initialize(m_systematicsList, m_inHandle, SG::AllowEmpty));
    ATH_CHECK (m_ele_idSF.initialize(m_systematicsList, m_inHandle, SG::AllowEmpty));
    ATH_CHECK (m_ele_isoSF.initialize(m_systematicsList, m_inHandle, SG::AllowEmpty));
    ATH_CHECK (m_ele_SF.initialize(m_systematicsList, m_outHandle, SG::AllowEmpty));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());    

    return StatusCode::SUCCESS;
  }

  StatusCode ElectronSelectorAlg::execute()
  {
    SG::ReadDecorHandle<xAOD::ElectronContainer, char> ORDecorHandle(m_ORElDecorKey);

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector()) {

      // Retrive inputs
      const xAOD::ElectronContainer *inContainer = nullptr;
      ANA_CHECK (m_inHandle.retrieve (inContainer, sys));      

      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      // Setup output 
      auto workContainer =
        std::make_unique<ConstDataVector<xAOD::ElectronContainer> >(
            SG::VIEW_ELEMENTS);

      // loop over electrons 
      for (const xAOD::Electron *electron : *inContainer)
      {
        // skip OR electrons
        if ( m_checkOR ){
          bool ispassOREl = ORDecorHandle(*electron);
          if ( !ispassOREl ) continue;
        }

        // cuts
        if (electron->pt() < m_minPt)
          continue;
        
        float this_electron_eta_abs = std::abs(electron->eta());
        if ((this_electron_eta_abs > m_minEtaVeto &&
            this_electron_eta_abs < m_maxEtaVeto) ||
            (this_electron_eta_abs > m_maxEta ))
          continue;

	// For some reason this decoration needs to be explicitly copied
	if(m_isMC){
	  float SF = m_ele_recoSF.get(*electron,sys) * m_ele_idSF.get(*electron,sys);
	  if(m_isoIncluded) SF *= m_ele_isoSF.get(*electron,sys);
	  m_ele_SF.set(*electron, SF, sys);
	}

        // If cuts are passed, save the object
        workContainer->push_back(electron);
      }
      
      int nElectrons = workContainer->size();
      m_nSelPart.set(*event, nElectrons, sys);
      
      // if we have less than the requested nr, empty the workcontainer to write
      // defaults/return empty container
      if (nElectrons < m_minimumAmount)
      {
        workContainer->clear();
        nElectrons = 0;
      }
      
      // sort and truncate
      int nKeep;
      if (nElectrons < m_truncateAtAmount) nKeep = nElectrons;
      else nKeep = m_truncateAtAmount;
      
      if (m_pTsort)
      {
        // if we give -1, sort the whole container
        if (m_truncateAtAmount == -1)
          {
            nKeep = nElectrons;
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

