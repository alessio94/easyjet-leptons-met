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
        if(m_loadDisCoJet) ATH_CHECK (m_discojet.initialize(m_systematicsList, m_LargeRJetsHandle));
        // Intialise syst list (must come after all syst-aware inputs and outputs)
        ANA_CHECK (m_systematicsList.initialize());

        return StatusCode::SUCCESS;
    }

    StatusCode SignalJetsSelectorAlg::execute(){
        /*
            signal jets selection algorithm
                - two leading pT large-R jets with m > 40 GeV (pT/eta cuts already applied)
                - two leading tagger score large-R jets with m > 40 GeV (pT/eta cuts already applied)
            ToDo: overlap removal with VBS jets
        */

        for (const auto& sys : m_systematicsList.systematicsVector()){
            // Retrieve inputs
            const xAOD::JetContainer *jets = nullptr;
            ANA_CHECK (m_LargeRJetsHandle.retrieve (jets, sys));
            auto SignalJetsCandidates = std::make_unique<ConstDataVector<xAOD::JetContainer> >(SG::VIEW_ELEMENTS);
            // init for discojet score and candidate signal jets
            float discojet1 = -99., discojet2 = -99., discojet = -99.;
            const xAOD::Jet* Jet1 = nullptr;
            const xAOD::Jet* Jet2 = nullptr;
            for(auto jet : *jets){
                // skip large-R jets with mass below 40 GeV
                if(jet -> m() < 40.*Athena::Units::GeV) continue;
                if(m_SigJetsCriteria == "HighestPT"){ // pT-order criteria, select the 2 leading pT jets passing jet mass cut.
                    if (!Jet1) {
                        Jet1 = jet;
                        continue;
                    }
                    if (!Jet2) {
                        Jet2 = jet;
                        break;
                    }
                }
                if(m_SigJetsCriteria == "HighestScore" && m_loadDisCoJet){ // score-order criteria, select the 2 jets with highest discojet scores passing mass cut.
                    discojet = m_discojet.get(*jet, sys);
                    if (discojet > discojet1) {
                        discojet2 = discojet1;
                        discojet1 = discojet;
                        Jet2 = Jet1;
                        Jet1 = jet;
                    }
                    else if (discojet > discojet2){
                        discojet2 = discojet;
                        Jet2 = jet;
                    }
                }
            }
            if (Jet1 && Jet2){ // sort the two candidates by pT.
                if (Jet1->pt() > Jet2->pt()){
                    SignalJetsCandidates -> push_back(Jet1);
                    SignalJetsCandidates -> push_back(Jet2);
                }
                else {
                    SignalJetsCandidates -> push_back(Jet2);
                    SignalJetsCandidates -> push_back(Jet1);
                } 
            }

            ATH_CHECK(m_SignalLargeRJetsOutHandle.record(std::move(SignalJetsCandidates), sys));

        }//sys

        return StatusCode::SUCCESS;
    }
}
