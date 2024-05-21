/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Adam Anderson

#include "NeutrinoWeightingAlg.h"
#include <AthenaKernel/Units.h>

#include <AthContainers/ConstDataVector.h>

#include "TLorentzVector.h"
#include "TRandom3.h"

namespace HHBBLL
{
  NeutrinoWeightingAlg ::NeutrinoWeightingAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
    : EL::AnaAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode NeutrinoWeightingAlg ::initialize()
  {

    // Read syst-aware input handles
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    for (const std::string &cut : m_cutList) {
      CP::SysReadDecorHandle<bool> rhandle{cut+"_%SYS%", this};
      m_cuts.emplace(cut, rhandle);
      ATH_CHECK (m_cuts.at(cut).initialize(m_systematicsList, m_eventHandle));
    }

    //Initialise Neutrino Weighting
    ATH_CHECK(m_reco_pairings.retrieve());

    // Intialise syst-aware output decorators 
    ATH_CHECK(m_solutions.initialize(m_systematicsList, m_eventHandle));

    for (const std::string &var : m_floatVariables) {
      CP::SysWriteDecorHandle<float> whandle{var+"_%SYS%", this};
      m_fBranches.emplace(var, whandle);
      ATH_CHECK(m_fBranches.at(var).initialize(m_systematicsList, m_eventHandle));
    }

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ANA_CHECK (m_systematicsList.initialize());   

    return StatusCode::SUCCESS;
  }

