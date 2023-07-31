/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


#include "AthContainers/AuxElement.h"
#include "BaselineVarsbbttAlg.h"
#include <FourMomUtils/xAODP4Helpers.h>
#include <AthContainers/ConstDataVector.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODTau/TauJetContainer.h>

namespace HH4B
{
  BaselineVarsbbttAlg::BaselineVarsbbttAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
    declareProperty("bTagWP", m_bTagWP);
    if (!m_bTagWP.empty()) m_bTagWP = "_" + m_bTagWP;
  }

  StatusCode BaselineVarsbbttAlg::initialize()
  {

    // Read syst-aware input handles
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_tauHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    // Intialise syst-aware output decorators
    ATH_CHECK(m_leading_muon_pt.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_leading_muon_eta.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK(m_leading_elec_pt.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_leading_elec_eta.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK(m_leading_tau_pt.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_leading_tau_eta.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());    

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsbbttAlg::execute()
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

      const xAOD::TauJetContainer *taus = nullptr;
      ANA_CHECK (m_tauHandle.retrieve (taus, sys));

      const xAOD::MissingETContainer *metCont = nullptr;
      ANA_CHECK (m_metHandle.retrieve (metCont, sys));
      const xAOD::MissingET* met = (*metCont)["Final"];
      if (!met) {
	ATH_MSG_ERROR("Could not retrieve MET");
	return StatusCode::FAILURE;	
      }

      // Calculate vars

      if (muons->size() > 0) {
	m_leading_muon_pt.set(*event, muons->at(0)->pt(), sys);
	m_leading_muon_eta.set(*event, muons->at(0)->eta(), sys);
      } else {
	m_leading_muon_pt.set(*event, -99, sys);
	m_leading_muon_eta.set(*event, -99, sys);
      }

      if (electrons->size() > 0) {
	m_leading_elec_pt.set(*event, electrons->at(0)->pt(), sys);
	m_leading_elec_eta.set(*event, electrons->at(0)->eta(), sys);
      } else {
	m_leading_elec_pt.set(*event, -99, sys);
	m_leading_elec_eta.set(*event, -99, sys);
      }

      if (taus->size() > 0) {
	m_leading_tau_pt.set(*event, taus->at(0)->pt(), sys);
	m_leading_tau_eta.set(*event, taus->at(0)->eta(), sys);
      } else {
 	m_leading_tau_pt.set(*event, -99, sys);
	m_leading_tau_eta.set(*event, -99, sys);
      }
    }

    return StatusCode::SUCCESS;
  }
}
