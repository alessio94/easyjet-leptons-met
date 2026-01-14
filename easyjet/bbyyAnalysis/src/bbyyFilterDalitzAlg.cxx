/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "bbyyFilterDalitzAlg.h"
#include <EventBookkeeperTools/FilterReporter.h>

namespace HHBBYY
{

  bbyyFilterDalitzAlg::bbyyFilterDalitzAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
  }


  StatusCode bbyyFilterDalitzAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("      bbyyFilterDalitzAlg      \n");
    ATH_MSG_INFO("*********************************\n");

    if (!m_truthParticleBSMInKey.empty())
      ATH_CHECK(m_truthParticleBSMInKey.initialize());
    if (!m_truthParticleSMInKey.empty())
      ATH_CHECK(m_truthParticleSMInKey.initialize());
 
    ATH_CHECK (m_filterParams.initialize());   
 
    return StatusCode::SUCCESS;
  }


  StatusCode bbyyFilterDalitzAlg::execute()
  {
    FilterReporter filter (m_filterParams, false);
  
    SG::ReadHandle<xAOD::TruthParticleContainer> truthSMParticles(m_truthParticleSMInKey);
    ATH_CHECK(truthSMParticles.isValid());
    SG::ReadHandle<xAOD::TruthParticleContainer> truthBSMParticles(m_truthParticleBSMInKey);
    ATH_CHECK(truthBSMParticles.isValid());

    auto truthParticlesContainer =
      !(*truthBSMParticles).empty() ? (*truthBSMParticles) : (*truthSMParticles);
    if (!isDalitz(truthParticlesContainer)){
      // only save non Dalitz events in output
      filter.setPassed(true);
    }

    return StatusCode::SUCCESS;
  }

  StatusCode bbyyFilterDalitzAlg::finalize()
  {

    return StatusCode::SUCCESS;

  }


  bool bbyyFilterDalitzAlg::isDalitz(const xAOD::TruthParticleContainer &truthPtcls)
  {  
    // copy paste from HGam
    // not sure but seems to be working only for Pythia8 
    if (truthPtcls.empty()) { ATH_MSG_ERROR("isDalitz FATAL: truthPtcls is NULL (probably not saved in input sample)");}
    
    for (auto *ptcl : truthPtcls) // if H -> y*
      if (abs(ptcl->pdgId()) == 25 && (ptcl->status() == 62 || ptcl->status() == 52) && ptcl->nChildren() >= 2 &&
          ((ptcl->child(0) && ptcl->child(0)->pdgId() == 22 && ptcl->child(0)->status() != 1) ||
           (ptcl->child(1) && ptcl->child(1)->pdgId() == 22 && ptcl->child(1)->status() != 1)))
      { return true; }

    return false;
  }



}
