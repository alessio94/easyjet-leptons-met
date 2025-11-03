/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Derrick Allen
/// @author Jason Oliver - systematics, scale factors, handle aliasing
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

        ATH_CHECK (m_ttcalib_jetHandle.initialize(m_systematicsList));
        ATH_CHECK (m_ttcalib_lrjetHandle.initialize(m_systematicsList));
        ATH_CHECK (m_ttcalib_electronHandle.initialize(m_systematicsList));
        ATH_CHECK (m_ttcalib_muonHandle.initialize(m_systematicsList));
        ATH_CHECK (m_metHandle.initialize(m_systematicsList));

        m_eleWPDecorHandle = charHandle_t("baselineSelection_" + m_eleWPName+"_%SYS%", this);
        m_muonWPDecorHandle = charHandle_t("baselineSelection_"+m_muonWPName+"_%SYS%", this);

        // Intialise syst list (must come after all syst-aware inputs and outputs)
        ATH_CHECK (m_systematicsList.initialize());

        return StatusCode::SUCCESS;
    }


    StatusCode ttCalibSelectorAlg::execute() {

        // Global filter originally false
        CP::SysFilterReporterCombiner filterCombiner (m_filterParams, false);

        // Loop over all systs
        for (const auto& systematic : m_systematicsList.systematicsVector()) {
            CP::SysFilterReporter filter (filterCombiner, systematic);

            bool doEventSelection = !m_bypass;
            if(doEventSelection == false){
                filter.setPassed(true);
                continue;
            }


            // define objects to fill
            const xAOD::EventInfo *eventinfo = nullptr;
            const xAOD::JetContainer *jets = nullptr;
            const xAOD::JetContainer *lrjets = nullptr;
            const xAOD::ElectronContainer *electrons = nullptr;
            const xAOD::MuonContainer *muons = nullptr;
            const xAOD::MissingETContainer *metCont = nullptr;

            // retreive information from containers
            ANA_CHECK (m_eventHandle.retrieve (eventinfo, systematic));
            ANA_CHECK (m_ttcalib_jetHandle.retrieve (jets, systematic));
            ANA_CHECK (m_ttcalib_lrjetHandle.retrieve (lrjets, systematic));
            ANA_CHECK (m_ttcalib_electronHandle.retrieve (electrons, systematic));
            ANA_CHECK (m_ttcalib_muonHandle.retrieve (muons, systematic));
            ANA_CHECK (m_metHandle.retrieve (metCont, systematic));

            // post retreival checks - here is only a met check
            // in principle. we can do any checks
            const xAOD::MissingET* met = (*metCont)["Final"];
            if (!met) {
                ATH_MSG_ERROR("Could not retrieve MET");
                return StatusCode::FAILURE;

            }

            // define objects for event selection
            TLorentzVector lepton;
            const xAOD::Jet* tag_jet = nullptr;

            // define event selection booleans

            // met selections
            bool pass_minMET_selection =  met->met() > m_minMet;

            if ( !pass_minMET_selection) continue;

            // lepton selections
            int nMuons = muons->size();
            int nElectrons = electrons->size();
            int nLeptons = nMuons + nElectrons;

            bool is1LeptonEvent = (nLeptons == 1);

            if ( !is1LeptonEvent) continue;

            if ( nMuons==1 ) lepton = muons->at(0)->p4();
            else if  ( nElectrons==1) lepton = electrons->at(0)->p4();

            // initial jet selections
            int nLargeRJets = lrjets->size();

            int nCandidateTagJets = jets->size();

            if (nLargeRJets < 1) continue;
            if (nCandidateTagJets < 1) continue;

            // tag jet selection - find jet with minimum deltaR to lepton
            float minDeltaR = std::numeric_limits<float>::max();

            // find the minimum deltaR jet to lepton - define as tag jet
            for ( const xAOD::Jet *jet: *jets ) {
                float deltaR = lepton.DeltaR(jet->p4());
                if ( deltaR < minDeltaR ) {
                    minDeltaR = deltaR;
                    tag_jet = jet;

                }

            }

            if (! tag_jet) continue;

            // probe jet selection - deltaR(tag, largeRjet) > 1.0
            const xAOD::Jet* leadLargeRJet = lrjets->at(0);
            float deltaR_tag_largeR = tag_jet->p4().DeltaR(leadLargeRJet->p4());

            bool pass_deltaR_probe_selection = std::abs(deltaR_tag_largeR) > 1.0;
            if ( !pass_deltaR_probe_selection) continue;


            // if we reach this point, event has passed selection
            // it has one lepton, the minimum met, a tag jet and a probe jet
            filter.setPassed(true);
        }
        return StatusCode::SUCCESS;
    }

    StatusCode ttCalibSelectorAlg::finalize() {
        ANA_CHECK (m_filterParams.finalize());
        return StatusCode::SUCCESS;
    }
}
