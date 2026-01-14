/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

*/

#include "ZcandSelectorAlg.h"

namespace XBBCALIB{
  ZcandSelectorAlg :: ZcandSelectorAlg(const std::string& name, ISvcLocator *pSvcLocator)
    : EL::AnaAlgorithm(name, pSvcLocator){}

  StatusCode ZcandSelectorAlg::initialize(){

    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("      ZcandSelectorAlg           \n");
    ATH_MSG_INFO("*********************************\n");

    // Read syst-aware input handles
    ATH_CHECK (m_lrjetHandle.initialize(m_systematicsList));

    ATH_CHECK (m_photonHandle.initialize(m_systematicsList));

    // Intialise syst-aware output decorators
    ATH_CHECK (m_ZcandjetOutHandle.initialize(m_systematicsList));


    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ANA_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode ZcandSelectorAlg::execute(){
    /*
       Z candidate selection algorithm
       1) First select large-R jets pt>200GeV, m>40GeV (should be already done by the jet selector)
       2) require the photon to be Photon pT > 175 GeV (maybe lower to 150 for now) (exactly one photon should already be req.)
       3) only consider large-r jets with dR_ljets_photon > 1
       4) At leat one Zcand, pick the leading Z cand only.
    */

    for (const auto& sys : m_systematicsList.systematicsVector()){
      // Retrieve inputs
      const xAOD::JetContainer *ljets = nullptr;
      ANA_CHECK (m_lrjetHandle.retrieve (ljets, sys));

      const xAOD::PhotonContainer *photons = nullptr;
      ANA_CHECK (m_photonHandle.retrieve (photons, sys));

      auto ZcandsJetCandidates= std::make_unique<ConstDataVector<xAOD::JetContainer> >(SG::VIEW_ELEMENTS);

      if (!photons->empty()) {
        const xAOD::Photon* photon = photons->at(0); // leading photon
        for(unsigned int i=0; i<ljets->size(); i++){
          const xAOD::Jet* ljet = ljets->at(i);
          if ( photon->pt()<150.e+3 || ljet->p4().DeltaR(photon->p4())<1.) continue;
          ZcandsJetCandidates->push_back(ljet);
        }
      }
      ATH_CHECK(m_ZcandjetOutHandle.record(std::move(ZcandsJetCandidates), sys));
    }//sys

    return StatusCode::SUCCESS;
  }
}
