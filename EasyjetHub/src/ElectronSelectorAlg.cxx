/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Minori Fujimoto

#include "ElectronSelectorAlg.h"

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

    for (int i = 0; i < m_electronAmount; i++){
      std::string index = std::to_string(i + 1);
      CP::SysWriteDecorHandle<bool> whandle{"isElectron" + index + "_%SYS%", this};
      m_leadBranches.emplace("isElectron" + index, whandle);
      ATH_CHECK(m_leadBranches.at("isElectron" + index).initialize(m_systematicsList, m_inHandle));
    }
    ANA_CHECK (m_isSelectedElectron.initialize(m_systematicsList, m_inHandle));

    // Select flags
    for(const auto& wp : m_eleWPs){
      m_select_in.emplace_back("baselineSelection_"+wp+"_%SYS%", this);
      m_select_out.emplace_back("baselineSelection_"+wp+"_%SYS%", this);
    }

    for(auto& handle : m_select_in)
      ATH_CHECK(handle.initialize(m_systematicsList, m_inHandle, SG::AllowEmpty));
    for(auto& handle : m_select_out)
      ATH_CHECK(handle.initialize(m_systematicsList, m_outHandle, SG::AllowEmpty));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());    

    return StatusCode::SUCCESS;
  }

  StatusCode ElectronSelectorAlg::execute()
  {
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
        m_isSelectedElectron.set(*electron, false, sys);

        // cuts
        if (electron->pt() < m_minPt)
          continue;

        if (std::abs(electron->eta()) > m_maxEta)
          continue;

        // For some reason this decoration needs to be explicitly copied
        for(unsigned int i=0; i<m_eleWPs.size(); i++)
          m_select_out[i].set(*electron, m_select_in[i].get(*electron,sys), sys);
        
        // If cuts are passed, save the object
        workContainer->push_back(electron);
        m_isSelectedElectron.set(*electron, true, sys);
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

      if(m_electronAmount > 0){
        int nElectron = 0;
        for (const xAOD::Electron *electron : *workContainer) {
          nElectron++;
          m_leadBranches.at("isElectron"+std::to_string(nElectron)).set(*electron, true, sys);
          if ( nElectron == m_electronAmount ) break;
        }
      }
      
      // Write to eventstore
      ATH_CHECK(m_outHandle.record(std::move(workContainer), sys));   
    }
      
    return StatusCode::SUCCESS;
  }
}

