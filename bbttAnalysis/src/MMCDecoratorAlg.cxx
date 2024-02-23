/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Carl Gwilliam

#include "MMCDecoratorAlg.h"

#include "TLorentzVector.h"

namespace HHBBTT
{
  MMCDecoratorAlg ::MMCDecoratorAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
    : EL::AnaAlgorithm(name, pSvcLocator)
  {
  
  }

  StatusCode MMCDecoratorAlg ::initialize()
  {

    // Read syst-aware input handles
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_tauHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    ATH_CHECK (m_pass_SLT.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_pass_LTT.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_pass_STT.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_pass_DTT.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_pass_STT_1B.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_pass_DTT_1B.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_pass_ZCR.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_pass_TopEMuCR.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK (m_selected_el.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK (m_selected_mu.initialize(m_systematicsList, m_muonHandle));
    ATH_CHECK (m_selected_tau.initialize(m_systematicsList, m_tauHandle));

    // Intialise syst-aware output decorators
    ATH_CHECK(m_mmc_status.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_mmc_pt.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_mmc_eta.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_mmc_phi.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_mmc_m.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK(m_mmc_nu1_pt.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_mmc_nu1_eta.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_mmc_nu1_phi.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_mmc_nu1_m.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK(m_mmc_nu2_pt.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_mmc_nu2_eta.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_mmc_nu2_phi.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_mmc_nu2_m.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ANA_CHECK (m_systematicsList.initialize());    

    for ( auto name : m_channel_names){
      if( name == "lephad") m_channels.push_back(HHBBTT::LepHad);
      else if ( name == "hadhad") m_channels.push_back(HHBBTT::HadHad);
      else if ( name == "hadhad1b") m_channels.push_back(HHBBTT::HadHad1B);
      else if ( name == "ZCR") m_channels.push_back(HHBBTT::ZCR);
      else if ( name == "TopEMuCR") m_channels.push_back(HHBBTT::TopEMuCR);
      else{
        ATH_MSG_ERROR("Unknown channel");
        return StatusCode::FAILURE;
      }
    }

    // Initialise MMC tool
    m_mmcTool.reset(new DiTauMassTools::MissingMassToolV2("MissingMassToolV2"));
    ATH_CHECK (m_mmcTool->setProperty("Decorate", false));
    ATH_CHECK (m_mmcTool->setProperty("UseVerbose", m_verbose));
    ATH_CHECK (m_mmcTool->setProperty("FloatStoppingCrit", m_float_stop));
    ATH_CHECK (m_mmcTool->setProperty("CalibSet", m_calib_set));
    ATH_CHECK (m_mmcTool->initialize());

    if (m_method_str == "MLNU3P") 
      m_method = DiTauMassTools::MMCFitMethodV2::MLNU3P;
    else if (m_method_str == "MAXW") 
      m_method = DiTauMassTools::MMCFitMethodV2::MAXW;
    else if (m_method_str == "MLM") 
      m_method = DiTauMassTools::MMCFitMethodV2::MLM;
    else {
      ATH_MSG_ERROR("Unknown MMC method " << m_method_str);
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

  StatusCode MMCDecoratorAlg ::execute()
  {

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      // Retrive inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      // Bail out early for CRs
      for(const auto& channel : m_channels){
        if ((channel == HHBBTT::ZCR && m_pass_ZCR.get(*event, sys)) ||
            (channel == HHBBTT::TopEMuCR && m_pass_TopEMuCR.get(*event, sys))) {
          m_mmc_status.set(*event, -999, sys);
          m_mmc_pt.set(*event, -999., sys);
          m_mmc_eta.set(*event, -999., sys);
          m_mmc_phi.set(*event, -999., sys);
          m_mmc_m.set(*event, -999., sys);

          m_mmc_nu1_pt.set(*event, -999., sys);
          m_mmc_nu1_eta.set(*event, -999., sys);
          m_mmc_nu1_phi.set(*event, -999., sys);
          m_mmc_nu1_m.set(*event, -999., sys);

          m_mmc_nu2_pt.set(*event, -999., sys);
          m_mmc_nu2_eta.set(*event, -999., sys);
          m_mmc_nu2_phi.set(*event, -999., sys);
          m_mmc_nu2_m.set(*event, -999., sys);

          return StatusCode::SUCCESS;
        }
      }

      // Retrive inputs
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

      // Setup MMC inputs and outputs
      const xAOD::IParticle* part1 = nullptr;
      const xAOD::IParticle* part2 = nullptr;
      int status = 0;      
      TLorentzVector res(0,0,0,0);
      TLorentzVector nu1(0,0,0,0);
      TLorentzVector nu2(0,0,0,0);
      const auto method = DiTauMassTools::MMCFitMethodV2::MLNU3P;

      bool is_lephad = false;
      bool is_hadhad = false;

      for(const auto& channel : m_channels){
        if(channel == HHBBTT::LepHad){
          is_lephad =
            m_pass_SLT.get(*event, sys) || m_pass_LTT.get(*event, sys);
        }
        else if(channel == HHBBTT::HadHad || channel == HHBBTT::HadHad1B){
          if(channel == HHBBTT::HadHad)
            is_hadhad |=
              m_pass_STT.get(*event, sys) || m_pass_DTT.get(*event, sys);
          if(channel == HHBBTT::HadHad1B)
            is_hadhad |=
              m_pass_STT_1B.get(*event, sys) || m_pass_DTT_1B.get(*event, sys);
        }
      }

      for(const xAOD::TauJet* tau : *taus) {
        if (m_selected_tau.get(*tau, sys)){
          if(!part1) part1 = tau;
          else{
            if(!is_hadhad) break;
            else{
              part2 = tau;
              break;
            }
          }
        }
      }

      if(is_lephad){
        for(const xAOD::Electron* electron : *electrons) {
          if (part2) break;
          if (m_selected_el.get(*electron, sys)) part2 = electron;
        }

        for(const xAOD::Muon* muon : *muons) {
          if(part2) break;
          if (m_selected_mu.get(*muon, sys)) part2 = muon;
        }
      }

      // Run MMC if find eligible particle content
      if (part1 && part2) {
	auto code = m_mmcTool->apply(*event, part1, part2, met, jets->size());
	
	if (code != CP::CorrectionCode::Ok) 
	  {
	    ATH_MSG_ERROR("MMC application failed");
	    return StatusCode::FAILURE;	    
	  }
	
	status = m_mmcTool->GetFitStatus(method);	
	if (status == 1) {
	  res = m_mmcTool->GetResonanceVec(method)*1e3;	
	  nu1 = m_mmcTool->GetNeutrino4vec(method, 0)*1e3;	
	  nu2 = m_mmcTool->GetNeutrino4vec(method, 1)*1e3;	
	}
      }

      // Decorate ouput
      m_mmc_status.set(*event, status, sys);
      m_mmc_pt.set(*event, res.Pt(), sys);
      m_mmc_eta.set(*event, res.Eta(), sys);
      m_mmc_phi.set(*event, res.Phi(), sys);
      m_mmc_m.set(*event, res.M(), sys);

      m_mmc_nu1_pt.set(*event, nu1.Pt(), sys);
      m_mmc_nu1_eta.set(*event, nu1.Eta(), sys);
      m_mmc_nu1_phi.set(*event, nu1.Phi(), sys);
      m_mmc_nu1_m.set(*event, nu1.M(), sys);            

      m_mmc_nu2_pt.set(*event, nu2.Pt(), sys);
      m_mmc_nu2_eta.set(*event, nu2.Eta(), sys);
      m_mmc_nu2_phi.set(*event, nu2.Phi(), sys);
      m_mmc_nu2_m.set(*event, nu2.M(), sys);            
    }

    return StatusCode::SUCCESS;
  }
}
