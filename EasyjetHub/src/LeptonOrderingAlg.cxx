/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Minori Fujimoto

#include "LeptonOrderingAlg.h"

namespace Easyjet
{
  LeptonOrderingAlg::LeptonOrderingAlg(const std::string &name,
                                       ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator) { }

  StatusCode LeptonOrderingAlg::initialize()
  {
    // Read syst-aware input/output handles
    ATH_CHECK (m_inEleHandle.initialize(m_systematicsList));
    ATH_CHECK (m_inMuHandle.initialize(m_systematicsList));

    // Intialise syst-aware input/output decorators    
    ATH_CHECK (m_isSelectedElectron.initialize(m_systematicsList, m_inEleHandle));
    ATH_CHECK (m_isSelectedMuon.initialize(m_systematicsList, m_inMuHandle));

    std::vector<std::string> leadBranchesEle_keys{}, leadBranchesMu_keys{};
    for (int i = 0; i < m_leptonAmount; i++){
      leadBranchesEle_keys.emplace_back("isLepton" + std::to_string(i + 1) + "_%SYS%");
      leadBranchesMu_keys.emplace_back("isLepton" + std::to_string(i + 1) + "_%SYS%");
    }
    m_leadBranchesEle = CP::SysWriteDecorHandleArray<bool>(leadBranchesEle_keys,this);
    m_leadBranchesMu = CP::SysWriteDecorHandleArray<bool>(leadBranchesMu_keys,this);
    ATH_CHECK( m_leadBranchesEle.initialize(m_systematicsList, m_inEleHandle) );
    ATH_CHECK( m_leadBranchesMu.initialize(m_systematicsList, m_inMuHandle) );

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());    

    return StatusCode::SUCCESS;
  }

  StatusCode LeptonOrderingAlg::execute()
  {
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector()) {

      // Retrive inputs
      const xAOD::ElectronContainer *inEleContainer = nullptr;
      ANA_CHECK (m_inEleHandle.retrieve (inEleContainer, sys));      
      const xAOD::MuonContainer *inMuContainer = nullptr;
      ANA_CHECK (m_inMuHandle.retrieve (inMuContainer, sys));      

      std::vector<std::pair<const xAOD::IParticle*, int>> leptons;

      // electron
      for (const xAOD::Electron *electron : *inEleContainer){
        if (m_isSelectedElectron.get(*electron, sys)){
         leptons.emplace_back(electron, -11*electron->charge());
	}
      }

      // muon
      for (const xAOD::Muon *muon : *inMuContainer){
        if (m_isSelectedMuon.get(*muon, sys)){
         leptons.emplace_back(muon, -13*muon->charge());
	}
      }

      // sort by pt
      std::sort(leptons.begin(), leptons.end(),
		[](const std::pair<const xAOD::IParticle*, int>& a,
		   const std::pair<const xAOD::IParticle*, int>& b) {
		  return a.first->pt() > b.first->pt(); });

      // decorate ele/mu
      if(m_leptonAmount > 0){
        for (int nLepton = 0;auto lepton : leptons) {
           if (std::abs(lepton.second) == 11){ //ele
              m_leadBranchesEle.at(nLepton).set(*lepton.first, true, sys);
	         } else if (std::abs(lepton.second) == 13) { //mu
              m_leadBranchesMu.at(nLepton).set(*lepton.first, true, sys);
	         } 
           nLepton++;
           if ( nLepton == m_leptonAmount ) break;
        }
      }

  } 
    return StatusCode::SUCCESS;
 }
}

