/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#include "AthContainers/AuxElement.h"
#include "BaselineVarsbbllAlg.h"
#include <FourMomUtils/xAODP4Helpers.h>
#include <AthContainers/ConstDataVector.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>

namespace HHBBLL
{
  BaselineVarsbbllAlg::BaselineVarsbbllAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {

  }

  StatusCode BaselineVarsbbllAlg::initialize()
  {

    // Read syst-aware input handles
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    // Intialise syst-aware output decorators
    for (const std::string &string_var: m_Fvarnames) {
      CP::SysWriteDecorHandle<float> var {string_var+"_%SYS%", this};
      m_Fbranches.emplace(string_var, var);
      ATH_CHECK (m_Fbranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }


    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());    

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsbbllAlg::execute()
  {

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {

      // Retrive inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      const xAOD::MissingETContainer *metCont = nullptr;
      ANA_CHECK (m_metHandle.retrieve (metCont, sys));
      const xAOD::MissingET* met = (*metCont)["Final"];
      if (!met) {
	ATH_MSG_ERROR("Could not retrieve MET");
	return StatusCode::FAILURE;	
      }

      // Calculate vars

      for (const std::string &string_var: m_Fvarnames) {
        m_Fbranches.at(string_var).set(*event, -99., sys);
      }
      
      //Lepton vars
      if (muons->size() > 0) {
	      m_Fbranches.at("Leading_Muon_pt").set(*event, muons->at(0)->pt(), sys);
	      m_Fbranches.at("Leading_Muon_eta").set(*event, muons->at(0)->eta(), sys);
      }

      if (electrons->size() > 0) {
	      m_Fbranches.at("Leading_Electron_pt").set(*event, electrons->at(0)->pt(), sys);
	      m_Fbranches.at("Leading_Electron_eta").set(*event, electrons->at(0)->eta(), sys);
      }

      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      for(const xAOD::Jet* jet : *jets) {
        if (WPgiven) {
          if (m_isBtag.get(*jet, sys)) bjets->push_back(jet);
        }
      }
     

    }

    return StatusCode::SUCCESS;
  }
}
