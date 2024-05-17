/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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

    ATH_CHECK (m_pass_LepHad.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_pass_HadHad.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK (m_mmc_m.initialize(m_systematicsList, m_eventHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    for ( auto name : m_channel_names){
      if( name == "lephad2b") m_channels.push_back(HHBBTT::LepHad2B);
      else if( name == "lephad1b") m_channels.push_back(HHBBTT::LepHad1B);
      else if ( name == "hadhad2b") m_channels.push_back(HHBBTT::HadHad2B);
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
        if(channel == HHBBTT::LepHad2B || channel == HHBBTT::LepHad1B)
          is_lephad |= m_pass_LepHad.get(*event, sys);
        else if(channel == HHBBTT::HadHad2B || channel == HHBBTT::HadHad1B)
          is_hadhad |= m_pass_HadHad.get(*event, sys);
      }

      // No MMC selection for CRs -> bail out early
      if (!(is_lephad || is_hadhad)) {
        filter.setPassed(true);
        continue;
      }

      bool MMC_MASS = m_mmc_m.get(*event, sys) > m_mmc_min;
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

