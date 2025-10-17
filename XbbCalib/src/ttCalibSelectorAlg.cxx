/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Derrick Allen

#include "ttCalibSelectorAlg.h"
#include "xAODEgamma/Electron.h"
#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>
#include <AthenaKernel/Units.h>
#include <cmath>

namespace XBBCALIB {

  ttCalibSelectorAlg::ttCalibSelectorAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator) {
  }

  StatusCode ttCalibSelectorAlg::initialize() {
    // Initialise global event filter
    ATH_CHECK (m_filterParams.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_lrjetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));

    m_eleWPDecorHandle = CP::SysReadDecorHandle<char>
      ("baselineSelection_" + m_eleWPName+"_%SYS%", this);
    m_muonWPDecorHandle = CP::SysReadDecorHandle<char>
      ("baselineSelection_"+m_muonWPName+"_%SYS%", this);

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }


  StatusCode ttCalibSelectorAlg::execute() {

    // Global filter originally false
    CP::SysFilterReporterCombiner filterCombiner (m_filterParams, false);

    // Loop over all systs
    for (const auto& sys : m_systematicsList.systematicsVector()) {
      CP::SysFilterReporter filter (filterCombiner, sys);

      // Retrive inputs
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK (m_eventHandle.retrieve (event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      const xAOD::JetContainer *lrjets = nullptr;
      ANA_CHECK (m_lrjetHandle.retrieve (lrjets, sys));

      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK (m_electronHandle.retrieve (electrons, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      const xAOD::MissingETContainer *metCont = nullptr;
      ANA_CHECK (m_metHandle.retrieve (metCont, sys));
      const xAOD::MissingET* met = (*metCont)["Final"];
      if (!met) {
      	ATH_MSG_ERROR("Could not retrieve MET");
      	return StatusCode::FAILURE;

      }

      bool hasOneLepton = false;
      bool minMet = false;
      bool hasTagJet = false;
      bool hasProbeJet = false;

      TLorentzVector lepton;
      float minDeltaR = std::numeric_limits<float>::max();
      const xAOD::Jet* tagjet = nullptr;
      const xAOD::Electron* electron = nullptr;
      const xAOD::Muon* muon = nullptr;

      // First selecting a single lepton from an event.
      if ( muons->size() == 1 && electrons->size() == 0 ) {
        muon = muons->at(0);
        lepton = muon->p4();
        hasOneLepton = true;
      } else if  ( electrons->size() == 1 && muons->size() == 0 ) {
        electron = electrons->at(0);
        lepton = electron->p4();
        hasOneLepton = true;
      }

      if ( hasOneLepton && met->met() > m_minMet) {
        minMet = true;
        for ( const xAOD::Jet *jet: *jets ) {
        float deltaR = lepton.DeltaR(jet->p4());
          if ( deltaR < minDeltaR ) {
            minDeltaR = deltaR;
            tagjet = jet;
          }
        }
      }

      if ( tagjet ) {
        hasTagJet = true;
        if ( !lrjets->empty() && std::abs(lrjets->at(0)->p4().DeltaPhi(tagjet->p4())) > 1.0 ) {
          hasProbeJet = true;
        }
      }

      bool passed_event = hasOneLepton && minMet && hasTagJet && hasProbeJet;
      if (!m_bypass && !passed_event) {
        continue;
      }
      filter.setPassed(true);
    }
    return StatusCode::SUCCESS;
  }

  StatusCode ttCalibSelectorAlg::finalize() {
    ANA_CHECK (m_filterParams.finalize());
    return StatusCode::SUCCESS;
  }
}
