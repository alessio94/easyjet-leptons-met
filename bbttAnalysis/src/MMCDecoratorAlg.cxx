/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Carl Gwilliam

#include "MMCDecoratorAlg.h"
#include <AthenaKernel/Units.h>
#include <AthContainers/ConstDataVector.h>

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

    ATH_CHECK (m_pass_LepHad.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_pass_HadHad.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_pass_LepHad_1B.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_pass_HadHad_1B.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_pass_LepHad_OS.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK (m_selected_el.initialize(m_systematicsList, m_electronHandle));
    ATH_CHECK (m_selected_mu.initialize(m_systematicsList, m_muonHandle));
    ATH_CHECK (m_selected_tau.initialize(m_systematicsList, m_tauHandle));
    ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));

    // Initialise syst-aware output decorators
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

    // Initialise syst list (must come after all syst-aware inputs and outputs)
    ANA_CHECK (m_systematicsList.initialize());    

    for ( auto name : m_channel_names){
      if(name == "LepHad") m_channels.push_back(HHBBTT::LepHad);
      else if (name == "HadHad") m_channels.push_back(HHBBTT::HadHad);
      else if (name == "AntiIsoLepHad") m_channels.push_back(HHBBTT::AntiIsoLepHad);
    }

    ATH_CHECK (m_mmcTool.retrieve());

    if (m_method_str == "MLNU3P") 
      m_method = DiTauMassTools::MMCFitMethod::MLNU3P;
    else if (m_method_str == "MAXW") 
      m_method = DiTauMassTools::MMCFitMethod::MAXW;
    else if (m_method_str == "MLM") 
      m_method = DiTauMassTools::MMCFitMethod::MLM;
    else {
      ATH_MSG_ERROR("Unknown MMC method " << m_method_str);
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

  StatusCode MMCDecoratorAlg ::execute()
  {

    MMCResult result_NOSYS;
    bool condition_NOSYS = false;
    bool first_sys = true;

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {

      if(m_skipSystematicsCR && first_sys && sys.name()!=""){
        ATH_MSG_ERROR("The algorithm assumes that nominal comes first in the systematics list.");
        ATH_MSG_ERROR("This is not the case, so the implementation should be revisited.");
        return StatusCode::FAILURE;
      }
      first_sys = false;

      // Retrieve inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      // Initialise all outputs to dummy values
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

      // Check if one of the signal regions
      bool is_lephad = false;
      bool is_hadhad = false;
      bool is_lephad_1B = false;
      bool is_hadhad_1B = false;
      bool is_lephad_OS = false;

      for(const auto& channel : m_channels){
        if(channel == HHBBTT::LepHad || channel == HHBBTT::AntiIsoLepHad){
          is_lephad |= m_pass_LepHad.get(*event, sys);
          is_lephad_1B |= m_pass_LepHad_1B.get(*event, sys);
          is_lephad_OS |= m_pass_LepHad_OS.get(*event, sys);
        }
        else if(channel == HHBBTT::HadHad){
          is_hadhad |= m_pass_HadHad.get(*event, sys);
          is_hadhad_1B |= m_pass_HadHad_1B.get(*event, sys);
        }
      }

      // Bail out early for CRs
      if (!(is_lephad || is_hadhad)) {
        continue;
      }

      // Retrieve nominal results for systematics variation in some case
      // We acknowledge and accept all associated risks and liabilities arising from the use of this block.
      if (m_skipSystematicsCR && sys.name()!="" && condition_NOSYS){
        m_mmc_status.set(*event, result_NOSYS.status, sys);
        m_mmc_pt.set(*event, result_NOSYS.res.Pt(), sys);
        m_mmc_eta.set(*event, result_NOSYS.res.Eta(), sys);
        m_mmc_phi.set(*event, result_NOSYS.res.Phi(), sys);
        m_mmc_m.set(*event, result_NOSYS.res.M(), sys);

        m_mmc_nu1_pt.set(*event, result_NOSYS.nu1.Pt(), sys);
        m_mmc_nu1_eta.set(*event, result_NOSYS.nu1.Eta(), sys);
        m_mmc_nu1_phi.set(*event, result_NOSYS.nu1.Phi(), sys);
        m_mmc_nu1_m.set(*event, result_NOSYS.nu1.M(), sys);

        m_mmc_nu2_pt.set(*event, result_NOSYS.nu2.Pt(), sys);
        m_mmc_nu2_eta.set(*event, result_NOSYS.nu2.Eta(), sys);
        m_mmc_nu2_phi.set(*event, result_NOSYS.nu2.Phi(), sys);
        m_mmc_nu2_m.set(*event, result_NOSYS.nu2.M(), sys);
        continue;
      }

      // Retrieve inputs
      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      if (m_skipSystematicsCR && sys.name()==""){
        auto bjets = std::make_unique<ConstDataVector<xAOD::JetContainer>> (SG::VIEW_ELEMENTS);
        for(const xAOD::Jet* jet : *jets) {
          if (m_isBtag.get(*jet, sys) && std::abs(jet->eta())<2.5)
            bjets->push_back(jet);
        }

        float mbb = -999.;
        if(bjets->size()>=2){
          TLorentzVector b1 = bjets->at(0)->p4();
          TLorentzVector b2 = bjets->at(1)->p4();
          mbb = (b1+b2).M();
        }

        condition_NOSYS = is_lephad &&
          (is_lephad_1B || mbb>150 * Athena::Units::GeV || !is_lephad_OS);
        condition_NOSYS |= is_hadhad_1B;
      }

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
      PtEtaPhiMVector res(0,0,0,0);
      PtEtaPhiMVector nu1(0,0,0,0);
      PtEtaPhiMVector nu2(0,0,0,0);

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
	
	status = m_mmcTool->GetFitStatus(m_method);
	if (status == 1) {
	  res = m_mmcTool->GetResonanceVec(m_method)*1e3;
	  nu1 = m_mmcTool->GetNeutrino4vec(m_method, 0)*1e3;
	  nu2 = m_mmcTool->GetNeutrino4vec(m_method, 1)*1e3;
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

      if(sys.name()==""){
        MMCResult result(status, res, nu1, nu2);
        result_NOSYS = result;
      }
    }

    return StatusCode::SUCCESS;
  }
}
