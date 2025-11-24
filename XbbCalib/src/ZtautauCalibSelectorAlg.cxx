/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ZtautauCalibSelectorAlg.h"
#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>
#include <AthenaKernel/Units.h>

namespace XBBCALIB
{

  ZtautauCalibSelectorAlg::ZtautauCalibSelectorAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {

  }

  StatusCode ZtautauCalibSelectorAlg::initialize()
  {
    // Initialise global event filter
    ATH_CHECK (m_filterParams.initialize(m_systematicsList));
    ATH_CHECK (m_lRjetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_photonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize()); 

    return StatusCode::SUCCESS;
  }


  StatusCode ZtautauCalibSelectorAlg::execute()
  {

    // Global filter originally false
    CP::SysFilterReporterCombiner filterCombiner (m_filterParams, false);

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      CP::SysFilterReporter filter (filterCombiner, sys);

      // Initialize Variables
      bool pass_two_jets = false;
      bool pass_one_jet = false;
      bool pass_baseline = false;

      // Retrive inputs
      const xAOD::JetContainer *lRjets = nullptr;
      ANA_CHECK (m_lRjetHandle.retrieve (lRjets, sys));
      const xAOD::PhotonContainer *photons = nullptr;
      ANA_CHECK (m_photonHandle.retrieve (photons, sys));

      //************
      // Large-R Jet
      //************
      int n_boosted_jets = lRjets->size();

      //************
      // photons
      //************
      int n_photons = photons->size();
      
      //************
      // Apply Selection
      //************
      // Need to have a at least one large-R jet and a recoiling object
      // or 2 large-R jets
      pass_two_jets = n_boosted_jets >= 2;
      pass_one_jet = n_boosted_jets >= 1 && n_photons >= 1;
      pass_baseline = pass_one_jet || pass_two_jets;

      if (!m_bypass && !pass_baseline) continue;

      // Global event filter true if any syst passes and controls
      // if event is passed to output writing or not
      filter.setPassed(true);
    }

    return StatusCode::SUCCESS;
  }

  StatusCode ZtautauCalibSelectorAlg::finalize()
  {
    ANA_CHECK (m_filterParams.finalize());
    return StatusCode::SUCCESS;
  }

}
