/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "AthContainers/AuxElement.h"
#include "BaselineVarsbbllAlg.h"
#include <FourMomUtils/xAODP4Helpers.h>
#include <AthContainers/ConstDataVector.h>
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>

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

    for (const std::string &var : m_Ivarnames){
      ATH_MSG_DEBUG("initializing integer variable: " << var);
      CP::SysWriteDecorHandle<int> whandle{var+"_%SYS%", this};
      m_Ibranches.emplace(var, whandle);
      ATH_CHECK(m_Ibranches.at(var).initialize(m_systematicsList, m_eventHandle));
    };

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

      // Retrieve inputs
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
      for (const std::string &string_var: m_Fvarnames) {
        m_Fbranches.at(string_var).set(*event, -99., sys);
      }

      for (const auto& var: m_Ivarnames) {
        m_Ibranches.at(var).set(*event, -99, sys);
      }

      TLorentzVector bb;
      TLorentzVector ee;
      TLorentzVector mumu;

      bool TWO_ISO_MUONS = false;
      bool TWO_ISO_ELECTRONS = false;
      bool pass_ee = false;
      bool pass_mumu = false;
      bool EXACTLY_TWO_B_JETS = 0;
      int n_jets=0;
      int n_bjets=0;
      int n_electrons=0;
      int n_muons=0;
      std::vector<float> PassElectronIsos;
      std::vector<float> PassMuonIsos;
      double Mee = -99;
      double Mmumu = -99;

      // Count electrons
      n_electrons = electrons->size();

      // Count muons
      n_muons = muons->size();

      // Count jets
      n_jets = jets->size();

      if(electrons->size() >= 2){
        Mee = (electrons->at(0)->p4() + electrons->at(1)->p4()).M();
        TWO_ISO_ELECTRONS = true;
      }
      if(muons->size() >= 2){
        Mmumu = (muons->at(0)->p4() + muons->at(1)->p4()).M();
        TWO_ISO_MUONS = true;
      }

      // b-jet sector
      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      for(const xAOD::Jet* jet : *jets) {
        if (WPgiven) {
          if (m_isBtag.get(*jet, sys)) bjets->push_back(jet);
        }
      }
      n_bjets = bjets->size();       

      EXACTLY_TWO_B_JETS = (n_bjets==2);

      pass_ee = TWO_ISO_ELECTRONS && EXACTLY_TWO_B_JETS;          
      pass_mumu = TWO_ISO_MUONS && EXACTLY_TWO_B_JETS;

      m_Fbranches.at("TWO_ISO_ELECTRONS").set(*event, TWO_ISO_ELECTRONS, sys);
      m_Fbranches.at("TWO_ISO_MUONS").set(*event, TWO_ISO_MUONS, sys);
      m_Fbranches.at("EXACTLY_TWO_B_JETS").set(*event, EXACTLY_TWO_B_JETS, sys);
      m_Fbranches.at("Pass_ee").set(*event, pass_ee, sys);
      m_Fbranches.at("Pass_mumu").set(*event, pass_mumu, sys);
      m_Fbranches.at("Mee").set(*event, Mee, sys);
      m_Fbranches.at("Mmumu").set(*event, Mmumu, sys);

      m_Ibranches.at("nJets").set(*event, n_jets, sys);
      m_Ibranches.at("nElectrons").set(*event, n_electrons, sys);
      m_Ibranches.at("nMuons").set(*event, n_muons, sys);
      m_Ibranches.at("nBJets").set(*event, n_bjets, sys);

      // Electron sector
      if (electrons->size() > 0)
      {
        // Leading electron
        m_Fbranches.at("Leading_Electron_pt").set(*event, electrons->at(0)->pt(), sys);
        m_Fbranches.at("Leading_Electron_eta").set(*event, electrons->at(0)->eta(), sys);
        m_Fbranches.at("Leading_Electron_phi").set(*event, electrons->at(0)->phi(), sys);
        m_Fbranches.at("Leading_Electron_E").set(*event, electrons->at(0)->e(), sys);
      }
      if (electrons->size() >= 2)
      {
        // Subleading electron
        m_Fbranches.at("Subleading_Electron_pt").set(*event, electrons->at(1)->pt(), sys);
        m_Fbranches.at("Subleading_Electron_eta").set(*event, electrons->at(1)->eta(), sys);
        m_Fbranches.at("Subleading_Electron_phi").set(*event, electrons->at(1)->phi(), sys);
        m_Fbranches.at("Subleading_Electron_E").set(*event, electrons->at(1)->e(), sys);

        // ee
        ee = electrons->at(0)->p4() + electrons->at(1)->p4();
        m_Fbranches.at("mee").set(*event, ee.M(), sys);
        m_Fbranches.at("pTee").set(*event, ee.Pt(), sys);
        m_Fbranches.at("Etaee").set(*event, ee.Eta(), sys);
        m_Fbranches.at("Phiee").set(*event, ee.Phi(), sys);
        m_Fbranches.at("dRee").set(*event, (electrons->at(0)->p4()).DeltaR(electrons->at(1)->p4()), sys);

      } //end electron sector

      // Muon sector
      if (muons->size() >= 1)
      {
        // Leading muon
        m_Fbranches.at("Leading_Muon_pt").set(*event, muons->at(0)->pt(), sys);
        m_Fbranches.at("Leading_Muon_eta").set(*event, muons->at(0)->eta(), sys);
        m_Fbranches.at("Leading_Muon_phi").set(*event, muons->at(0)->phi(), sys);
        m_Fbranches.at("Leading_Muon_E").set(*event, muons->at(0)->e(), sys);
      }
      if (muons->size() >= 2) 
      { 
        // Subleading muon
        m_Fbranches.at("Subleading_Muon_pt").set(*event, muons->at(1)->pt(), sys);
        m_Fbranches.at("Subleading_Muon_eta").set(*event, muons->at(1)->eta(), sys);
        m_Fbranches.at("Subleading_Muon_phi").set(*event, muons->at(1)->phi(), sys);
        m_Fbranches.at("Subleading_Muon_E").set(*event, muons->at(1)->e(), sys);

        // mumu
        mumu = muons->at(0)->p4() + muons->at(1)->p4();
        m_Fbranches.at("mmumu").set(*event, mumu.M(), sys);
        m_Fbranches.at("pTmumu").set(*event, mumu.Pt(), sys);
        m_Fbranches.at("Etamumu").set(*event, mumu.Eta(), sys);
        m_Fbranches.at("Phimumu").set(*event, mumu.Phi(), sys);
        m_Fbranches.at("dRmumu").set(*event, (muons->at(0)->p4()).DeltaR(muons->at(1)->p4()), sys);
      }// end muon 

      if (jets->size()>=1)
      {
        m_Fbranches.at("Leading_Jet_pt").set(*event, jets->at(0)->pt(), sys);
        m_Fbranches.at("Leading_Jet_eta").set(*event, jets->at(0)->eta(), sys);
        m_Fbranches.at("Leading_Jet_phi").set(*event, jets->at(0)->phi(), sys);
        m_Fbranches.at("Leading_Jet_E").set(*event, jets->at(0)->e(), sys);   
      }

      if (jets->size()>=2)
      {
        m_Fbranches.at("Subleading_Jet_pt").set(*event, jets->at(1)->pt(), sys);
        m_Fbranches.at("Subleading_Jet_eta").set(*event, jets->at(1)->eta(), sys);
        m_Fbranches.at("Subleading_Jet_phi").set(*event, jets->at(1)->phi(), sys);
        m_Fbranches.at("Subleading_Jet_E").set(*event, jets->at(1)->e(), sys);
      }

      if (bjets->size()>=1)
      {
        m_Fbranches.at("Jet_pt_b1").set(*event, bjets->at(0)->pt(), sys);
        m_Fbranches.at("Jet_eta_b1").set(*event, bjets->at(0)->eta(), sys);
        m_Fbranches.at("Jet_phi_b1").set(*event, bjets->at(0)->phi(), sys);
        m_Fbranches.at("Jet_E_b1").set(*event, bjets->at(0)->e(), sys);
      }
      if (bjets->size()>=2)
      {
        m_Fbranches.at("Jet_pt_b2").set(*event, bjets->at(1)->pt(), sys);
        m_Fbranches.at("Jet_eta_b2").set(*event, bjets->at(1)->eta(), sys);
        m_Fbranches.at("Jet_phi_b2").set(*event, bjets->at(1)->phi(), sys);
        m_Fbranches.at("Jet_E_b2").set(*event, bjets->at(1)->e(), sys);

        // build the H(bb) candidate
        bb = bjets->at(0)->p4()+bjets->at(1)->p4();
        m_Fbranches.at("mbb").set(*event, bb.M(), sys);
        m_Fbranches.at("pTbb").set(*event, bb.Pt(), sys);
        m_Fbranches.at("Etabb").set(*event, bb.Eta(), sys);
        m_Fbranches.at("Phibb").set(*event, bb.Phi(), sys);
        m_Fbranches.at("dRbb").set(*event, (bjets->at(0)->p4()).DeltaR(bjets->at(1)->p4()), sys);
      }


    }
    return StatusCode::SUCCESS;
  }
}
