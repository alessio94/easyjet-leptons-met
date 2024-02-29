/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MMCSelectorAlg.h"

#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>

namespace HHBBTT
{
  MMCSelectorAlg::MMCSelectorAlg(const std::string &name,
				 ISvcLocator *pSvcLocator)
    : EL::AnaAlgorithm(name, pSvcLocator)
  {

  }

  StatusCode MMCSelectorAlg::initialize()
  {
    // Initialise global event filter
    ATH_CHECK (m_filterParams.initialize(m_systematicsList));

    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    ATH_CHECK (m_pass_SLT.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_pass_LTT.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_pass_SLT_1B.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_pass_LTT_1B.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_pass_STT.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_pass_DTT.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_pass_STT_1B.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_pass_DTT_1B.initialize(m_systematicsList, m_eventHandle));

    ATH_CHECK (m_mmc_m.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    for ( auto name : m_channel_names){
      if( name == "lephad") m_channels.push_back(HHBBTT::LepHad);
      else if( name == "lephad1b") m_channels.push_back(HHBBTT::LepHad1B);
      else if ( name == "hadhad") m_channels.push_back(HHBBTT::HadHad);
      else if ( name == "hadhad1b") m_channels.push_back(HHBBTT::HadHad1B);
    }

    return StatusCode::SUCCESS;
  }

  StatusCode MMCSelectorAlg ::execute()
  {
    // Global filter originally false
    CP::SysFilterReporterCombiner filterCombiner (m_filterParams, false);

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      CP::SysFilterReporter filter (filterCombiner, sys);

      // Retrive inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      // Check if one of the signal regions
      bool is_lephad = false;
      bool is_hadhad = false;

      for(const auto& channel : m_channels){
        if(channel == HHBBTT::LepHad)
          is_lephad = m_pass_SLT.get(*event, sys) || m_pass_LTT.get(*event, sys);
        else if(channel == HHBBTT::LepHad1B)
          is_lephad = m_pass_SLT_1B.get(*event, sys) || m_pass_LTT_1B.get(*event, sys);
        else if(channel == HHBBTT::HadHad)
            is_hadhad |= m_pass_STT.get(*event, sys) || m_pass_DTT.get(*event, sys);
        else if(channel == HHBBTT::HadHad1B)
            is_hadhad |= m_pass_STT_1B.get(*event, sys) || m_pass_DTT_1B.get(*event, sys);
      }

      // No MMC selection for CRs -> bail out early
      if (!(is_lephad || is_hadhad)) {
        filter.setPassed(true);

        continue;
      }

      MMC_MASS = false;

      if (m_mmc_m.get(*event, sys) > m_mmc_min)
        MMC_MASS = true;

      if (!m_bypass && !MMC_MASS) continue;

      // Global event filter true if any syst passes and controls
      // if event is passed to output writing or not
      filter.setPassed(true);
    }

    return StatusCode::SUCCESS;
  }

  StatusCode MMCSelectorAlg::finalize() {
    ANA_CHECK (m_filterParams.finalize ());
    return StatusCode::SUCCESS;
  }

}

