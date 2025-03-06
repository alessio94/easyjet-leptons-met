/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "BJetsVarsbbllAlg.h"
#include "AthContainers/AuxElement.h"
#include <AthContainers/ConstDataVector.h>

#include "TLorentzVector.h"
#include "EasyjetHub/MT2_ROOT.h"



namespace HHBBLL
{
  BJetsVarsbbllAlg::BJetsVarsbbllAlg(const std::string &name,
                                       ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {}

  StatusCode BJetsVarsbbllAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("       BJetsVarsbbllAlg       \n");
    ATH_MSG_INFO("*********************************\n");

    ATH_CHECK (m_bbllJetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

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

    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_bbllJetHandle));
    }
    for (const std::string &var : m_PCBTnames) {
      ATH_MSG_DEBUG("initializing PCBT: " << var);
      CP::SysReadDecorHandle<int> rhandle{var, this};
      m_PCBTs.emplace(var, rhandle);
      ATH_CHECK (m_PCBTs.at(var).initialize(m_systematicsList, m_bbllJetHandle));
    };
    if (m_isMC) ATH_CHECK (m_truthFlav.initialize(m_systematicsList, m_bbllJetHandle));
    ATH_CHECK (m_nmuons.initialize(m_systematicsList, m_bbllJetHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode BJetsVarsbbllAlg::execute()
  {
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      // Retrieve inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_bbllJetHandle.retrieve (jets, sys));

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
      int n_jets=0;
      int n_bjets=0;
      int nCentralJets = 0;
      TLorentzVector Leading_bjet;
      TLorentzVector Subleading_bjet;
      TLorentzVector bb;
      TLorentzVector met_vector;
      //
      n_jets = jets->size();
      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      for(const xAOD::Jet* jet : *jets) {
	// count central jets
        if (std::abs(jet->eta())<2.5) {
          nCentralJets++;
	  if (WPgiven && m_isBtag.get(*jet, sys)) bjets->push_back(jet);
	}
      }
      n_bjets = bjets->size();
      for(unsigned int i=0; i<std::min(size_t(2),bjets->size()); i++){
        std::string prefix = "Jet_b"+std::to_string(i+1);
        if(i==0) Leading_bjet = bjets->at(i)->p4();
        else if(i==1) Subleading_bjet = bjets->at(i)->p4();
        m_Fbranches.at(prefix+"_pt").set(*event, bjets->at(i)->pt(), sys);
        m_Fbranches.at(prefix+"_eta").set(*event, bjets->at(i)->eta(), sys);
        m_Fbranches.at(prefix+"_phi").set(*event, bjets->at(i)->phi(), sys);
        m_Fbranches.at(prefix+"_E").set(*event, bjets->at(i)->e(), sys);
	if (m_isMC) {
          int truthLabel = m_truthFlav.get(*bjets->at(i), sys);
          m_Ibranches.at(prefix+"_truthLabel").set(*event, truthLabel, sys);
        }
        for (const auto& var: m_PCBTnames) {
          std::string new_var = var;
          new_var.erase(0, 14);
          new_var.erase(new_var.length() - 11, new_var.length());
          m_Ibranches.at(prefix+"_pcbt_"+new_var).set(*event, m_PCBTs.at(var).get(*bjets->at(i), sys), sys);
        }
	m_Ibranches.at(prefix+"_nmuons").set
	  (*event, m_nmuons.get(*bjets->at(i), sys), sys);
      }
      if (bjets->size()>=2) {
        bb = Leading_bjet + Subleading_bjet;
        m_Fbranches.at("mbb").set(*event, bb.M(), sys);
        m_Fbranches.at("pTbb").set(*event, bb.Pt(), sys);
        m_Fbranches.at("Phibb").set(*event, bb.Phi(), sys);
        m_Fbranches.at("dRbb").set(*event, Leading_bjet.DeltaR(Subleading_bjet), sys);
      }
      met_vector.SetPtEtaPhiE(met->met(), 0, met->phi(), met->met());
      ComputeMT2 mt2_calculator = ComputeMT2(Leading_bjet, Subleading_bjet, met_vector, 0, 0);
      double mT2_bb = mt2_calculator.Compute();
      m_Fbranches.at("mT2_bb").set(*event, mT2_bb, sys);
      //
      if(m_save_extra_vars) {
	m_Ibranches.at("nJets").set(*event, n_jets, sys);
	m_Ibranches.at("nCentralJets").set(*event, nCentralJets, sys);
      }
      m_Ibranches.at("nBJets").set(*event, n_bjets, sys);
    }
    return StatusCode::SUCCESS;
  }
}
