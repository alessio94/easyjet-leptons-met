/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "TruthLeptonOrderingAlg.h"
#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteHandle.h>
#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

namespace Easyjet
{
  TruthLeptonOrderingAlg::TruthLeptonOrderingAlg(const std::string &name,
                                       ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator) { }

  StatusCode TruthLeptonOrderingAlg::initialize()
  {
    // Read input/output handle keys
    ATH_CHECK (m_inTruthElectronHandleKey.initialize());
    ATH_CHECK (m_inTruthMuonHandleKey.initialize());

    // Intialise input/output decorators
    ATH_CHECK (m_isSelectedElectronKey.initialize());
    ATH_CHECK (m_isSelectedMuonKey.initialize());

    for (int i = 0; i < m_leptonAmount; i++){
      std::string index = std::to_string(i + 1);
      std::string elekey = m_inTruthElectronHandleKey.key() + ".isTruthLepton" + index;
      std::string mukey = m_inTruthMuonHandleKey.key() + ".isTruthLepton" + index;
      SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> EleDecorKey(elekey);
      SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> MuDecorKey(mukey);
      m_leadBranchesEleKey.emplace("isTruthLepton" + index, std::move(EleDecorKey));
      ATH_CHECK(m_leadBranchesEleKey.at("isTruthLepton" + index).initialize());
      m_leadBranchesMuKey.emplace("isTruthLepton" + index, std::move(MuDecorKey));
      ATH_CHECK(m_leadBranchesMuKey.at("isTruthLepton" + index).initialize());
    }

    return StatusCode::SUCCESS;
  }

  StatusCode TruthLeptonOrderingAlg::execute()
  {
    // Retrieve inputs
    SG::ReadHandle<xAOD::TruthParticleContainer> inTruthElectronContainer(m_inTruthElectronHandleKey);
    SG::ReadHandle<xAOD::TruthParticleContainer> inTruthMuonContainer(m_inTruthMuonHandleKey);
    SG::ReadDecorHandle<xAOD::TruthParticleContainer, bool> m_isSelectedElectron(m_isSelectedElectronKey);
    SG::ReadDecorHandle<xAOD::TruthParticleContainer, bool> m_isSelectedMuon(m_isSelectedMuonKey);  

    std::vector<std::pair<const xAOD::TruthParticle*, int>> leptons;

      // electron
      for (const xAOD::TruthParticle *electron : *inTruthElectronContainer){
        if (m_isSelectedElectron(*electron)){
         leptons.emplace_back(electron, -11*electron->charge());
	}
      }

      // muon
      for (const xAOD::TruthParticle *muon : *inTruthMuonContainer){
        if (m_isSelectedMuon(*muon)){
         leptons.emplace_back(muon, -13*muon->charge());
	}
      }


    // sort by pt
    std::sort(leptons.begin(), leptons.end(),
    [](const std::pair<const xAOD::TruthParticle*, int>& a,
        const std::pair<const xAOD::TruthParticle*, int>& b) {
        return a.first->pt() > b.first->pt(); });

    // decorate ele/mu
    if(m_leptonAmount > 0){
    int nLepton = 0;
    for (auto lepton : leptons) {
        nLepton++;
        std::string key = "isTruthLepton"+std::to_string(nLepton);
        if (std::abs(lepton.second) == 11){ //ele
            SG::WriteDecorHandle<xAOD::TruthParticleContainer, bool> eledecor(m_leadBranchesEleKey.at(key));
            eledecor(*lepton.first) = true;
    } else if (std::abs(lepton.second) == 13) { //mu
            SG::WriteDecorHandle<xAOD::TruthParticleContainer, bool> mudecor(m_leadBranchesMuKey.at(key));
            mudecor(*lepton.first) = true;
    }
        if ( nLepton == m_leptonAmount ) break;
    }
    }

  
    return StatusCode::SUCCESS;
 }
}