  StatusCode NeutrinoWeightingAlg ::execute()
  {

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {

      // Retrive inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      m_solutions.set(*event, 0, sys);

      for (const std::string &string_var : m_floatVariables) {
        if (string_var == "NW_neutrinoweight") {
          m_fBranches.at(string_var).set(*event, -1., sys);
        } else {
          m_fBranches.at(string_var).set(*event, -99., sys);
        }
      }

      bool pass_NW_check = true;
      for (const std::string &cut : m_cutList) {
        pass_NW_check &= m_cuts.at(cut).get(*event, sys);
      }

      if (!pass_NW_check) {
        continue;
      }

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      bool WPgiven = !m_isBtag.empty();
      auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
      for(const xAOD::Jet* jet : *jets) {
        if (WPgiven) {
          if (m_isBtag.get(*jet, sys)) bjets->push_back(jet);
        }
      }

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

      // Reset Neutrino Weighting
      for (auto& reco_pairing : m_reco_pairings){
        reco_pairing->Reset();
      }

      //Setup neutrino weighting inputs and outputs
      const xAOD::IParticle* lepton1 = nullptr;
      int lepton1_charge = 0;

      const xAOD::IParticle* lepton2 = nullptr;
      int lepton2_charge = 0;

      TLorentzVector lepton_pos;
      TLorentzVector lepton_neg;
      TLorentzVector b;
      TLorentzVector bbar;

      float neutrinoweight = -1.;
      uint solutions = 0;
      TLorentzVector top;
      TLorentzVector tbar;
      TLorentzVector nu;
      TLorentzVector nubar;

      if (muons->size() == 2 and electrons->size() == 0){
        lepton1_charge = muons->at(0)->charge();
        lepton1 = muons->at(0);
        lepton2_charge = muons->at(1)->charge();
        lepton2 = muons->at(1);
      } else if (muons->size() == 0 && electrons->size() == 2){
        lepton1_charge = electrons->at(0)->charge();
        lepton1 = electrons->at(0);
        lepton2_charge = electrons->at(1)->charge();
        lepton2 = electrons->at(1);
      } else if (muons->size() == 1 && electrons->size() == 1){
        lepton1_charge = electrons->at(0)->charge();
        lepton1 = electrons->at(0);
        lepton2_charge = muons->at(0)->charge();
        lepton2 = muons->at(0);
      } else {
        ATH_MSG_ERROR("number of leptons condition not set properly");
        return StatusCode::FAILURE;
      }

      if (lepton1_charge*lepton2_charge > 0){
        ATH_MSG_ERROR("opposite charge leptons condition not set properly");
        return StatusCode::FAILURE;
      }

      lepton_pos = lepton1->p4()*0.001;
      lepton_neg = lepton2->p4()*0.001;
      if(lepton1_charge<0) std::swap(lepton_pos, lepton_neg);

      //Temporarily just setting b and bbar as such until better method found
      b = bjets->at(0)->p4()*0.001;
      bbar = bjets->at(1)->p4()*0.001;
      
      uint nsmears = 2;
      TRandom3 random = TRandom3(1000);

      for (uint i = 0; i < nsmears ; ++i) {
        double mtop = random.Gaus(172.5, 1.480);
        double mtbar = random.Gaus(172.5, 1.480);
        double mWpos = random.Gaus(80.379, 2.085);
        double mWneg = random.Gaus(80.379, 2.085);

        double met_ex = met->mpx()/1000.;
        double met_ey = met->mpy()/1000.;
        
        ATH_CHECK(m_reco_pairings[0]->Reconstruct(lepton_pos, lepton_neg, b, bbar, met_ex, met_ey, mtop, mtbar, mWpos, mWneg));
        ATH_CHECK(m_reco_pairings[1]->Reconstruct(lepton_pos, lepton_neg, bbar, b, met_ex, met_ey, mtop, mtbar, mWpos, mWneg));

        if (m_reco_pairings[0]->GetWeight() > 0. || m_reco_pairings[1]->GetWeight() > 0.) {
          if (m_reco_pairings[0]->GetWeight() > m_reco_pairings[1]->GetWeight()) {
            std::swap(m_reco_pairings[0], m_reco_pairings[1]);
          } 
          neutrinoweight = m_reco_pairings[0]->GetWeight();
          solutions = m_reco_pairings[0]->GetWeights().size();
          top = m_reco_pairings[0]->GetTop();
          tbar = m_reco_pairings[0]->GetTbar();
          nu = m_reco_pairings[0]->GetNu();
          nubar = m_reco_pairings[0]->GetNubar();
        }
      }
 
      m_solutions.set(*event, solutions, sys);
  
      m_fBranches.at("NW_neutrinoweight").set(*event, neutrinoweight, sys);

      m_fBranches.at("NW_top_pt").set(*event, top.Pt(), sys);
      m_fBranches.at("NW_top_eta").set(*event, top.Eta(), sys);
      m_fBranches.at("NW_top_phi").set(*event, top.Phi(), sys);
      m_fBranches.at("NW_top_e").set(*event, top.E(), sys);

      m_fBranches.at("NW_tbar_pt").set(*event, tbar.Pt(), sys);
      m_fBranches.at("NW_tbar_eta").set(*event, tbar.Eta(), sys);
      m_fBranches.at("NW_tbar_phi").set(*event, tbar.Phi(), sys);
      m_fBranches.at("NW_tbar_e").set(*event, tbar.E(), sys);

      TLorentzVector ttbar = top + tbar;

      m_fBranches.at("NW_ttbar_pt").set(*event, ttbar.Pt(), sys);
      m_fBranches.at("NW_ttbar_eta").set(*event, ttbar.Eta(), sys);
      m_fBranches.at("NW_ttbar_phi").set(*event, ttbar.Phi(), sys);
      m_fBranches.at("NW_ttbar_e").set(*event, ttbar.E(), sys);

      m_fBranches.at("NW_nu_pt").set(*event, nu.Pt(), sys);
      m_fBranches.at("NW_nu_eta").set(*event, nu.Eta(), sys);
      m_fBranches.at("NW_nu_phi").set(*event, nu.Phi(), sys);
      m_fBranches.at("NW_nu_e").set(*event, nu.E(), sys);

      m_fBranches.at("NW_nubar_pt").set(*event, nubar.Pt(), sys);
      m_fBranches.at("NW_nubar_eta").set(*event, nubar.Eta(), sys);
      m_fBranches.at("NW_nubar_phi").set(*event, nubar.Phi(), sys);
      m_fBranches.at("NW_nubar_e").set(*event, nubar.E(), sys);

    }
    return StatusCode::SUCCESS;
  }
}
