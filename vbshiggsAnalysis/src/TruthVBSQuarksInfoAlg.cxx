/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Antonio Giannini

//
// includes
//
#include "TruthVBSQuarksInfoAlg.h"

#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

//
// method implementations
//
namespace VBSHIGGS
{
  TruthVBSQuarksInfoAlg ::TruthVBSQuarksInfoAlg(
    const std::string &name, ISvcLocator *pSvcLocator)
    : AthAlgorithm(name, pSvcLocator) { }

  StatusCode TruthVBSQuarksInfoAlg ::initialize()
  {
    ATH_MSG_DEBUG("Initialising " << name());

    if (!m_truthParticleInKey.empty())
      ATH_CHECK(m_truthParticleInKey.initialize());

    ATH_CHECK(m_EventInfoKey.initialize());
    ATH_CHECK(m_nVBSQuarksKey.initialize());

    if (!m_VBSQuarksKey.empty())
      ATH_CHECK(m_VBSQuarksKey.initialize());

    ATH_CHECK(m_VBSQuark1pTKey.initialize());
    ATH_CHECK(m_VBSQuark1etaKey.initialize());
    ATH_CHECK(m_VBSQuark1phiKey.initialize());
    ATH_CHECK(m_VBSQuark1EKey.initialize());

    ATH_CHECK(m_VBSQuark2pTKey.initialize());
    ATH_CHECK(m_VBSQuark2etaKey.initialize());
    ATH_CHECK(m_VBSQuark2phiKey.initialize());
    ATH_CHECK(m_VBSQuark2EKey.initialize());

    return StatusCode::SUCCESS;

  }


  StatusCode TruthVBSQuarksInfoAlg ::execute(){

    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    ATH_CHECK(eventInfo.isValid());

    SG::ReadHandle<xAOD::TruthParticleContainer> truthParticleContainer(m_truthParticleInKey);
    ATH_CHECK(truthParticleContainer.isValid());

    int nVBSQuarks (0); 
    const xAOD::TruthParticle *quark1 = nullptr;
    const xAOD::TruthParticle *quark2 = nullptr;
    
    // loop over particles
    for (const xAOD::TruthParticle *tp : (*truthParticleContainer)){

        // only quarks
        if(std::abs(tp->pdgId()) > 5) continue;
        if(tp->status()!= 23) continue;

        const xAOD::TruthParticle *parent = tp->parent(0);
        if( !(parent->status()==21 && std::abs(parent->pdgId())<6) ) continue;

        nVBSQuarks += 1;

        if(!quark1) quark1 = tp;
        else if(!quark2) quark2 = tp;
    }

    // sort by pT
    if(quark1 && quark2 && quark1->pt()<quark2->pt()) std::swap(quark1, quark2);
    
    // Update the user-defined variable in EventInfo
    SG::WriteDecorHandle<xAOD::EventInfo, unsigned int> nVBSQuarksDecorHandle(m_nVBSQuarksKey);
    nVBSQuarksDecorHandle(*eventInfo) = nVBSQuarks;

    SG::WriteDecorHandle<xAOD::EventInfo, float> VBSQuark1pTDecorHandle(m_VBSQuark1pTKey);
    SG::WriteDecorHandle<xAOD::EventInfo, float> VBSQuark1etaDecorHandle(m_VBSQuark1etaKey);
    SG::WriteDecorHandle<xAOD::EventInfo, float> VBSQuark1phiDecorHandle(m_VBSQuark1phiKey);
    SG::WriteDecorHandle<xAOD::EventInfo, float> VBSQuark1EDecorHandle(m_VBSQuark1EKey);
    SG::WriteDecorHandle<xAOD::EventInfo, float> VBSQuark2pTDecorHandle(m_VBSQuark2pTKey);
    SG::WriteDecorHandle<xAOD::EventInfo, float> VBSQuark2etaDecorHandle(m_VBSQuark2etaKey);
    SG::WriteDecorHandle<xAOD::EventInfo, float> VBSQuark2phiDecorHandle(m_VBSQuark2phiKey);
    SG::WriteDecorHandle<xAOD::EventInfo, float> VBSQuark2EDecorHandle(m_VBSQuark2EKey);

    VBSQuark1pTDecorHandle(*eventInfo) = quark1 ? quark1 -> pt() : -99.;
    VBSQuark1etaDecorHandle(*eventInfo) = quark1 ? quark1 -> eta() : -99.;
    VBSQuark1phiDecorHandle(*eventInfo) = quark1 ? quark1 -> phi() : -99.;
    VBSQuark1EDecorHandle(*eventInfo) = quark1 ? quark1 -> e() : -99.;

    VBSQuark2pTDecorHandle(*eventInfo) = quark2 ? quark2 -> pt() : -99.;
    VBSQuark2etaDecorHandle(*eventInfo) = quark2 ? quark2 -> eta() : -99.;
    VBSQuark2phiDecorHandle(*eventInfo) = quark2 ? quark2 -> phi() : -99.;
    VBSQuark2EDecorHandle(*eventInfo) = quark2 ? quark2 -> e() : -99.;

    // store the two quarks in the output container
    auto VBSQuarks = std::make_unique<ConstDataVector<xAOD::TruthParticleContainer>>(SG::VIEW_ELEMENTS);

    if(quark1) VBSQuarks -> push_back(quark1);
    if(quark2) VBSQuarks -> push_back(quark2);

    SG::WriteHandle<ConstDataVector<xAOD::TruthParticleContainer>> writeHandle(m_VBSQuarksKey);
    ATH_CHECK(writeHandle.record(std::move(VBSQuarks)));

    return StatusCode::SUCCESS;

  }

}
