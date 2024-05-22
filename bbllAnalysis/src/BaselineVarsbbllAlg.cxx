/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "BaselineVarsbbllAlg.h"
#include "AthContainers/AuxElement.h"
#include <AthContainers/ConstDataVector.h>

#include "TLorentzVector.h"
#include "CalcGenericMT2/MT2_ROOT.h"

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

    if(m_isMC){
      m_ele_SF = CP::SysReadDecorHandle<float>("el_effSF_"+m_eleWPName+"_%SYS%", this);
    }
    ATH_CHECK (m_ele_SF.initialize(m_systematicsList, m_electronHandle, SG::AllowEmpty));

    if(m_isMC){
      m_mu_SF = CP::SysReadDecorHandle<float>("muon_effSF_"+m_muWPName+"_%SYS%", this);
    }
    ATH_CHECK (m_mu_SF.initialize(m_systematicsList, m_muonHandle, SG::AllowEmpty));

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

    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    ATH_CHECK (m_met_sig.initialize(m_systematicsList, m_metHandle));

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

      for (const std::string &string_var: m_floatVariables) {
        m_Fbranches.at(string_var).set(*event, -99., sys);
      }

      for (const auto& var: m_intVariables) {
        m_Ibranches.at(var).set(*event, -99, sys);
      }
      
      static const SG::AuxElement::ConstAccessor<int>  HadronConeExclTruthLabelID("HadronConeExclTruthLabelID");

      TLorentzVector bb;
      TLorentzVector ee;
      TLorentzVector mumu;
      TLorentzVector emu;
      TLorentzVector Leading_lep;
      TLorentzVector Subleading_lep;
      TLorentzVector Leading_bjet;
      TLorentzVector Subleading_bjet;
      TLorentzVector met_vector;
      TLorentzVector bbll;
      TLorentzVector bbllmet;
      TLorentzVector b1l1;
      TLorentzVector b1l2;
      TLorentzVector b2l1;
      TLorentzVector b2l2;

      int n_jets=0;
      int n_bjets=0;
      int n_electrons=0;
      int n_muons=0;
      std::vector<float> PassElectronIsos;
      std::vector<float> PassMuonIsos;
      int truthLabel_b1 = -99;
      int truthLabel_b2 = -99;
      int nCentralJets = 0;

      // Count electrons
      n_electrons = electrons->size();

      // Count muons
      n_muons = muons->size();

      // Count jets
      n_jets = jets->size();

      // b-jet sector
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

      m_Ibranches.at("nJets").set(*event, n_jets, sys);
      m_Ibranches.at("nElectrons").set(*event, n_electrons, sys);
      m_Ibranches.at("nMuons").set(*event, n_muons, sys);
      m_Ibranches.at("nBJets").set(*event, n_bjets, sys);
      m_Ibranches.at("nCentralJets").set(*event, nCentralJets, sys);

      // Electron sector
      if (electrons->size() > 0)
      {
        // Leading electron
        const xAOD::Electron* ele0 = electrons->at(0);
        m_Fbranches.at("Electron1_pt").set(*event, ele0->pt(), sys);
        m_Fbranches.at("Electron1_eta").set(*event, ele0->eta(), sys);
        m_Fbranches.at("Electron1_phi").set(*event, ele0->phi(), sys);
        m_Fbranches.at("Electron1_E").set(*event, ele0->e(), sys);
        if(m_isMC){
          float ele_SF = m_ele_SF.get(*ele0, sys);
          m_Fbranches.at("Electron1_effSF").set(*event, ele_SF, sys);
        }
      }
      if (electrons->size() >= 2)
      {
        // Subleading electron
        const xAOD::Electron* ele1 = electrons->at(1);
        m_Fbranches.at("Electron2_pt").set(*event, ele1->pt(), sys);
        m_Fbranches.at("Electron2_eta").set(*event, ele1->eta(), sys);
        m_Fbranches.at("Electron2_phi").set(*event, ele1->phi(), sys);
        m_Fbranches.at("Electron2_E").set(*event, ele1->e(), sys);
        if(m_isMC){
          float ele_SF = m_ele_SF.get(*ele1, sys);
          m_Fbranches.at("Electron2_effSF").set(*event, ele_SF, sys);
        }

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
        const xAOD::Muon* mu0 = muons->at(0);
        m_Fbranches.at("Muon1_pt").set(*event, mu0->pt(), sys);
        m_Fbranches.at("Muon1_eta").set(*event, mu0->eta(), sys);
        m_Fbranches.at("Muon1_phi").set(*event, mu0->phi(), sys);
        m_Fbranches.at("Muon1_E").set(*event, mu0->e(), sys);
        if(m_isMC){
          float mu_SF = m_mu_SF.get(*mu0, sys);
          m_Fbranches.at("Muon1_effSF").set(*event, mu_SF, sys);
        }
      }
      if (muons->size() >= 2)
      {
        // Subleading muon
        const xAOD::Muon* mu1 = muons->at(1);
        m_Fbranches.at("Muon2_pt").set(*event, mu1->pt(), sys);
        m_Fbranches.at("Muon2_eta").set(*event, mu1->eta(), sys);
        m_Fbranches.at("Muon2_phi").set(*event, mu1->phi(), sys);
        m_Fbranches.at("Muon2_E").set(*event, mu1->e(), sys);
        if(m_isMC){
          float mu_SF = m_mu_SF.get(*mu1, sys);
          m_Fbranches.at("Muon2_effSF").set(*event, mu_SF, sys);
        }

        // mumu
        mumu = muons->at(0)->p4() + muons->at(1)->p4();
        m_Fbranches.at("mmumu").set(*event, mumu.M(), sys);
        m_Fbranches.at("pTmumu").set(*event, mumu.Pt(), sys);
        m_Fbranches.at("Etamumu").set(*event, mumu.Eta(), sys);
        m_Fbranches.at("Phimumu").set(*event, mumu.Phi(), sys);
        m_Fbranches.at("dRmumu").set(*event, (muons->at(0)->p4()).DeltaR(muons->at(1)->p4()), sys);
      }// end muon

      //emu
      if (electrons->size() == 1 && muons->size() == 1)
      {
        emu = electrons->at(0)->p4() + muons->at(0)->p4();
        m_Fbranches.at("memu").set(*event, emu.M(), sys);
        m_Fbranches.at("pTemu").set(*event, emu.Pt(), sys);
        m_Fbranches.at("Etaemu").set(*event, emu.Eta(), sys);
        m_Fbranches.at("Phiemu").set(*event, emu.Phi(), sys);
        m_Fbranches.at("dRemu").set(*event, (electrons->at(0)->p4()).DeltaR(muons->at(0)->p4()), sys);
      }

      //Leading lepton
      if (electrons->size() >= 1 || muons->size() >= 1)
      {
        const xAOD::Muon* mu0 = nullptr;
        const xAOD::Electron* ele0 = nullptr;
        float lep1_SF = -99;
        int lep1_charge = -99;
        int lep1_pdgid = -99;
        if (electrons->size() >= 1)
          ele0 = electrons->at(0);
        if (muons->size() >= 1)
          mu0 = muons->at(0);

        if (ele0 && !mu0){
          Leading_lep = ele0->p4();
          if(m_isMC) lep1_SF = m_ele_SF.get(*ele0, sys);
          lep1_charge = ele0->charge();
          lep1_pdgid = ele0->charge() > 0 ? -11 : 11;
        }

	else if (!ele0 && mu0) {
          Leading_lep = mu0->p4();
	  if(m_isMC) lep1_SF = m_mu_SF.get(*mu0, sys);
          lep1_charge = mu0->charge();
          lep1_pdgid = mu0->charge() > 0 ? -13 : 13;
        }

	else if (ele0 && mu0) {
          if (ele0->pt() > mu0->pt()){
            Leading_lep =  ele0->p4();
            if(m_isMC) lep1_SF = m_ele_SF.get(*ele0, sys);
            lep1_charge = ele0->charge();
            lep1_pdgid = ele0->charge() > 0 ? -11 : 11;
          } else {
            Leading_lep = mu0->p4();
            if(m_isMC) lep1_SF = m_mu_SF.get(*mu0, sys);
            lep1_charge = mu0->charge();
            lep1_pdgid = mu0->charge() > 0 ? -13 : 13;
          }
        }

        m_Fbranches.at("Lepton1_pt").set(*event, Leading_lep.Pt(), sys);
        m_Fbranches.at("Lepton1_eta").set(*event, Leading_lep.Eta(), sys);
        m_Fbranches.at("Lepton1_phi").set(*event, Leading_lep.Phi(), sys);
        m_Fbranches.at("Lepton1_E").set(*event, Leading_lep.E(), sys);
        if(m_isMC) m_Fbranches.at("Lepton1_effSF").set(*event, lep1_SF, sys);
        m_Ibranches.at("Lepton1_charge").set(*event, lep1_charge, sys);
        m_Ibranches.at("Lepton1_pdgid").set(*event, lep1_pdgid, sys);
      }

      //Subleading lepton
      if (electrons->size() + muons->size() == 2)
      {
        const xAOD::Electron* ele0 = nullptr;
        const xAOD::Muon* mu0 = nullptr;
        const xAOD::Electron* ele1 = nullptr;
        const xAOD::Muon* mu1 = nullptr;
        float lep2_SF = -99;
        int lep2_charge = -99;
        int lep2_pdgid = -99;

        if (electrons->size() == 2)
          ele1 = electrons->at(1);
        else if (muons->size() == 2)
          mu1 = muons->at(1);
        else {
          ele0 = electrons->at(0);
          mu0 = muons->at(0);
        }

        if (ele1){
          Subleading_lep = ele1->p4();
          if(m_isMC) lep2_SF = m_ele_SF.get(*ele1, sys);
          lep2_charge = ele1->charge();
          lep2_pdgid = ele1->charge() > 0 ? -11 : 11;
        }

	else if (mu1) {
          Subleading_lep = mu1->p4();
          if(m_isMC) lep2_SF = m_mu_SF.get(*mu1, sys);
          lep2_charge = mu1->charge();
          lep2_pdgid = mu1->charge() > 0 ? -13 : 13;
        }

	else if (ele0 && mu0) {
          if (ele0->pt() > mu0->pt()){
            Subleading_lep = mu0->p4();
            if(m_isMC) lep2_SF = m_mu_SF.get(*mu0, sys);
            lep2_charge = mu0->charge();
            lep2_pdgid = mu0->charge() > 0 ? -13 : 13;
          } else {
            Subleading_lep = ele0->p4();
            if(m_isMC) lep2_SF = m_ele_SF.get(*ele0, sys);
            lep2_charge = ele0->charge();
            lep2_pdgid = ele0->charge() > 0 ? -11 : 11;
          }
        }

        m_Fbranches.at("Lepton2_pt").set(*event, Subleading_lep.Pt(), sys);
        m_Fbranches.at("Lepton2_eta").set(*event, Subleading_lep.Eta(), sys);
        m_Fbranches.at("Lepton2_phi").set(*event, Subleading_lep.Phi(), sys);
        m_Fbranches.at("Lepton2_E").set(*event, Subleading_lep.E(), sys);
        if(m_isMC) m_Fbranches.at("Lepton2_effSF").set(*event, lep2_SF, sys);
        m_Ibranches.at("Lepton2_charge").set(*event, lep2_charge, sys);
        m_Ibranches.at("Lepton2_pdgid").set(*event, lep2_pdgid, sys);
      }

      //ll_m
      double ll_m = (electrons->size() == 2 && muons->size() == 0) ? ee.M() :
        (electrons->size() == 0 && muons->size() == 2) ? mumu.M() :
        (electrons->size() == 1 && muons->size() == 1 ) ? emu.M() : -99;
      m_Fbranches.at("mll").set(*event, ll_m, sys);

      //ll_pt
      double ll_pt = (electrons->size() == 2 && muons->size() == 0) ? ee.Pt() :
        (electrons->size() == 0 && muons->size() == 2) ? mumu.Pt() :
        (electrons->size() == 1 && muons->size() == 1 ) ? emu.Pt() : -99;
      m_Fbranches.at("pTll").set(*event, ll_pt, sys);

      // ll_dR
      double ll_dR = -99.;
      if(electrons->size() == 2 && muons->size() == 0) ll_dR = electrons->at(0)->p4().DeltaR(electrons->at(1)->p4());
      else if(electrons->size() == 0 && muons->size() == 2) ll_dR = muons->at(0)->p4().DeltaR(muons->at(1)->p4());
      else if(electrons->size() == 1 && muons->size() == 1 ) ll_dR = electrons->at(0)->p4().DeltaR(muons->at(0)->p4());
      m_Fbranches.at("dRll").set(*event, ll_dR, sys);

      //jet sector
      if (jets->size()>=1)
      {
        m_Fbranches.at("Jet1_pt").set(*event, jets->at(0)->pt(), sys);
        m_Fbranches.at("Jet1_eta").set(*event, jets->at(0)->eta(), sys);
        m_Fbranches.at("Jet1_phi").set(*event, jets->at(0)->phi(), sys);
        m_Fbranches.at("Jet1_E").set(*event, jets->at(0)->e(), sys);   
      }

      if (jets->size()>=2)
      {
        m_Fbranches.at("Jet2_pt").set(*event, jets->at(1)->pt(), sys);
        m_Fbranches.at("Jet2_eta").set(*event, jets->at(1)->eta(), sys);
        m_Fbranches.at("Jet2_phi").set(*event, jets->at(1)->phi(), sys);
        m_Fbranches.at("Jet2_E").set(*event, jets->at(1)->e(), sys);
      }

      //b-jet sector
      if (bjets->size()>=1)
      {
        m_Fbranches.at("Jet_b1_pt").set(*event, bjets->at(0)->pt(), sys);
        m_Fbranches.at("Jet_b1_eta").set(*event, bjets->at(0)->eta(), sys);
        m_Fbranches.at("Jet_b1_phi").set(*event, bjets->at(0)->phi(), sys);
        m_Fbranches.at("Jet_b1_E").set(*event, bjets->at(0)->e(), sys);

	if (m_isMC) {
          truthLabel_b1 = HadronConeExclTruthLabelID(*bjets->at(0));
          m_Ibranches.at("Jet_b1_truthLabel").set(*event, truthLabel_b1, sys);
        }
      }
      if (bjets->size()>=2)
      {
        m_Fbranches.at("Jet_b2_pt").set(*event, bjets->at(1)->pt(), sys);
        m_Fbranches.at("Jet_b2_eta").set(*event, bjets->at(1)->eta(), sys);
        m_Fbranches.at("Jet_b2_phi").set(*event, bjets->at(1)->phi(), sys);
        m_Fbranches.at("Jet_b2_E").set(*event, bjets->at(1)->e(), sys);

	if (m_isMC) {
          truthLabel_b2 = HadronConeExclTruthLabelID(*bjets->at(1));
          m_Ibranches.at("Jet_b2_truthLabel").set(*event, truthLabel_b2, sys);
        }

        // build the H(bb) candidate
        bb = bjets->at(0)->p4()+bjets->at(1)->p4();
        m_Fbranches.at("mbb").set(*event, bb.M(), sys);
        m_Fbranches.at("pTbb").set(*event, bb.Pt(), sys);
        m_Fbranches.at("Etabb").set(*event, bb.Eta(), sys);
        m_Fbranches.at("Phibb").set(*event, bb.Phi(), sys);
        m_Fbranches.at("dRbb").set(*event, (bjets->at(0)->p4()).DeltaR(bjets->at(1)->p4()), sys);
      }

      // b-jet + lepton sector
      if (bjets->size()>=2 && (n_electrons+n_muons)>=2) {
	b1l1 = bjets->at(0)->p4()+Leading_lep;
	b2l1 = bjets->at(1)->p4()+Leading_lep;
	b1l2 = bjets->at(0)->p4()+Subleading_lep;
	b2l2 = bjets->at(1)->p4()+Subleading_lep;
	double m_b1l1 = b1l1.M();
	double m_b2l1 = b2l1.M();
	double m_b1l2 = b1l2.M();
	double m_b2l2 = b2l2.M();
	double m_bl = std::min(std::max(m_b1l1, m_b2l1), std::max(m_b1l2, m_b2l2));
        m_Fbranches.at("mbl").set(*event, m_bl, sys);
      }

      // MET
      m_Fbranches.at("met_x").set(*event, met->mpx(), sys);
      m_Fbranches.at("met_y").set(*event, met->mpy(), sys);
      
      // DeltaR_min of all 𝑏-tagged jet and lepton combinations
      std::vector<const xAOD::IParticle*> allLeptons;

      for (const auto& electron : *electrons) {
        allLeptons.push_back(electron);
      }

      for (const auto& muon : *muons) {
        allLeptons.push_back(muon);
      }

      std::vector<double> deltaRs;
      for (const auto& lepton : allLeptons) {
        for (const auto& bjet : *bjets) {
          deltaRs.push_back(bjet->p4().DeltaR(lepton->p4()));
        }
      }
      if (!deltaRs.empty()) {
	auto minDeltaR = *std::min_element(std::begin(deltaRs), std::end(deltaRs));
        m_Fbranches.at("dRbl_min").set(*event, minDeltaR, sys);
      }

      // met
      met_vector.SetPtEtaPhiE(met->met(), 0, met->phi(), met->met());
      float met_sig = m_met_sig.get(*met, sys);
      m_Fbranches.at("MET_sig").set(*event, met_sig, sys);

      // combine bb + ll
      if (bjets->size()>=2 && (n_electrons+n_muons)>=2) {
        bbll = Leading_lep + Subleading_lep + bb;
	bbllmet = Leading_lep + Subleading_lep + bb + met_vector;
	m_Fbranches.at("mbbll").set(*event, bbll.M(), sys);
	m_Fbranches.at("mbbllmet").set(*event, bbllmet.M(), sys);

        // Ht2r mesure for boostedness of the two Higgs bosons
        Leading_bjet = bjets->at(0)->p4();
        Subleading_bjet = bjets->at(1)->p4();

        double ht2 = (met_vector + Leading_lep + Subleading_lep).Perp() + (Leading_bjet + Subleading_bjet).Perp();
        double ht2r = ht2 / (met->met() + Leading_lep.Pt() + Subleading_lep.Pt() + Leading_bjet.Pt() + Subleading_bjet.Pt());

        m_Fbranches.at("HT2").set(*event, ht2, sys);
        m_Fbranches.at("HT2r").set(*event, ht2r, sys);
      }

      // Transverse mass of the pT-leading lepton wrt met
      if ((electrons->size()+muons->size()) >= 1)
      {
	float mt_lept1_met = TMath::Sqrt(2 * met->met() * Leading_lep.Pt() * (1 - TMath::Cos(Leading_lep.DeltaPhi(met_vector))));
	m_Fbranches.at("mT_Lepton1_Met").set(*event, mt_lept1_met, sys);

	if ((electrons->size()+muons->size()) >= 2)
	{
	  float mt_lept2_met = TMath::Sqrt(2 * met->met() * Subleading_lep.Pt() * (1 - TMath::Cos(Subleading_lep.DeltaPhi(met_vector))));
	  float mt_l_min = std::min(mt_lept1_met, mt_lept2_met);
	  m_Fbranches.at("mT_Lepton2_Met").set(*event, mt_lept2_met, sys);
	  m_Fbranches.at("mT_L_min").set(*event, mt_l_min, sys);
	}
      }
      // stransverse mass of b-jet pair MT2_bb
      ComputeMT2 mt2_calculator = ComputeMT2(Leading_bjet, Subleading_bjet, met_vector, 0, 0);
      double mT2_bb = mt2_calculator.Compute();
      m_Fbranches.at("mT2_bb").set(*event, mT2_bb, sys);

    }

    return StatusCode::SUCCESS;
  }

}


