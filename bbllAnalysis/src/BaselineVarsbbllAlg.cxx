/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "BaselineVarsbbllAlg.h"
#include "AthContainers/AuxElement.h"
#include <AthContainers/ConstDataVector.h>

#include "TLorentzVector.h"

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
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    // Intialise syst-aware output decorators
    for (const std::string &var : m_floatVariables) {
      CP::SysWriteDecorHandle<float> whandle{var+"_%SYS%", this};
      m_Fbranches.emplace(var, whandle);
      ATH_CHECK (m_Fbranches.at(var).initialize(m_systematicsList, m_eventHandle));
    }

    for (const std::string &var : m_intVariables){
      ATH_MSG_DEBUG("initializing integer variable: " << var);
      CP::SysWriteDecorHandle<int> whandle{var+"_%SYS%", this};
      m_Ibranches.emplace(var, whandle);
      ATH_CHECK(m_Ibranches.at(var).initialize(m_systematicsList, m_eventHandle));
    };

    ATH_CHECK (m_met_sig.initialize(m_systematicsList, m_metHandle));

    ATH_CHECK (m_Lepton1_pt.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_Lepton1_eta.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_Lepton1_phi.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_Lepton1_E.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK (m_Lepton2_pt.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_Lepton2_eta.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_Lepton2_phi.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_Lepton2_E.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK (m_nLeptons.initialize(m_systematicsList, m_eventHandle));
    //
    ATH_CHECK (m_Jet_b1_pt.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_Jet_b1_eta.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_Jet_b1_phi.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_Jet_b1_E.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK (m_Jet_b2_pt.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_Jet_b2_eta.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_Jet_b2_phi.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_Jet_b2_E.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK (m_bjets.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsbbllAlg::execute()
  {

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {

      // Retrieve inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::MissingETContainer *metCont = nullptr;
      ANA_CHECK (m_metHandle.retrieve (metCont, sys));
      const xAOD::MissingET* met = (*metCont)["Final"];
      if (!met) {
	ATH_MSG_ERROR("Could not retrieve MET");
       	return StatusCode::FAILURE;	
      }

      for (const std::string &string_var: m_floatVariables) {
        m_Fbranches.at(string_var).set(*event, -99., sys);
      }

      for (const auto& var: m_intVariables) {
        m_Ibranches.at(var).set(*event, -99, sys);
      }

      TLorentzVector Leading_lep(0.,0.,0.,0.);
      TLorentzVector Subleading_lep(0.,0.,0.,0.);
      TLorentzVector Leading_bjet(0.,0.,0.,0.);
      TLorentzVector Subleading_bjet(0.,0.,0.,0.);
      TLorentzVector met_vector;
      TLorentzVector bb;
      TLorentzVector ll;
      TLorentzVector bbll(0.,0.,0.,0.);
      TLorentzVector bbllmet(0.,0.,0.,0.);
      TLorentzVector b1l1;
      TLorentzVector b1l2;
      TLorentzVector b2l1;
      TLorentzVector b2l2;

      int n_leptons=m_nLeptons.get(*event, sys);
      int n_bjets=m_bjets.get(*event, sys);
      // met
      met_vector.SetPtEtaPhiE(met->met(), 0, met->phi(), met->met());
      float met_sig = m_met_sig.get(*met, sys);
      m_Fbranches.at("MET_sig").set(*event, met_sig, sys);
      Leading_lep.SetPtEtaPhiE(m_Lepton1_pt.get(*event, sys), 
			       m_Lepton1_eta.get(*event, sys), 
			       m_Lepton1_phi.get(*event, sys), 
			       m_Lepton1_E.get(*event, sys));

      Subleading_lep.SetPtEtaPhiE(m_Lepton2_pt.get(*event, sys), 
				  m_Lepton2_eta.get(*event, sys), 
				  m_Lepton2_phi.get(*event, sys), 
				  m_Lepton2_E.get(*event, sys));
      
      Leading_bjet.SetPtEtaPhiE(m_Jet_b1_pt.get(*event, sys), 
				m_Jet_b1_eta.get(*event, sys), 
				m_Jet_b1_phi.get(*event, sys), 
				m_Jet_b1_E.get(*event, sys));

      Subleading_bjet.SetPtEtaPhiE(m_Jet_b2_pt.get(*event, sys), 
				m_Jet_b2_eta.get(*event, sys), 
				m_Jet_b2_phi.get(*event, sys), 
				m_Jet_b2_E.get(*event, sys));

      if(n_leptons>=2) ll = Leading_lep + Subleading_lep;
      if (n_bjets>=2) bb = Leading_bjet + Subleading_bjet;
      // b-jet + lepton sector
      if (n_bjets>=2 && n_leptons>=2) {
	b1l1 = Leading_bjet+Leading_lep;
	b2l1 = Subleading_bjet+Leading_lep;
	b1l2 = Leading_bjet+Subleading_lep;
	b2l2 = Subleading_bjet+Subleading_lep;
	double m_b1l1 = b1l1.M();
	double m_b2l1 = b2l1.M();
	double m_b1l2 = b1l2.M();
	double m_b2l2 = b2l2.M();
	double m_bl = std::min(std::max(m_b1l1, m_b2l2), std::max(m_b2l1, m_b1l2));
        m_Fbranches.at("mbl").set(*event, m_bl, sys);
	bbll = ll + bb;
        bbllmet = bbll + met_vector;
        m_Fbranches.at("mbbll").set(*event, bbll.M(), sys);
        m_Fbranches.at("mbbllmet").set(*event, bbllmet.M(), sys);
	if(m_save_extra_vars) {
	  // Ht2r mesure for boostedness of the two Higgs bosons
	  double ht2 = (met_vector + ll).Perp() + bb.Perp();
	  double ht2r = ht2 / (met->met() + Leading_lep.Pt() + Subleading_lep.Pt() + Leading_bjet.Pt() + Subleading_bjet.Pt());
	  m_Fbranches.at("HT2").set(*event, ht2, sys);
	  m_Fbranches.at("HT2r").set(*event, ht2r, sys);
	}
      }
      if(m_save_extra_vars) {
	// Transverse mass of the pT-leading lepton wrt met
	if (n_leptons >= 1)
	  {
	    float mt_lept1_met = TMath::Sqrt(2 * met->met() * Leading_lep.Pt() * (1 - TMath::Cos(Leading_lep.DeltaPhi(met_vector))));
	    m_Fbranches.at("mT_Lepton1_Met").set(*event, mt_lept1_met, sys);
	    if (n_leptons >= 2)
	      {
		float mt_lept2_met = TMath::Sqrt(2 * met->met() * Subleading_lep.Pt() * (1 - TMath::Cos(Subleading_lep.DeltaPhi(met_vector))));
		m_Fbranches.at("mT_Lepton2_Met").set(*event, mt_lept2_met, sys);
		float mt_l_min = std::min(mt_lept1_met, mt_lept2_met);
		m_Fbranches.at("mT_L_min").set(*event, mt_l_min, sys);
	      }
	  }
      }
    }
    return StatusCode::SUCCESS;
  }

}
