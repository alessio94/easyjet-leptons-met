/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/// @author Kira Abeling, JaeJin Hong
#include "BaselineVarsbbVVAlg.h"
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
      m_ele_SF = CP::SysReadDecorHandle<float>("el_effSF_"+m_eleWPName+"_%SYS%", this);
    }
    ATH_CHECK (m_ele_SF.initialize(m_systematicsList, m_electronHandle, SG::AllowEmpty));

    if(m_isMC){
      m_mu_SF = CP::SysReadDecorHandle<float>("muon_effSF_"+m_muWPName+"_%SYS%", this);
    }
    ATH_CHECK (m_mu_SF.initialize(m_systematicsList, m_muonHandle, SG::AllowEmpty ));

    ATH_CHECK (m_selected_el.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK (m_selected_mu.initialize(m_systematicsList, m_muonHandle));
    ATH_CHECK(m_Hbb.initialize(m_systematicsList, m_lrjetHandle)); // Hbb jet
    ATH_CHECK(m_Whad.initialize(m_systematicsList, m_lrjetHandle)); // Whad jet
    ATH_CHECK(m_Whad2.initialize(m_systematicsList, m_lrjetHandle)); // Whad2 jet

    for(auto wp: m_GN2X_wps)
    {
      CP::SysReadDecorHandle<bool> gnn_handle{"GN2X_select_" + wp, this};
      m_GN2X_wp_Handles.emplace(wp, gnn_handle);
      ATH_CHECK(m_GN2X_wp_Handles.at(wp).initialize(m_systematicsList, m_lrjetHandle));
    }

    m_WTag_score = CP::SysReadDecorHandle<float>
      (m_WTag_Type.value()+m_WTag_WP.value()+"Tagger_Score", this);
    m_Pass_WTag = CP::SysReadDecorHandle<bool>
      (m_WTag_Type.value()+m_WTag_WP.value()+"Tagger_Tagged", this);
    ATH_CHECK(m_WTag_score.initialize(m_systematicsList, m_lrjetHandle));
    ATH_CHECK(m_Pass_WTag.initialize(m_systematicsList, m_lrjetHandle));

    // Initialise syst-aware output decorators
    for(const std::string &var : m_floatVariables){
      CP::SysWriteDecorHandle<float> whandle{var+"_%SYS%", this};
      m_Fbranches.emplace(var, whandle);
      ATH_CHECK(m_Fbranches.at(var).initialize(m_systematicsList, m_eventHandle));
    }

    for(const std::string &var : m_intVariables){
      CP::SysWriteDecorHandle<int> whandle{var+"_%SYS%", this};
      m_Ibranches.emplace(var, whandle);
      ATH_CHECK(m_Ibranches.at(var).initialize(m_systematicsList, m_eventHandle));
    };

    for(const std::string &channel : m_channel_names){
      if (channel.std::string::find("1Lep") != std::string::npos)m_run_lep = true;

      if( channel == "Boosted1Lep") m_channels.push_back(HHBBVV::Boosted1Lep);
      else if ( channel == "SplitBoosted1Lep") m_channels.push_back(HHBBVV::SplitBoosted1Lep);
      else if( channel == "Boosted0Lep") m_channels.push_back(HHBBVV::Boosted0Lep);
      else if ( channel == "SplitBoosted0Lep") m_channels.push_back(HHBBVV::SplitBoosted0Lep);
      else{
        ATH_MSG_ERROR("Unknown channel: "
          << channel << std::endl
          << "Available are: [\"Boosted1Lep\", \"SplitBoosted1Lep\", \"Boosted0Lep\", \"SplitBoosted0lep\"]");
        return StatusCode::FAILURE;
      }
      ATH_MSG_DEBUG("Running Channel: " << channel);
    };

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

      for (const std::string &string_var: m_floatVariables) { // Initialize
        m_Fbranches.at(string_var).set(*event, -99., sys);
      }
      
      for (const std::string &string_var: m_intVariables) {
        m_Ibranches.at(string_var).set(*event, -99, sys);
      }


      // Calculate vars

      TLorentzVector signal_lepton;
      float signal_lepton_SF = -99.;
      int signal_lepton_charge = -99;
      int signal_lepton_id = -99;

      for(const xAOD::Electron* electron : *electrons) {
        if (m_selected_el.get(*electron, sys)){
          signal_lepton = electron->p4();
          signal_lepton_SF = m_ele_SF.get(*electron, sys);
          signal_lepton_charge = electron->charge();
          signal_lepton_id = signal_lepton_charge > 0 ? -11 : 11;
          break; // At most one lepton selected
      	}
      }
      for(const xAOD::Muon* muon : *muons) {
        if (m_selected_mu.get(*muon, sys)){
          signal_lepton = muon->p4();
          signal_lepton_SF = m_mu_SF.get(*muon, sys);
          signal_lepton_charge = muon->charge();
          signal_lepton_id = signal_lepton_charge > 0 ? -13 : 13;
          break; 
        }
      }

      if(m_run_lep){ // Fill in the selected lepton branches
        m_Fbranches.at("Lepton_pt").set(*event, signal_lepton.Pt(), sys);
        m_Fbranches.at("Lepton_eta").set(*event, signal_lepton.Eta(), sys);
        m_Fbranches.at("Lepton_phi").set(*event, signal_lepton.Phi(), sys);
        m_Fbranches.at("Lepton_E").set(*event, signal_lepton.E(), sys);
        m_Fbranches.at("Lepton_effSF").set(*event, signal_lepton_SF, sys);
        m_Ibranches.at("Lepton_charge").set(*event, signal_lepton_charge, sys);
        m_Ibranches.at("Lepton_pdgid").set(*event, signal_lepton_id, sys);
      }

      static const std::vector<SG::ConstAccessor<float>> acc_tau_wta{
        SG::ConstAccessor<float>("Tau1_wta"),
        SG::ConstAccessor<float>("Tau2_wta"),
        SG::ConstAccessor<float>("Tau3_wta"),
        SG::ConstAccessor<float>("Tau4_wta")
      };
      static const std::vector<SG::ConstAccessor<float>> acc_ecf{
        SG::ConstAccessor<float>("ECF1"),
        SG::ConstAccessor<float>("ECF2"),
        SG::ConstAccessor<float>("ECF3"),
      };

      static const SG::AuxElement::ConstAccessor<float>  GN2Xv01_phbb("GN2Xv01_phbb");
      static const SG::AuxElement::ConstAccessor<float>  GN2Xv01_pqcd("GN2Xv01_pqcd");
      static const SG::AuxElement::ConstAccessor<float>  GN2Xv01_phcc("GN2Xv01_phcc");
      static const SG::AuxElement::ConstAccessor<float>  GN2Xv01_ptop("GN2Xv01_ptop");

      for(const xAOD::Jet* lrjet : *lrjets)
      {
        std::string prefix = "";
        if (m_Whad.get(*lrjet, sys))
        {
          prefix = "Whad_Jet";
          m_Fbranches.at(prefix+"_DeltaR").set(*event, lrjet->p4().DeltaR(signal_lepton), sys);

          for(auto channel: m_channels){
            if(channel == HHBBVV::Boosted0Lep){
              for (int i = 0; i < 4; i++){
                if(acc_tau_wta[i].isAvailable(*lrjet)) { // Suggested ConstAccessor
                  float wta_value = acc_tau_wta[i](*lrjet);
                  wta_value = (wta_value < 1e-8) ? -99. : wta_value ;
                  m_Fbranches.at(prefix + "_Tau" + std::to_string(i + 1) + "_wta").set(*event, wta_value, sys);
                }
              }
              for (int i = 0; i < 3; i++){
                if(acc_ecf[i].isAvailable(*lrjet)) { // Suggested ConstAccessor
                  float ecf_value = acc_ecf[i](*lrjet);
                  ecf_value = (ecf_value < 1e-8) ? -99. : ecf_value ;
                  m_Fbranches.at(prefix + "_ECF" + std::to_string(i + 1)).set(*event, ecf_value, sys);
                }
              }
            }
          }
        }
        else if (m_Whad2.get(*lrjet, sys)){
          for(auto channel: m_channels){
            if(channel == HHBBVV::SplitBoosted0Lep)prefix = "Whad2_Jet";
          }
        }
        else if (m_Hbb.get(*lrjet, sys)){
          prefix = "Hbb_Jet";
        }

        if(!prefix.empty()){
          m_Fbranches.at(prefix + "_pt").set(*event, lrjet->pt(), sys);
          m_Fbranches.at(prefix + "_eta").set(*event, lrjet->eta(), sys);
          m_Fbranches.at(prefix + "_phi").set(*event, lrjet->phi(), sys);
          m_Fbranches.at(prefix + "_m").set(*event, lrjet->m(), sys);

          float phbb_score = GN2Xv01_phbb(*lrjet);
          float pqcd_score = GN2Xv01_pqcd(*lrjet);
          float phcc_score = GN2Xv01_phcc(*lrjet);
          float ptop_score = GN2Xv01_ptop(*lrjet);
          float wtag_score = m_WTag_score.get(*lrjet, sys);
          m_Fbranches.at(prefix+"_GN2Xv01_phbb").set(*event, phbb_score, sys);
          m_Fbranches.at(prefix+"_GN2Xv01_pqcd").set(*event, pqcd_score, sys);
          m_Fbranches.at(prefix+"_GN2Xv01_phcc").set(*event, phcc_score, sys);
          m_Fbranches.at(prefix+"_GN2Xv01_ptop").set(*event, ptop_score, sys);
          m_Fbranches.at(prefix+"_"+m_WTag_Type+"_"+m_WTag_WP+"_Score").set(*event, wtag_score, sys);
          
          int pass_wtag = (int)m_Pass_WTag.get(*lrjet, sys);
          m_Ibranches.at(prefix+"_Pass_"+m_WTag_Type+"_"+m_WTag_WP).set(*event, pass_wtag, sys);

          for(auto& wp: m_GN2X_wps)
          {
            int pass_GN2X = (int)m_GN2X_wp_Handles.at(wp).get(*lrjet, sys);
            m_Ibranches.at(prefix+"_Pass_GN2X_"+wp).set(*event, pass_GN2X, sys);
          }
        }
      }

      m_Ibranches.at("lrjets_n").set(*event, lrjets->size(), sys);
      m_Ibranches.at("srjets_n").set(*event, jets->size(), sys);
      m_Ibranches.at("Selected_Lepton_n").set(*event, (electrons->size()+muons->size()), sys);      

      // DiHiggs mass 
      TLorentzVector bb(0,0,0,0);
      TLorentzVector VV(0,0,0,0);
      TLorentzVector HH(0,0,0,0);
      TLorentzVector HH_vis(0,0,0,0);
      TLorentzVector HH_visMet(0,0,0,0);

      // TODO: implement bbVV variables: Hbb, Whad, HH system
    }

    return StatusCode::SUCCESS;
  }
}
