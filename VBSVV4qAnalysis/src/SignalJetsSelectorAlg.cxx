/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

*/

#include "SignalJetsSelectorAlg.h"
 #include <AthenaKernel/Units.h>

namespace VBSVV4q{

    SignalJetsSelectorAlg :: SignalJetsSelectorAlg(const std::string& name, ISvcLocator *pSvcLocator)
                                         : EL::AnaAlgorithm(name, pSvcLocator){}

    StatusCode SignalJetsSelectorAlg::initialize(){

        ATH_MSG_INFO("*********************************\n");
        ATH_MSG_INFO("      SignalJetsSelectorAlg      \n");
        ATH_MSG_INFO("*********************************\n");

        // Read syst-aware input handles
        ATH_CHECK (m_SmallRJetsHandle.initialize(m_systematicsList));
        ATH_CHECK (m_LargeRJetsHandle.initialize(m_systematicsList));

        // Intialise syst-aware output decorators
        ATH_CHECK (m_SignalLargeRJetsOutHandle.initialize(m_systematicsList));

        if (!m_isBtag.empty()) {
            ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_SmallRJetsHandle));
        }

        // Intialise syst list (must come after all syst-aware inputs and outputs)
        ANA_CHECK (m_systematicsList.initialize());

        return StatusCode::SUCCESS;
    }

    StatusCode SignalJetsSelectorAlg::execute(){
        /*
            signal jets selection algorithm
                - leading two large-R jets with m > 40 GeV (pT/eta cuts already applied)
            ToDo: overlap removal with VBS jets
        */

        for (const auto& sys : m_systematicsList.systematicsVector()){
            // Retrieve inputs
            const xAOD::JetContainer *jets = nullptr;
            ANA_CHECK (m_LargeRJetsHandle.retrieve (jets, sys));

            auto SignalJetsCandidates = std::make_unique<ConstDataVector<xAOD::JetContainer> >(SG::VIEW_ELEMENTS);

	    for(auto jet : *jets){
                // skip large-R jets with mass below 40 GeV
                if(jet -> m() < 40.*Athena::Units::GeV) continue;
                SignalJetsCandidates -> push_back(jet);
            }

            ATH_CHECK(m_SignalLargeRJetsOutHandle.record(std::move(SignalJetsCandidates), sys));

        }//sys

        return StatusCode::SUCCESS;
    }
}
