/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MjjExampleAlg.h"

namespace EXANA
{
  MjjExampleAlg ::MjjExampleAlg(const std::string &name,
                                                  ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode MjjExampleAlg ::initialize()
  {
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK (m_JetHandle.initialize(m_systematicsList));

    // Intialise syst-aware output decorators
      ATH_CHECK (m_mjj.initialize(m_systematicsList, m_eventHandle));

    // Initialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode MjjExampleAlg::execute()
  {
    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_JetHandle.retrieve (jets, sys));

      m_mjj.set(*event, -99., sys);

      float mjj = -99;
      if (jets->size() >= 2) {
        TLorentzVector jet_pair = jets->at(0)->p4() + jets->at(1)->p4();
        mjj = jet_pair.M();
      }

      m_mjj.set(*event, mjj, sys);
    }

    return StatusCode::SUCCESS;
  }
}
