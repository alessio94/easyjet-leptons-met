/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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

    ATH_CHECK (m_passesOR.initialize(m_systematicsList, m_inHandle));

    // Intialise syst-aware input/output decorators    
    ATH_CHECK (m_nSelPart.initialize(m_systematicsList, m_eventHandle));

    for(const auto& wp : m_photonWPNames){
      // Scale factors
      CP::SysReadDecorHandle<float> idSF(m_isMC ? "ph_id_effSF_"+wp+"_%SYS%" : "", this);
      m_ph_idSF.emplace_back(idSF);
      ATH_CHECK (m_ph_idSF.back().initialize(m_systematicsList, m_inHandle, SG::AllowEmpty));
      CP::SysReadDecorHandle<float> isoSF((m_isMC && wp.find("NonIso")!=std::string::npos) ? "ph_isol_effSF_"+wp+"_%SYS%" : "", this);
      m_ph_isoSF.emplace_back(isoSF);
      ATH_CHECK (m_ph_isoSF.back().initialize(m_systematicsList, m_inHandle, SG::AllowEmpty));

      CP::SysWriteDecorHandle<float> phSF(m_isMC ? "ph_effSF_"+wp+"_%SYS%" : "", this);
      m_ph_SF.emplace_back(phSF);
      ATH_CHECK (m_ph_SF.back().initialize(m_systematicsList, m_outHandle, SG::AllowEmpty));

      // Select flags
      CP::SysReadDecorHandle<char> sel_in("baselineSelection_"+wp+"_%SYS%", this);
      m_select_in.emplace_back(sel_in);
      ATH_CHECK (m_select_in.back().initialize(m_systematicsList, m_inHandle));

      CP::SysWriteDecorHandle<char> sel_out("baselineSelection_"+wp+"_%SYS%", this);
      m_select_out.emplace_back(sel_out);
      ATH_CHECK (m_select_out.back().initialize(m_systematicsList, m_outHandle));
    }

    // Initialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());    

    // Initialise vertex container for photon pointing
    ATH_CHECK (m_vertexContainerInKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode PhotonSelectorAlg::execute()
  {
    // vertex related objects and variables
    SG::ReadHandle<xAOD::VertexContainer> vertices_(m_vertexContainerInKey);
    const xAOD::Vertex* primary = nullptr;
    for (const xAOD::Vertex* vtx : *vertices_) {
      if (vtx->vertexType() == xAOD::VxType::PriVtx) {
        primary = vtx;
        break;
      }
    }
    if (!primary) {
      ATH_MSG_WARNING("Could not find a Primary vertex");
    }

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector()) {

      // Retrive inputs
      const xAOD::PhotonContainer *inContainer = nullptr;
      ANA_CHECK (m_inHandle.retrieve (inContainer, sys));      

      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      // Setup output 
      auto workContainer =
        std::make_unique<ConstDataVector<xAOD::PhotonContainer> >();

      for (const xAOD::Photon *photon : *inContainer)
      {
        // skip OR photons
        if ( m_checkOR ){
          bool passesOR = m_passesOR.get(*photon, sys);
          if ( !passesOR ) continue;
        }

	// Recompute photon pt and eta with respect to the hardest vertex z position
        auto thisPhoton = std::make_unique<xAOD::Photon>(*photon);

        if(m_recomputePhotons){
          photonWrtPoint::correctForZ(*thisPhoton, primary->z());
        };
        
        if (thisPhoton->pt() < m_minPt)
          continue;
        
        float this_photon_eta_abs = std::abs(thisPhoton->eta());
        if((this_photon_eta_abs > m_minEtaVeto &&
            this_photon_eta_abs < m_maxEtaVeto) ||
            (this_photon_eta_abs > m_maxEta ))
          continue ;

        // For some reason this decoration needs to be explicitly copied
        for(unsigned int i=0; i<m_photonWPNames.size(); i++){
          std::string wp = m_photonWPNames[i];
          if(m_isMC){
            float SF = m_ph_idSF[i].get(*thisPhoton,sys);
            if(wp.find("NonIso")!=std::string::npos) SF *= m_ph_isoSF[i].get(*thisPhoton,sys);
            m_ph_SF[i].set(*thisPhoton, SF, sys);
          }
          m_select_out[i].set(*thisPhoton, m_select_in[i].get(*thisPhoton,sys), sys);
        }

        workContainer->push_back(thisPhoton.release());
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
