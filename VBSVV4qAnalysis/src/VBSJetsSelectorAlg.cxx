/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

*/

#include "VBSJetsSelectorAlg.h"

namespace VBSVV4q{
    VBSJetsSelectorAlg :: VBSJetsSelectorAlg(const std::string& name, ISvcLocator *pSvcLocator)
                                             : EL::AnaAlgorithm(name, pSvcLocator){}
                                           

    StatusCode VBSJetsSelectorAlg::initialize(){

        ATH_MSG_INFO("*********************************\n");
        ATH_MSG_INFO("      VBSJetsSelectorAlg         \n");
        ATH_MSG_INFO("*********************************\n");

        // Read syst-aware input handles
        ATH_CHECK (m_smallRjetHandle.initialize(m_systematicsList));
        ATH_CHECK (m_largeRjetHandle.initialize(m_systematicsList));

        // Intialise syst-aware output decorators
        ATH_CHECK (m_VBSjetOutHandle.initialize(m_systematicsList));
        ATH_CHECK (m_NonVBSjetOutHandle.initialize(m_systematicsList));

        if (!m_isBtag.empty()) {
            ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_smallRjetHandle));
        }

        // Intialise syst list (must come after all syst-aware inputs and outputs)
        ANA_CHECK (m_systematicsList.initialize());

        return StatusCode::SUCCESS;
    }

    
    StatusCode VBSJetsSelectorAlg::execute(){

        /*
            -) HighestMjjForwardJets : small-R jets pair among forward, no btag and opposite eta, with the highest mjj
            -) LeadingPTjj           : small-R jets pair with highest pT after overlap removal with signal large-R jets
        */

        // Run over all systs
        for (const auto& sys : m_systematicsList.systematicsVector()){

            // Retrieve inputs
            const xAOD::JetContainer *smallRjets = nullptr;
            ANA_CHECK (m_smallRjetHandle.retrieve (smallRjets, sys));

            const xAOD::JetContainer *largeRjets = nullptr;
            ANA_CHECK (m_largeRjetHandle.retrieve (largeRjets, sys));

            // make sure they are pT sorted
            //std::sort(smallRjets->begin(), smallRjets->end(), [] (const xAOD::Jet* a, const xAOD::Jet* b) {return a->pt() > b->pt(); });
            //std::sort(largeRjets->begin(), largeRjets->end(), [] (const xAOD::Jet* a, const xAOD::Jet* b) {return a->pt() > b->pt(); });

            auto VBSJetContainer = std::make_unique<ConstDataVector<xAOD::JetContainer> >(SG::VIEW_ELEMENTS);
            auto nonVBSJetContainer = std::make_unique<ConstDataVector<xAOD::JetContainer>>(
                smallRjets->begin(), smallRjets->end(), SG::VIEW_ELEMENTS);

            float maxMjj = 0;
            const xAOD::Jet* vbsJet1 = nullptr;
            const xAOD::Jet* vbsJet2 = nullptr;

            // tag jets pairing 
            if(m_TagJetsCriteria == "HighestMjjForwardJets"){
                for(unsigned int i=0; i<smallRjets->size(); i++){
                    const xAOD::Jet* jet1 = smallRjets->at(i);
                    // only forward jets (for now)
                    if ( abs(jet1->eta())<2.5 ) continue;

                    // non b-tagged jets
                    if ( !m_isBtag.empty() && m_isBtag.get(*jet1, sys) ) continue;

                    for(unsigned int j=0; j<i; j++){
                        const xAOD::Jet* jet2 = smallRjets->at(j);
        
                        if ( abs(jet2->eta())<2.5 ) continue;
        
                        if ( !m_isBtag.empty() && m_isBtag.get(*jet2, sys) ) continue;
                        
                        // select only back-to-back jets
                        if (jet1->eta() * jet2->eta() > 0 ) continue;

                        double mjj = (jet1->p4() + jet2->p4()).M();
                        if (mjj > maxMjj) {
                            maxMjj = mjj;
                            vbsJet1 = jet1;
                            vbsJet2 = jet2;
                        }
                    }
                }

                if (vbsJet1 && vbsJet2) {
                    VBSJetContainer->push_back(vbsJet1);
                    VBSJetContainer->push_back(vbsJet2);
                }

            }
            else if(m_TagJetsCriteria == "LeadingPTjj"){
                // loop over small-R jets
                for(auto srjet : *smallRjets){
                    bool passORJ = false;
                    // loop over signal large-R jets
                    for(auto lrjet : *largeRjets){
                        // skip overlapping jets
                        if( srjet->p4().DeltaR(lrjet->p4()) < m_DeltaRJj ) continue;
            			passORJ = true;
		            }

                    // store the tag jets
		            if(passORJ)
                        VBSJetContainer -> push_back(srjet);

                    // ToImprove
                    // up to two
                    if(VBSJetContainer->size() == 2) break;
                }
            }

            // sort VBS tag jets
            std::sort(VBSJetContainer->begin(), VBSJetContainer->end(),
                [] (const xAOD::Jet* a,
                    const xAOD::Jet* b) {
                    return a->pt() > b->pt(); });

            // now erase VBS jets from the signal jet collection
            if (vbsJet1 && vbsJet2) {
                nonVBSJetContainer->erase( std::remove_if( nonVBSJetContainer->begin(), nonVBSJetContainer->end(),
                    [&](const xAOD::Jet* jet) -> bool { return jet == vbsJet1 || jet == vbsJet2 ;} ),
                    nonVBSJetContainer->end() );
            }
            
            ATH_CHECK(m_VBSjetOutHandle.record(std::move(VBSJetContainer), sys));
            ATH_CHECK(m_NonVBSjetOutHandle.record(std::move(nonVBSJetContainer), sys));

        }//syst

        return StatusCode::SUCCESS;
    }//execute
}//VBS namespace
