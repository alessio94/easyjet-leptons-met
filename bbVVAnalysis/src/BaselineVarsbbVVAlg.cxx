/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "AthContainers/AuxElement.h"
#include "BaselineVarsbbVVAlg.h"
#include <FourMomUtils/xAODP4Helpers.h>
#include <AthContainers/ConstDataVector.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <AthContainers/ConstDataVector.h>

#include "TLorentzVector.h"

namespace HHBBVV
{
  BaselineVarsbbVVAlg::BaselineVarsbbVVAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {

  }

  StatusCode BaselineVarsbbVVAlg::initialize()
  {

    // Read syst-aware input handles
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_lrjetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    if(m_isMC){
      m_ele_recoSF = CP::SysReadDecorHandle<float>("el_reco_effSF_"+m_eleWPName+"_%SYS%", this);
      m_ele_idSF = CP::SysReadDecorHandle<float>("el_id_effSF_"+m_eleWPName+"_%SYS%", this);
      m_ele_isoSF = CP::SysReadDecorHandle<float>("el_isol_effSF_"+m_eleWPName+"_%SYS%", this);
    }
    ATH_CHECK (m_ele_recoSF.initialize(m_systematicsList, m_electronHandle, SG::AllowEmpty));
    ATH_CHECK (m_ele_idSF.initialize(m_systematicsList, m_electronHandle, SG::AllowEmpty));
    ATH_CHECK (m_ele_isoSF.initialize(m_systematicsList, m_electronHandle, SG::AllowEmpty));

    if(m_isMC){
      m_mu_recoSF = CP::SysReadDecorHandle<float>("muon_reco_effSF_"+m_muWPName+"_%SYS%", this);
      m_mu_isoSF = CP::SysReadDecorHandle<float>("muon_isol_effSF_"+m_muWPName+"_%SYS%", this);
    }
    ATH_CHECK (m_mu_recoSF.initialize(m_systematicsList, m_muonHandle, SG::AllowEmpty ));
    ATH_CHECK (m_mu_isoSF.initialize(m_systematicsList, m_muonHandle, SG::AllowEmpty));

    ATH_CHECK (m_selected_el.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK (m_selected_mu.initialize(m_systematicsList, m_muonHandle));

    // Intialise syst-aware output decorators
    // ATH_CHECK(m_HH_pt.initialize(m_systematicsList, m_eventHandle));
    // ATH_CHECK(m_HH_eta.initialize(m_systematicsList, m_eventHandle));
    // ATH_CHECK(m_HH_phi.initialize(m_systematicsList, m_eventHandle));
    // ATH_CHECK(m_HH_m.initialize(m_systematicsList, m_eventHandle));
    // ATH_CHECK(m_HH_vis_pt.initialize(m_systematicsList, m_eventHandle));
    // ATH_CHECK(m_HH_vis_eta.initialize(m_systematicsList, m_eventHandle));
    // ATH_CHECK(m_HH_vis_phi.initialize(m_systematicsList, m_eventHandle));
    // ATH_CHECK(m_HH_vis_m.initialize(m_systematicsList, m_eventHandle));
    // ATH_CHECK(m_HH_visMet_pt.initialize(m_systematicsList, m_eventHandle));
    // ATH_CHECK(m_HH_visMet_eta.initialize(m_systematicsList, m_eventHandle));
    // ATH_CHECK(m_HH_visMet_phi.initialize(m_systematicsList, m_eventHandle));
    // ATH_CHECK(m_HH_visMet_m.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK(m_selected_lepton_pt.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_selected_lepton_eta.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_selected_lepton_phi.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_selected_lepton_E.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_selected_lepton_charge.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_selected_lepton_pdgid.initialize(m_systematicsList, m_eventHandle));

    if(m_isMC){
      m_selected_lepton_SF = CP::SysWriteDecorHandle<float>("Selected_Lepton_SF_%SYS%", this);
    }
    ATH_CHECK(m_selected_lepton_SF.initialize(m_systematicsList, m_eventHandle, SG::AllowEmpty));

    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());    

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsbbVVAlg::execute()
  {

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {

      // Retrive inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      const xAOD::JetContainer *lrjets = nullptr;
      ANA_CHECK (m_lrjetHandle.retrieve (lrjets, sys));

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

      // selected leptons ; 
      float lepton_pt = -99;
      float lepton_eta = -99;
      float lepton_phi = -99;
      float lepton_E = -99;
      int lepton_charge = -99;
      int lepton_pdgid = -99;
      float lepton_SF = -99;

      for(const xAOD::Electron* electron : *electrons) {
        if (m_selected_el.get(*electron, sys)){
          lepton_pt = electron->pt();
          lepton_eta = electron->eta();
          lepton_phi = electron->phi();
          lepton_E = electron->e();
          lepton_charge = electron->charge();
          lepton_pdgid = electron->charge() > 0 ? -11 : 11;
          if(m_isMC){
            lepton_SF = m_ele_recoSF.get(*electron,sys) *
              m_ele_idSF.get(*electron,sys) * m_ele_isoSF.get(*electron,sys);
          }
          break; // At most one lepton selected
      	}
      }
      for(const xAOD::Muon* muon : *muons) {
        if (m_selected_mu.get(*muon, sys)){
          lepton_pt = muon->pt();
          lepton_eta = muon->eta();
          lepton_phi = muon->phi();
          lepton_E = muon->e();
          lepton_charge = muon->charge();
          lepton_pdgid = muon->charge() > 0 ? -13 : 13;
          if(m_isMC)
            lepton_SF = m_mu_recoSF.get(*muon,sys) * m_mu_isoSF.get(*muon,sys);
          break;
	}
      }
      m_selected_lepton_pt.set(*event, lepton_pt, sys);
      m_selected_lepton_eta.set(*event, lepton_eta, sys);
      m_selected_lepton_phi.set(*event, lepton_phi, sys);
      m_selected_lepton_E.set(*event, lepton_E, sys);
      m_selected_lepton_charge.set(*event, lepton_charge, sys);
      m_selected_lepton_pdgid.set(*event, lepton_pdgid, sys);
      if(m_isMC) m_selected_lepton_SF.set(*event, lepton_SF, sys);

      // DiHiggs mass 
      TLorentzVector bb(0,0,0,0);
      TLorentzVector VV(0,0,0,0);
      TLorentzVector HH(0,0,0,0);
      TLorentzVector HH_vis(0,0,0,0);
      TLorentzVector HH_visMet(0,0,0,0);
 
      // TODO: implement bbVV variables: Hbb, Whad, HH system
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
