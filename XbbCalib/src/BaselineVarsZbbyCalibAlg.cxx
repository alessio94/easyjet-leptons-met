/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Unknown

#include "BaselineVarsZbbyCalibAlg.h"
#include "FourVectorOutBlock.h"
#include "DefaultOutputs.h"
#include "SystVariableCopier.h"

#include <format>

namespace XBBCALIB
{

  BaselineVarsZbbyCalibAlg::BaselineVarsZbbyCalibAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  { }

  BaselineVarsZbbyCalibAlg::~BaselineVarsZbbyCalibAlg() = default;

  StatusCode BaselineVarsZbbyCalibAlg::initialize()
  {
    ATH_CHECK (m_lrjetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK (m_photonHandle.initialize(m_systematicsList));

    ATH_CHECK(m_nLRJetsHandle.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_nPhotonsHandle.initialize(m_systematicsList, m_eventHandle));

    m_photon_4vec = std::make_unique<FourVectorOutBlock>(
      this, "photon_", m_systematicsList, m_eventHandle);
    m_z_candidate_4vec = std::make_unique<FourVectorOutBlock>(
      this, "Zcand_", m_systematicsList, m_eventHandle);

    // set up the generic float copying
    m_float_copier = std::make_unique<SystVariableCopier<float>> (
      this,
      m_floats_to_copy,
      m_systematicsList,
      m_lrjetHandle,
      m_eventHandle,
      m_copied_variable_prefix);

    m_int_copier = std::make_unique<SystVariableCopier<int>> (
      this,
      m_ints_to_copy,
      m_systematicsList,
      m_lrjetHandle,
      m_eventHandle,
      m_copied_variable_prefix);

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsZbbyCalibAlg::execute()
  {
    //Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *lrjets = nullptr;
      ANA_CHECK (m_lrjetHandle.retrieve (lrjets, sys));

      const xAOD::PhotonContainer *photons = nullptr;
      ANA_CHECK (m_photonHandle.retrieve(photons, sys));

      // selected Probe Jet
      size_t n_lrjets = lrjets->size();
      ATH_MSG_VERBOSE(
        std::format(
          "Found {} large-R jets running syst {}",
          n_lrjets,
          sys.name()));
      m_nLRJetsHandle.set(*event, n_lrjets, sys);
      if (n_lrjets >= 1)
      {
        const xAOD::Jet* largeJet = lrjets->at(0);
        m_z_candidate_4vec->set(*event, *largeJet, sys);
        m_float_copier->set(*event, *largeJet, sys);
        m_int_copier->set(*event, *largeJet, sys);
      } else {
        m_z_candidate_4vec->setDefault(*event, sys);
        m_float_copier->setDefault(*event, sys);
        m_int_copier->setDefault(*event, sys);
      }

      size_t n_photons = photons->size();
      ATH_MSG_VERBOSE(
        std::format(
          "Found {} photons jets running syst {}",
          n_photons,
          sys.name()));
      m_nPhotonsHandle.set(*event, n_photons, sys);
      // This should be ok, as exactly one photon req.
      if (n_photons >= 1) {
        m_photon_4vec->set(*event, *photons->at(0), sys);
      } else {
        m_photon_4vec->setDefault(*event, sys);
      }

    }
    return StatusCode::SUCCESS;
  }

}



