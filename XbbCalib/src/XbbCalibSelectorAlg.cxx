/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "XbbCalibSelectorAlg.h"
#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>
#include <AthenaKernel/Units.h>

namespace XBBCALIB
{

  XbbCalibSelectorAlg::XbbCalibSelectorAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {

  }

  StatusCode XbbCalibSelectorAlg::initialize()
  {
    // Initialise global event filter
    ATH_CHECK (m_filterParams.initialize(m_systematicsList));

    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_lrjetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize()); 

    return StatusCode::SUCCESS;
  }


  StatusCode XbbCalibSelectorAlg::execute()
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

      // Apply selection

      // flags
      PROBE_JET = false;

      //************
      // Large-R Jet
      //************
      for (const xAOD::Jet *jet : *lrjets){
        if (jet->pt() > 300 * Athena::Units::GeV){
          PROBE_JET=true;
        }
      }

      //****************
      // event level info
      //****************
      //for example masses of the system, met, fired triggers etc

      if (!m_bypass && !PROBE_JET) continue;

      // Global event filter true if any syst passes and controls
      // if event is passed to output writing or not
      filter.setPassed(true);

    }

    return StatusCode::SUCCESS;
  }

  StatusCode XbbCalibSelectorAlg::finalize()
  {
    ANA_CHECK (m_filterParams.finalize());
    return StatusCode::SUCCESS;
  }

}
