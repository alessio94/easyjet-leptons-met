/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

*/

#include "VBSJetsSelectorAlg.h"

namespace VBSHIGGS{
    VBSJetsSelectorAlg :: VBSJetsSelectorAlg(const std::string& name, ISvcLocator *pSvcLocator)
                                             : EL::AnaAlgorithm(name, pSvcLocator){}
                                           

    StatusCode VBSJetsSelectorAlg::initialize(){

        ATH_MSG_INFO("*********************************\n");
        ATH_MSG_INFO("      VBSJetsSelectorAlg         \n");
        ATH_MSG_INFO("*********************************\n");

        // Read syst-aware input handles
        ATH_CHECK (m_jetHandle.initialize(m_systematicsList));

        // Intialise syst-aware output decorators
        ATH_CHECK (m_VBSjetOutHandle.initialize(m_systematicsList));
        ATH_CHECK (m_NonVBsjetOutHandle.initialize(m_systematicsList));

        if (!m_isBtag.empty()) {
            ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
        }

        // Intialise syst list (must come after all syst-aware inputs and outputs)
        ANA_CHECK (m_systematicsList.initialize());

        return StatusCode::SUCCESS;
    }

    
    StatusCode VBSJetsSelectorAlg::execute(){

        /*
            1) First select back-to-back jets
            2) pick the pair with highest mjj 
            3) to be added mjj cut 
            4) Remaining jets are stored as signal jets and will be used for Higgs and W reconstructions
        */

        // Run over all systs
        for (const auto& sys : m_systematicsList.systematicsVector()){

            // Retrieve inputs
            const xAOD::JetContainer *jets = nullptr;
            ANA_CHECK (m_jetHandle.retrieve (jets, sys));
            
            auto VBSJetContainer = std::make_unique<ConstDataVector<xAOD::JetContainer> >(SG::VIEW_ELEMENTS);
            auto nonVBSJetContainer = std::make_unique<ConstDataVector<xAOD::JetContainer>>(
                jets->begin(), jets->end(), SG::VIEW_ELEMENTS);

            float maxMjj = 0;
            const xAOD::Jet* vbsJet1 = nullptr;
            const xAOD::Jet* vbsJet2 = nullptr;

            for(unsigned int i=0; i<jets->size(); i++){
                const xAOD::Jet* jet1 = jets->at(i);
                // non b-tagged jets
                if ( !m_isBtag.empty() && m_isBtag.get(*jet1, sys) ) continue;

                for(unsigned int j=0; j<i; j++){
                    const xAOD::Jet* jet2 = jets->at(j);
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

            std::sort(VBSJetContainer->begin(), VBSJetContainer->end(),
                [] (const xAOD::Jet* a,
                    const xAOD::Jet* b) {
                    return a->pt() > b->pt(); });

            //not erase VBS jets from the signal jet collection
            if (vbsJet1 && vbsJet2) {
                nonVBSJetContainer->erase( std::remove_if( nonVBSJetContainer->begin(), nonVBSJetContainer->end(),
                    [&](const xAOD::Jet* jet) -> bool { return jet == vbsJet1 || jet == vbsJet2 ;} ),
                    nonVBSJetContainer->end() );
            }
            
            ATH_CHECK(m_VBSjetOutHandle.record(std::move(VBSJetContainer), sys));
            ATH_CHECK(m_NonVBsjetOutHandle.record(std::move(nonVBSJetContainer), sys));

        }//syst

        return StatusCode::SUCCESS;
    }//execute
}//VBS namespace
