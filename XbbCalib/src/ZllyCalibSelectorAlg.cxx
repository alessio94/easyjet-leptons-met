/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ZllyCalibSelectorAlg.h"
#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>
#include <AthenaKernel/Units.h>

namespace XBBCALIB
{

  ZllyCalibSelectorAlg::ZllyCalibSelectorAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {

  }

  StatusCode ZllyCalibSelectorAlg::initialize()
  {
    // Initialise global event filter
    ATH_CHECK (m_filterParams.initialize(m_systematicsList));

    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_lrjetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_photonHandle.initialize(m_systematicsList));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize()); 

    return StatusCode::SUCCESS;
  }


  StatusCode ZllyCalibSelectorAlg::execute()
  {

    // Global filter originally false
    CP::SysFilterReporterCombiner filterCombiner (m_filterParams, false);

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      CP::SysFilterReporter filter (filterCombiner, sys);

      // Initialize Variables
      bool pass_baseline = false;

      // Retrive inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *lrjets = nullptr;
      ANA_CHECK (m_lrjetHandle.retrieve (lrjets, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));
      
      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      const xAOD::PhotonContainer *photons = nullptr;
      ANA_CHECK (m_photonHandle.retrieve(photons, sys));

      //************
      // Apply Selection
      //************

      // flags
      //************
      // Photons
      //************
      pass_baseline = (photons->size() != 0);

      //************
      // Leptons
      //************
      pass_baseline &= (electrons->size() >= 2 || muons->size() >= 2);

      //****************
      // event level info
      //****************

      if (!m_bypass && !pass_baseline) continue;

      // Global event filter true if any syst passes and controls
      // if event is passed to output writing or not
      filter.setPassed(true);
    }

    return StatusCode::SUCCESS;
  }

  StatusCode ZllyCalibSelectorAlg::finalize()
  {
    ANA_CHECK (m_filterParams.finalize());
    return StatusCode::SUCCESS;
  }

}
