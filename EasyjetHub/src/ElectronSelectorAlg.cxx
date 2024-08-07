/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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

    if (m_electronAmount > 0)
    {
      for (int i = 0; i < m_electronAmount; i++)
      {
        std::string index = std::to_string(i + 1);
        CP::SysWriteDecorHandle<bool> whandle{"isElectron" + index + "_%SYS%", this};
        m_leadBranches.emplace("isElectron" + index, whandle);
        ATH_CHECK(m_leadBranches.at("isElectron" + index).initialize(m_systematicsList, m_inHandle));
      };
    }
    ANA_CHECK (m_isSelectedElectron.initialize(m_systematicsList, m_inHandle));

    ATH_CHECK (m_passesOR.initialize(m_systematicsList, m_inHandle));

    if(m_tightEleWPs.empty()) m_tightEleWPs.setValue({m_looseEleWP});

    m_select_loose_in = CP::SysReadDecorHandle<char>("baselineSelection_"+ m_looseEleWP +"_%SYS%", this);
    ATH_CHECK (m_select_loose_in.initialize(m_systematicsList, m_inHandle));

    for(const auto& wp : m_tightEleWPs){
      // Scale factors not available for DNN yet
      bool sfAvailable = m_isMC && !m_saveDummySF && wp.find("DNN")==std::string::npos;
      m_ele_recoSF.emplace_back(sfAvailable ? "el_reco_effSF_"+wp+"_%SYS%" : "", this);
      m_ele_idSF.emplace_back(sfAvailable ? "el_id_effSF_"+wp+"_%SYS%" : "", this);
      m_ele_isoSF.emplace_back
	((sfAvailable && wp.find("NonIso")==std::string::npos) ?
	 "el_isol_effSF_"+wp+"_%SYS%" : "", this);
      m_ele_SF.emplace_back(m_isMC ? "el_effSF_"+wp+"_%SYS%" : "", this);
      
      // Select flags
      m_select_tight_in.emplace_back("baselineSelection_"+wp+"_%SYS%", this);
      m_select_out.emplace_back("baselineSelection_"+wp+"_%SYS%", this);
    }

    if(m_isMC){
      for(const auto& trig : m_eleTrigSF){
	m_eleTriggerSF_in.emplace_back("el_trigEffSF_"+trig+"_%SYS%", this);
	m_eleTriggerSF_out.emplace_back("el_trigEffSF_"+trig+"_%SYS%", this);
      }
    }

    for(auto& handle : m_ele_recoSF)
      ATH_CHECK(handle.initialize(m_systematicsList, m_inHandle, SG::AllowEmpty));
    for(auto& handle : m_ele_idSF)
      ATH_CHECK(handle.initialize(m_systematicsList, m_inHandle, SG::AllowEmpty));
    for(auto& handle : m_ele_isoSF)
      ATH_CHECK(handle.initialize(m_systematicsList, m_inHandle, SG::AllowEmpty));
    for(auto& handle : m_ele_SF)
      ATH_CHECK(handle.initialize(m_systematicsList, m_outHandle, SG::AllowEmpty));
    for(auto& handle : m_eleTriggerSF_in)
      ATH_CHECK(handle.initialize(m_systematicsList, m_inHandle, SG::AllowEmpty));
    for(auto& handle : m_eleTriggerSF_out)
      ATH_CHECK(handle.initialize(m_systematicsList, m_outHandle, SG::AllowEmpty));
    for(auto& handle : m_select_tight_in)
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

        if(!m_select_loose_in.get(*electron, sys)) continue;

        // skip OR electrons
        if ( m_checkOR ){
          bool passesOR = m_passesOR.get(*electron, sys);
          if ( !passesOR ) continue;
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
        for(unsigned int i=0; i<m_tightEleWPs.size(); i++){
          std::string wp = m_tightEleWPs[i];
          if(m_isMC){
	    float SF = 1.;
	    if(!m_saveDummySF && wp.find("DNN")==std::string::npos){
	      SF = m_ele_recoSF[i].get(*electron,sys) * m_ele_idSF[i].get(*electron,sys);
	      if(wp.find("NonIso")==std::string::npos) SF *= m_ele_isoSF[i].get(*electron,sys);
	    }
            m_ele_SF[i].set(*electron, SF, sys);
          }
          m_select_out[i].set(*electron, m_select_tight_in[i].get(*electron,sys), sys);
        }

        if(m_isMC){
          for(unsigned int i=0; i<m_eleTrigSF.size(); i++){
            m_eleTriggerSF_out[i].set
              (*electron, m_eleTriggerSF_in[i].get(*electron, sys), sys);
          }
        }
        
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

