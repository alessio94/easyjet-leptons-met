/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "BaselineVarsbbllAlg.h"
#include "AthContainers/AuxElement.h"
#include <AthContainers/ConstDataVector.h>

#include "TLorentzVector.h"
#include "EasyjetHub/MT2_ROOT.h"

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
      ATH_CHECK (m_ele_SF.initialize(m_systematicsList, m_electronHandle));
      ATH_CHECK (m_ele_truthOrigin.initialize(m_systematicsList, m_electronHandle));
      ATH_CHECK (m_ele_truthType.initialize(m_systematicsList, m_electronHandle));
    }

    if(m_isMC){
      m_mu_SF = CP::SysReadDecorHandle<float>("muon_effSF_"+m_muWPName+"_%SYS%", this);
      ATH_CHECK (m_mu_SF.initialize(m_systematicsList, m_muonHandle));
      ATH_CHECK (m_mu_truthOrigin.initialize(m_systematicsList, m_muonHandle));
      ATH_CHECK (m_mu_truthType.initialize(m_systematicsList, m_muonHandle));
    }

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
    for (const std::string &var : m_PCBTnames) {
      ATH_MSG_DEBUG("initializing PCBT: " << var);
      CP::SysReadDecorHandle<int> rhandle{var, this};
      m_PCBTs.emplace(var, rhandle);
      ATH_CHECK (m_PCBTs.at(var).initialize(m_systematicsList, m_jetHandle));
    };
    ATH_CHECK (m_nmuons.initialize(m_systematicsList, m_jetHandle));

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

      TLorentzVector Leading_lep;
      TLorentzVector Subleading_lep;
      TLorentzVector Leading_bjet;
      TLorentzVector Subleading_bjet;
      TLorentzVector met_vector;
      TLorentzVector bb;
      TLorentzVector ll;
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
      const xAOD::Electron* ele0 = nullptr;
      const xAOD::Electron* ele1 = nullptr;

      for(unsigned int i=0; i<std::min(size_t(2),electrons->size()); i++){
        const xAOD::Electron* ele = electrons->at(i);
        if(i==0) ele0 = ele;
        else if(i==1) ele1 = ele;
      }

      // Muon sector
      const xAOD::Muon* mu0 = nullptr;
      const xAOD::Muon* mu1 = nullptr;

      for(unsigned int i=0; i<std::min(size_t(2),muons->size()); i++){
        const xAOD::Muon* mu = muons->at(i);
        if(i==0) mu0 = mu;
        else if(i==1) mu1 = mu;
      }// end muon

      std::vector<std::pair<const xAOD::IParticle*, int>> leptons;
      if(ele0) leptons.emplace_back(ele0, -11*ele0->charge());
      if(mu0) leptons.emplace_back(mu0, -13*mu0->charge());
      if(ele1) leptons.emplace_back(ele1, -11*ele1->charge());
      if(mu1) leptons.emplace_back(mu1, -13*mu1->charge());

      std::sort(leptons.begin(), leptons.end(),
		[](const std::pair<const xAOD::IParticle*, int>& a,
		   const std::pair<const xAOD::IParticle*, int>& b) {
		  return a.first->pt() > b.first->pt(); });

      for(unsigned int i=0; i<std::min(size_t(2),leptons.size()); i++){
        std::string prefix = "Lepton"+std::to_string(i+1);
        TLorentzVector tlv = leptons[i].first->p4();
        int lep_pdgid = leptons[i].second;
        if(i==0) Leading_lep = tlv;
        else if(i==1) Subleading_lep = tlv;
        m_Fbranches.at(prefix+"_pt").set(*event, tlv.Pt(), sys);
        m_Fbranches.at(prefix+"_eta").set(*event, tlv.Eta(), sys);
        m_Fbranches.at(prefix+"_phi").set(*event, tlv.Phi(), sys);
        m_Fbranches.at(prefix+"_E").set(*event, tlv.E(), sys);
        if(m_isMC){
          float SF = std::abs(lep_pdgid)==11 ?
            m_ele_SF.get(*leptons[i].first,sys) :
            m_mu_SF.get(*leptons[i].first,sys);
          m_Fbranches.at(prefix+"_effSF").set(*event, SF, sys);
        }
        int charge = leptons[i].second>0 ? -1 : 1;
        m_Ibranches.at(prefix+"_charge").set(*event, charge, sys);
        m_Ibranches.at(prefix+"_pdgid").set(*event, leptons[i].second, sys);
          
        // leptons truth information
          if (m_isMC){
            int lep_truthOrigin = std::abs(lep_pdgid)==11 ?
              m_ele_truthOrigin.get(*leptons[i].first,sys) :
              m_mu_truthOrigin.get(*leptons[i].first,sys);
            m_Ibranches.at(prefix + "_truthOrigin").set(*event, lep_truthOrigin, sys);
            int lep_truthType = std::abs(lep_pdgid)==11 ?
              m_ele_truthType.get(*leptons[i].first,sys) :
              m_mu_truthType.get(*leptons[i].first,sys);
            m_Ibranches.at(prefix + "_truthType").set(*event, lep_truthType, sys);
            int lep_isPrompt = 0;
            if (std::abs(lep_pdgid)==13){ // simplistic
              if (lep_truthType==6) lep_isPrompt=1; // isolated prompts
            } else if (std::abs(lep_pdgid)==11){
              if (lep_truthType==2) lep_isPrompt=1; // isolated prompts
            }
            m_Ibranches.at(prefix + "_isPrompt").set(*event, lep_isPrompt, sys);
          }
        
      }

      if(leptons.size()>=2){
	ll = Leading_lep + Subleading_lep;
	m_Fbranches.at("mll").set(*event, ll.M(), sys);
	m_Fbranches.at("pTll").set(*event, ll.Pt(), sys);
	m_Fbranches.at("Etall").set(*event, ll.Eta(), sys);
	m_Fbranches.at("Phill").set(*event, ll.Phi(), sys);
	m_Fbranches.at("dRll").set(*event, Leading_lep.DeltaR(Subleading_lep), sys);
      }
      //b-jet sector
      for(unsigned int i=0; i<std::min(size_t(2),bjets->size()); i++){
        std::string prefix = "Jet_b"+std::to_string(i+1);
        if(i==0) Leading_bjet = bjets->at(i)->p4();
        else if(i==1) Subleading_bjet = bjets->at(i)->p4();
        m_Fbranches.at(prefix+"_pt").set(*event, bjets->at(i)->pt(), sys);
        m_Fbranches.at(prefix+"_eta").set(*event, bjets->at(i)->eta(), sys);
        m_Fbranches.at(prefix+"_phi").set(*event, bjets->at(i)->phi(), sys);
        m_Fbranches.at(prefix+"_E").set(*event, bjets->at(i)->e(), sys);

        if (m_isMC) {
          int truthLabel = HadronConeExclTruthLabelID(*bjets->at(i));
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
	float uncorrPt = bjets->at(i)->jetP4("NoBJetCalibMomentum").Pt();
	m_Fbranches.at(prefix+"_uncorrPt").set(*event, uncorrPt, sys);
	float muonCorrPt = bjets->at(i)->jetP4("MuonCorrMomentum").Pt();
	m_Fbranches.at(prefix+"_muonCorrPt").set(*event, muonCorrPt, sys);
      }

      if (bjets->size()>=2) {
        // build the H(bb) candidate
        bb = Leading_bjet + Subleading_bjet;
        m_Fbranches.at("mbb").set(*event, bb.M(), sys);
        m_Fbranches.at("pTbb").set(*event, bb.Pt(), sys);
        m_Fbranches.at("Etabb").set(*event, bb.Eta(), sys);
        m_Fbranches.at("Phibb").set(*event, bb.Phi(), sys);
        m_Fbranches.at("dRbb").set(*event, Leading_bjet.DeltaR(Subleading_bjet), sys);
      }

      // b-jet + lepton sector
      if (bjets->size()>=2 && (n_electrons+n_muons)>=2) {
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
        bbll = ll + bb;
        bbllmet = bbll + met_vector;
        m_Fbranches.at("mbbll").set(*event, bbll.M(), sys);
        m_Fbranches.at("mbbllmet").set(*event, bbllmet.M(), sys);

        // Ht2r mesure for boostedness of the two Higgs bosons
        double ht2 = (met_vector + ll).Perp() + bb.Perp();
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


