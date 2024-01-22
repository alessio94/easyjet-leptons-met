/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AthContainers/AuxElement.h"
#include "BaselineVarsbbyyAlg.h"
#include <FourMomUtils/xAODP4Helpers.h>

namespace HHBBYY
{
  BaselineVarsbbyyAlg::BaselineVarsbbyyAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {}

  StatusCode BaselineVarsbbyyAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("       BaselineVarsbbyyAlg       \n");
    ATH_MSG_INFO("*********************************\n");

    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    ATH_CHECK (m_photonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));

    if(m_isMC){
      m_ph_idSF = CP::SysReadDecorHandle<float>("ph_id_effSF_"+m_photonWPName+"_%SYS%", this);
      m_ph_isoSF = CP::SysReadDecorHandle<float>("ph_isol_effSF_"+m_photonWPName+"_%SYS%", this);
    }
    ATH_CHECK (m_ph_idSF.initialize(m_systematicsList, m_photonHandle, SG::AllowEmpty));
    ATH_CHECK (m_ph_isoSF.initialize(m_systematicsList, m_photonHandle, SG::AllowEmpty));

    // Intialise syst-aware output decorators
    // Add MC var
    if(m_isMC) m_Fvarnames.insert(m_Fvarnames.end(), m_Fvarnames_MC.begin(), m_Fvarnames_MC.end());

    for (const std::string &string_var: m_Fvarnames) {
      CP::SysWriteDecorHandle<float> var {string_var+"_%SYS%", this};
      m_Fbranches.emplace(string_var, var);
      ATH_CHECK (m_Fbranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }

    for (const std::string &string_var: m_Ivarnames) {
      CP::SysWriteDecorHandle<int> var {string_var+"_%SYS%", this};
      m_Ibranches.emplace(string_var, var);
      ATH_CHECK (m_Ibranches.at(string_var).initialize(m_systematicsList, m_eventHandle));
    }

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsbbyyAlg::execute()
  {
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      // container we read in
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      const xAOD::PhotonContainer *photons = nullptr;
      ANA_CHECK (m_photonHandle.retrieve (photons, sys));

      const xAOD::MissingETContainer *metCont = nullptr;
      ANA_CHECK (m_metHandle.retrieve (metCont, sys));
      const xAOD::MissingET* met = (*metCont)["Final"];
      if (!met) {
        ATH_MSG_ERROR("Could not retrieve MET");
        return StatusCode::FAILURE;	
      }

      static const SG::AuxElement::ConstAccessor<int>  HadronConeExclTruthLabelID("HadronConeExclTruthLabelID");

      // initialize
      TLorentzVector H_bb(0.,0.,0.,0.);
      TLorentzVector H_yy(0.,0.,0.,0.);
      TLorentzVector HH(0.,0.,0.,0.);
      TLorentzVector y1(0.,0.,0.,0.);
      TLorentzVector y2(0.,0.,0.,0.);
      TLorentzVector lj(0.,0.,0.,0.);
      TLorentzVector sj(0.,0.,0.,0.);
      TLorentzVector b1(0.,0.,0.,0.);
      TLorentzVector b2(0.,0.,0.,0.);

      int lj_passWP = -99, sj_passWP = -99;
      int truthLabel_lj = -99, truthLabel_sj = -99;
      int truthLabel_b1 = -99, truthLabel_b2 = -99;
      double dRHH = -99., dRyy = -99., dRbb = -99.;

      for (const std::string &string_var: m_Fvarnames) {
        m_Fbranches.at(string_var).set(*event, -99., sys);
      }
      
      for (const std::string &string_var: m_Ivarnames) {
        m_Ibranches.at(string_var).set(*event, -99, sys);
      }

      int nCentralJets = 0;
      double HT = 0.; // scalar sum of jet pT

      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      for(const xAOD::Jet* jet : *jets) {
        // Compute scalar pt sum (Ht) for all the jets in the event |eta|<4.4
        HT += jet->pt();

        // count central jets
        if (std::abs(jet->eta())<2.5) {
          nCentralJets++;
        }

        // check if jet is btagged
        if (WPgiven) {
          if (m_isBtag.get(*jet, sys)) bjets->push_back(jet);
        }
      }

      // photon sector
      if (photons->size() >= 1) {
        const xAOD::Photon* ph1 = photons->at(0);
        y1 = ph1->p4();
        m_Fbranches.at("Photon1_pt").set(*event, y1.Pt(), sys);
        m_Fbranches.at("Photon1_eta").set(*event, y1.Eta(), sys);
        m_Fbranches.at("Photon1_phi").set(*event, y1.Phi(), sys);
        m_Fbranches.at("Photon1_E").set(*event, y1.E(), sys);
        if(m_isMC){
          float ph_SF = m_ph_idSF.get(*ph1, sys) * m_ph_isoSF.get(*ph1, sys);
          m_Fbranches.at("Photon1_effSF").set(*event, ph_SF, sys);
        }
      }

      if (photons->size() >= 2) {
        const xAOD::Photon* ph2 = photons->at(1);
        y2 = ph2->p4();

        // Build the H(yy) candidate
        H_yy = y1 + y2;
        dRyy = (y1).DeltaR(y2);

        m_Fbranches.at("Photon2_pt").set(*event, y2.Pt(), sys);
        m_Fbranches.at("Photon2_eta").set(*event, y2.Eta(), sys);
        m_Fbranches.at("Photon2_phi").set(*event, y2.Phi(), sys);
        m_Fbranches.at("Photon2_E").set(*event, y2.E(), sys);
        if(m_isMC){
          float ph_SF = m_ph_idSF.get(*ph2, sys) * m_ph_isoSF.get(*ph2, sys);
          m_Fbranches.at("Photon2_effSF").set(*event, ph_SF, sys);
        }

        m_Fbranches.at("myy").set(*event, H_yy.M(), sys);
        m_Fbranches.at("pTyy").set(*event, H_yy.Pt(), sys);
        m_Fbranches.at("Etayy").set(*event, H_yy.Eta(), sys);
        m_Fbranches.at("Phiyy").set(*event, H_yy.Phi(), sys);
        m_Fbranches.at("dRyy").set(*event, dRyy, sys);

        m_Fbranches.at("Photon1_ptOvermyy").set(*event, y1.Pt()/H_yy.M(), sys);
        m_Fbranches.at("Photon2_ptOvermyy").set(*event, y2.Pt()/H_yy.M(), sys);
      }

      // inclusive jet sector
      if (jets->size()>=1) {
        lj = jets->at(0)->p4();

        m_Fbranches.at("Jet1_pt").set(*event,lj.Pt(),sys);
        m_Fbranches.at("Jet1_eta").set(*event,lj.Eta(),sys);
        m_Fbranches.at("Jet1_phi").set(*event,lj.Phi(),sys);
        m_Fbranches.at("Jet1_E").set(*event,lj.E(),sys);

        lj_passWP = static_cast<int>(m_isBtag.get(*jets->at(0), sys));
        m_Ibranches.at("Jet1_PassWP").set(*event,lj_passWP,sys);

        if (m_isMC) {
          truthLabel_lj = HadronConeExclTruthLabelID(*jets->at(0));
          m_Ibranches.at("Jet1_truthLabel").set(*event, truthLabel_lj, sys);
        } 
      }

      if (jets->size()>=2) {
        sj = jets->at(1)->p4();

        m_Fbranches.at("Jet2_pt").set(*event,sj.Pt(),sys);
        m_Fbranches.at("Jet2_eta").set(*event,sj.Eta(),sys);
        m_Fbranches.at("Jet2_phi").set(*event,sj.Phi(),sys);
        m_Fbranches.at("Jet2_E").set(*event,sj.E(),sys);

        sj_passWP = static_cast<int>(m_isBtag.get(*jets->at(1), sys));
        m_Ibranches.at("Jet2_PassWP").set(*event,sj_passWP,sys);

        if (m_isMC) {
          truthLabel_sj = HadronConeExclTruthLabelID(*jets->at(1));
          m_Ibranches.at("Jet2_truthLabel").set(*event, truthLabel_sj, sys);
        } 
      }

      // b-jet sector
      if (bjets->size() >= 1) {
        b1 = bjets->at(0)->p4();

        m_Fbranches.at("Jet_b1_pt").set(*event, b1.Pt(), sys);
        m_Fbranches.at("Jet_b1_eta").set(*event, b1.Eta(), sys);
        m_Fbranches.at("Jet_b1_phi").set(*event, b1.Phi(), sys);
        m_Fbranches.at("Jet_b1_E").set(*event, b1.E(), sys);

        if (m_isMC) {
          truthLabel_b1 = HadronConeExclTruthLabelID(*bjets->at(0));
          m_Ibranches.at("Jet_b1_truthLabel").set(*event, truthLabel_b1, sys);
        } 
      }
      if (bjets->size() >= 2) {
        b2 = bjets->at(1)->p4();

        // Build the H(bb) candidate
        H_bb = b1 + b2;
        dRbb = (b1).DeltaR(b2);

        m_Fbranches.at("Jet_b2_pt").set(*event, b2.Pt(), sys);
        m_Fbranches.at("Jet_b2_eta").set(*event, b2.Eta(), sys);
        m_Fbranches.at("Jet_b2_phi").set(*event, b2.Phi(), sys);
        m_Fbranches.at("Jet_b2_E").set(*event, b2.E(), sys);

        if (m_isMC) {
          truthLabel_b2 = HadronConeExclTruthLabelID(*bjets->at(1));
          m_Ibranches.at("Jet_b2_truthLabel").set(*event, truthLabel_b2, sys);
        }

        m_Fbranches.at("mbb").set(*event, H_bb.M(), sys);
        m_Fbranches.at("pTbb").set(*event, H_bb.Pt(), sys);
        m_Fbranches.at("Etabb").set(*event, H_bb.Eta(), sys);
        m_Fbranches.at("Phibb").set(*event, H_bb.Phi(), sys);
        m_Fbranches.at("dRbb").set(*event, dRbb, sys);
      }

      // Build the HH candidate
      if (photons->size() >= 2 && bjets->size() >= 2) {
        HH = H_yy + H_bb;
        dRHH = H_yy.DeltaR(H_bb);

        m_Fbranches.at("pTbbyy").set(*event, HH.Pt(), sys);
        m_Fbranches.at("Etabbyy").set(*event, HH.Eta(), sys);
        m_Fbranches.at("Phibbyy").set(*event, HH.Phi(), sys);
        m_Fbranches.at("dRbbyy").set(*event, dRHH, sys);

        m_Fbranches.at("mbbyy").set(*event, HH.M(), sys);
        m_Fbranches.at("mbbyy_star").set(*event, HH.M()-(H_bb.M()-125e3)-(H_yy.M()-125e3), sys);
      }

      m_Fbranches.at("HT").set(*event, HT, sys);

      if(jets->size()>=3){
	float topness = compute_Topness(jets);
	m_Fbranches.at("topness").set(*event, topness, sys);
      }
      m_Fbranches.at("missEt").set(*event, met->met(), sys);
      m_Fbranches.at("metphi").set(*event, met->phi(), sys);

      m_Ibranches.at("nPhotons").set(*event, photons->size(), sys);
      m_Ibranches.at("nJets").set(*event, jets->size(), sys);
      m_Ibranches.at("nCentralJets").set(*event, nCentralJets, sys);
      m_Ibranches.at("nBJets").set(*event, bjets->size(), sys);

    }

    return StatusCode::SUCCESS;
  }
  
  float BaselineVarsbbyyAlg::compute_Topness(const xAOD::JetContainer *jets){
    float minTopness=std::numeric_limits<float>::max();
    const float wmass=80e3;
    const float topmass=173e3;
    for(unsigned int j1=0;j1<jets->size()-2;j1++){
      for(unsigned int j2=j1+1;j2<jets->size()-1;j2++){
	for(unsigned int j3=j2+1;j3<jets->size();j3++){
	  // compute m_j1j2 and m_j1j2j3
	  float m_j1j2=((*jets)[j1]->p4()+(*jets)[j2]->p4()).M();
	  float m_j1j2j3=((*jets)[j1]->p4()+(*jets)[j2]->p4()+(*jets)[j3]->p4()).M();
	  // find minimum topness
	  float topness=std::hypot((m_j1j2-wmass)/wmass, (m_j1j2j3-topmass)/topmass);
	  if(topness<minTopness) minTopness=topness;
	}
      }
    }    
    return minTopness;
  }

}
