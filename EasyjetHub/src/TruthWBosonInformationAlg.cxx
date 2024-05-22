/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Jordy Degens, Osama Karkout

//
// includes
//
#include "TruthWBosonInformationAlg.h"
#include "TruthUtils.h"

#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

#include "TruthUtils/HepMCHelpers.h"

//
// method implementations
//
namespace Easyjet
{
  TruthWBosonInformationAlg ::TruthWBosonInformationAlg(
    const std::string &name, ISvcLocator *pSvcLocator)
    : AthAlgorithm(name, pSvcLocator) { }

  StatusCode TruthWBosonInformationAlg ::initialize()
  {
    ATH_MSG_DEBUG("Initialising " << name());

    if (!m_truthBosonParticleInKey.empty())
      ATH_CHECK(m_truthBosonParticleInKey.initialize());

    ATH_CHECK(m_EventInfoKey.initialize());
    ATH_CHECK(m_nWLepKey.initialize());

    return StatusCode::SUCCESS;

  }


  StatusCode TruthWBosonInformationAlg ::execute(){

    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    ATH_CHECK(eventInfo.isValid());

    SG::ReadHandle<xAOD::TruthParticleContainer> truthBosonParticleContainer(
        m_truthBosonParticleInKey);
    ATH_CHECK(truthBosonParticleContainer.isValid());

    int nWLep = 0; //number of leptonic W decays in event
    //W decay chain
    for (const xAOD::TruthParticle *tp : (*truthBosonParticleContainer)){
      if (std::abs(tp->pdgId()) == MC::WPLUSBOSON){
        const xAOD::TruthParticle* final_W =
        getFinalParticleOfType(tp, {tp->pdgId()});
        
        if (tp->barcode() == final_W->barcode()) {
          // the if statement makes sure that the W is the final W (avoiding repitition structure of container)
          bool isWLep = false;
          for (size_t i = 0; i < final_W->nChildren(); i++) {
            const xAOD::TruthParticle* W_child = final_W->child(i);
            if (std::abs(W_child->pdgId()) == 12 || std::abs(W_child->pdgId()) == 14 || std::abs(W_child->pdgId()) == 16){
              isWLep = true;
            }
            if(isWLep) break;
          }
          if(isWLep) nWLep++;
        }
      }
    }
    // Update the user-defined variable in EventInfo
    SG::WriteDecorHandle<xAOD::EventInfo, unsigned int> nWLepDecorHandle(m_nWLepKey);
    nWLepDecorHandle(*eventInfo) = nWLep;

    return StatusCode::SUCCESS;

  }




}
