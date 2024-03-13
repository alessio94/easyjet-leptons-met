/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Carl Gwilliam

#include "MMCDecoratorAlg.h"
#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>

#include "TLorentzVector.h"

namespace HHBBLL
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
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    // Intialise syst-aware output decorators
    ATH_CHECK(m_mmc_status.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_mmc_pt.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_mmc_eta.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_mmc_phi.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_mmc_m.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ANA_CHECK (m_systematicsList.initialize());    

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

      // Setup MMC inputs and outputs
      const xAOD::IParticle* part1 = nullptr;
      const xAOD::IParticle* part2 = nullptr;
      int status = 0;      
      TLorentzVector res(0,0,0,0);
      const auto method = DiTauMassTools::MMCFitMethodV2::MLNU3P;
     
      if (electrons->size() == 2 && muons->size() == 0) {
  part1 = electrons->at(0);
  part2 = electrons->at(1);
      }
      else if (electrons->size() == 0 && muons->size() == 2)
      {
	part1 = muons->at(0);
  part2 = muons->at(1);
      }
      else if (electrons->size() == 1 && muons->size() == 1) 
      {
	part1 = electrons->at(0);
  part2 = muons->at(0);
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
	  res = m_mmcTool->GetResonanceVec(method);	
	}
      }

      // Decorate ouput
      m_mmc_status.set(*event, status, sys);
      m_mmc_pt.set(*event, res.Pt(), sys);
      m_mmc_eta.set(*event, res.Eta(), sys);
      m_mmc_phi.set(*event, res.Phi(), sys);
      m_mmc_m.set(*event, res.M(), sys);            
    }

    return StatusCode::SUCCESS;
  }
}
