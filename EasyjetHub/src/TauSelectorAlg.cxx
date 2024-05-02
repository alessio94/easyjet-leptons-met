/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TauSelectorAlg.h"
#include <AsgDataHandles/ReadDecorHandle.h>

namespace Easyjet
{
  TauSelectorAlg::TauSelectorAlg(const std::string &name,
                                       ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator) { }

  StatusCode TauSelectorAlg::initialize()
  {

    // Read syst-aware input/output handles
    ATH_CHECK (m_inHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK (m_outHandle.initialize(m_systematicsList));

    m_IDTauDecorKey = m_inHandle.getNamePattern() + "." + m_IDTauDecorName;
    m_antiTauDecorKey = m_inHandle.getNamePattern() + "." + m_antiTauDecorName;

    ATH_CHECK (m_IDTauDecorKey.initialize());
    ATH_CHECK (m_antiTauDecorKey.initialize());

    ATH_CHECK (m_passesOR.initialize(m_systematicsList, m_inHandle));

    if(m_isMC){
      m_tau_SF_in = CP::SysReadDecorHandle<float>("tau_effSF_"+m_tauWPName+"_%SYS%", this);
      m_tau_SF_out = CP::SysWriteDecorHandle<float>("tau_effSF_"+m_tauWPName+"_%SYS%", this);
    }
    ATH_CHECK (m_tau_SF_in.initialize(m_systematicsList, m_inHandle, SG::AllowEmpty));
    ATH_CHECK (m_tau_SF_out.initialize(m_systematicsList, m_outHandle, SG::AllowEmpty));

    m_select_in = CP::SysReadDecorHandle<char>("baselineSelection_"+m_tauWPName+"_%SYS%", this);
    m_select_out = CP::SysWriteDecorHandle<char>("baselineSelection_"+m_tauWPName+"_%SYS%", this);
    ATH_CHECK (m_select_in.initialize(m_systematicsList, m_inHandle));
    ATH_CHECK (m_select_out.initialize(m_systematicsList, m_outHandle));

    // Initialise syst-aware input/output decorators 
    ATH_CHECK (m_nSelPart.initialize(m_systematicsList, m_eventHandle));

    // Initialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());    

    return StatusCode::SUCCESS;
  }

  StatusCode TauSelectorAlg::execute()
  {

    SG::ReadDecorHandle<xAOD::TauJetContainer, char> idTauDecorHandle(m_IDTauDecorKey);
    SG::ReadDecorHandle<xAOD::TauJetContainer, char> antiTauDecorHandle(m_antiTauDecorKey);

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector()) {

      // Retrive inputs
      const xAOD::TauJetContainer *inContainer = nullptr;
      ANA_CHECK (m_inHandle.retrieve (inContainer, sys));      

      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      // Setup output 
      auto workContainer =
        std::make_unique<ConstDataVector<xAOD::TauJetContainer> >(
            SG::VIEW_ELEMENTS);
    
      // loop over taus 
      for (const xAOD::TauJet *tau : *inContainer) {

        // If not ID tau nor anti tau, skip
        if(m_keepAntiTaus){
          bool isTauID = idTauDecorHandle(*tau) || antiTauDecorHandle(*tau);
          if( !isTauID ) continue;
        }

        // If not passing OR, skip
        if( m_checkOR ){
          bool passesOR = m_passesOR.get(*tau, sys);
          if ( !passesOR ) continue;
        }
	
        if (tau->pt() < m_minPt)
          continue;
    
        float this_tau_eta_abs = std::abs(tau->eta());
        if ((this_tau_eta_abs > m_minEtaVeto &&
            this_tau_eta_abs < m_maxEtaVeto) ||
            (this_tau_eta_abs > m_maxEta))
          continue;

        // For some reason this decoration needs to be explicitly copied
        if(m_isMC) m_tau_SF_out.set(*tau, m_tau_SF_in.get(*tau,sys), sys);
        m_select_out.set(*tau, m_select_in.get(*tau,sys), sys);

        // If cuts are passed, save the object
        workContainer->push_back(tau);
      }

      int nTaus = workContainer->size();      
      m_nSelPart.set(*event, nTaus, sys);
      
      // if we have less than the requested nr, empty the workcontainer to write
      // defaults/return empty container
      if (nTaus < m_minimumAmount) {
        workContainer->clear();
        nTaus = 0;
      }
      
      // sort and truncate
      int nKeep;
      if (nTaus < m_truncateAtAmount) nKeep = nTaus;
      else nKeep = m_truncateAtAmount;
      
      if (m_pTsort) {
        // if we give -1, sort the whole container
        if (m_truncateAtAmount == -1)  nKeep = nTaus;
    
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

