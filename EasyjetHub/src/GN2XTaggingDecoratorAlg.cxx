/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Thomas Strebler

#include "GN2XTaggingDecoratorAlg.h"

#include <AsgDataHandles/WriteDecorHandle.h>

namespace Easyjet
{
  GN2XTaggingDecoratorAlg::GN2XTaggingDecoratorAlg(const std::string &name,
						   ISvcLocator *pSvcLocator)
    : AthAlgorithm(name, pSvcLocator)
  {}

  StatusCode GN2XTaggingDecoratorAlg::initialize()
  {
    ATH_CHECK(m_jetsInKey.initialize());

    ATH_CHECK(m_seltool.retrieve());
    ATH_CHECK(m_efftool.retrieve());

    m_GN2XTagDecorKey = m_jetsInKey.key() + ".GN2X_select_" + m_WPname;
    ATH_CHECK(m_GN2XTagDecorKey.initialize());

    if(m_isMC){
      m_GN2XSFDecorKeys.emplace_back
	(m_jetsInKey.key() + ".GN2X_effSF_" + m_WPname + "_NOSYS");

      if(m_doSyst){
	CP::SystematicSet sysSet = m_efftool->recommendedSystematics();
	for(const auto& sys : sysSet){
	  m_GN2XSFDecorKeys.emplace_back
	    (m_jetsInKey.key() + ".GN2X_effSF_" + m_WPname + "_" + sys.name());
	}
      }
    }

    for(auto& sfKey : m_GN2XSFDecorKeys) ATH_CHECK(sfKey.initialize());
    
    return StatusCode::SUCCESS;
  }


  StatusCode GN2XTaggingDecoratorAlg::execute()
  {
    // input handles
    SG::ReadHandle<xAOD::JetContainer> jetsIn(m_jetsInKey);
    ATH_CHECK(jetsIn.isValid());

    SG::WriteDecorHandle<xAOD::JetContainer, bool> GN2XTagDecor(m_GN2XTagDecorKey);
    std::vector<SG::WriteDecorHandle<xAOD::JetContainer, float>> GN2XSFDecors;
    for(const auto& sfKey : m_GN2XSFDecorKeys) GN2XSFDecors.emplace_back(sfKey);

    for(const xAOD::Jet* jet : *jetsIn){
      GN2XTagDecor(*jet) = static_cast<bool>(m_seltool->accept(*jet));

      if(m_isMC){
	float sf = 1.;
	if(m_efftool->getScaleFactor(*jet, sf)!=CP::CorrectionCode::Ok) sf = -999.;
	GN2XSFDecors[0](*jet) = sf;

	if(m_doSyst){
	  CP::SystematicSet sysSet = m_efftool->recommendedSystematics();
	  int i=1;

	  for(const auto& sys : sysSet){
	    CP::SystematicSet set;
	    set.insert(sys);
	    ATH_CHECK(m_efftool->applySystematicVariation(set));
	    if(m_efftool->getScaleFactor(*jet, sf) != CP::CorrectionCode::Ok) sf = -999.;
	    GN2XSFDecors[i](*jet) = sf;
	    i++;
	  }

	  // Reconfigure nominal
	  CP::SystematicSet emptySet;
	  ATH_CHECK(m_efftool->applySystematicVariation(emptySet));	  
	}	
      }
    }

    return StatusCode::SUCCESS;
  }

}
