/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LeptonVarsbbyyAlg.h"

namespace HHBBYY
{
  LeptonVarsbbyyAlg::LeptonVarsbbyyAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {}

  StatusCode LeptonVarsbbyyAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("       LeptonVarsbbyyAlg       \n");
    ATH_MSG_INFO("*********************************\n");

    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK(m_year.initialize(m_systematicsList, m_eventHandle));

    m_Ibranches_lep.emplace("nLeptons", CP::SysWriteDecorHandle<int>{"nLeptons_%SYS%", this});
    ATH_CHECK(m_Ibranches_lep.at("nLeptons").initialize(m_systematicsList, m_eventHandle));
  
    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode LeptonVarsbbyyAlg::execute()
  {
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      // In case of special Higgs sample, run only on NOSYS
      if (!m_doSystematics && sys.name()!="") continue;

      // container we read in
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      m_Ibranches_lep.at("nLeptons").set(*event, -99, sys);
      m_Ibranches_lep.at("nLeptons").set(*event, electrons->size() + muons->size(), sys);

    }

    return StatusCode::SUCCESS;
  }

}
