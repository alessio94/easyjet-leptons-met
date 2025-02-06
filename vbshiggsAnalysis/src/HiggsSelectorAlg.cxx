/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

*/

#include "HiggsSelectorAlg.h"

namespace VBSHIGGS{
    HiggsSelectorAlg :: HiggsSelectorAlg(const std::string& name, ISvcLocator *pSvcLocator)
                                         : EL::AnaAlgorithm(name, pSvcLocator){}

    StatusCode HiggsSelectorAlg::initialize(){

        ATH_MSG_INFO("*********************************\n");
        ATH_MSG_INFO("      HiggsSelectorAlg           \n");
        ATH_MSG_INFO("*********************************\n");

        // Read syst-aware input handles
        ATH_CHECK (m_jetHandle.initialize(m_systematicsList));

        // Intialise syst-aware output decorators
        ATH_CHECK (m_HiggsjetOutHandle.initialize(m_systematicsList));

        if (!m_isBtag.empty()) {
            ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
        }

        // Intialise syst list (must come after all syst-aware inputs and outputs)
        ANA_CHECK (m_systematicsList.initialize());

        return StatusCode::SUCCESS;
    }

    StatusCode HiggsSelectorAlg::execute(){
        /*
            Higgs selection algorithm
            1) First select leading 2 b-jets
            2) if only 1 b-jet present  select it with leading light-jet
            3) if no b-jet found select leading 2 light-jets
        */

        for (const auto& sys : m_systematicsList.systematicsVector()){
            // Retrieve inputs
            const xAOD::JetContainer *jets = nullptr;
            ANA_CHECK (m_jetHandle.retrieve (jets, sys));

            auto HiggsJetCandidates= std::make_unique<ConstDataVector<xAOD::JetContainer> >(SG::VIEW_ELEMENTS);

            const xAOD::Jet* jet1 = nullptr; // higher pt
            const xAOD::Jet* jet2 = nullptr; // lower pt
            const xAOD::Jet* bJet1 = nullptr; // higher pt
            const xAOD::Jet* bJet2 = nullptr; // lower pt
            unsigned nBjets = 0;

            for(unsigned int i=0; i<jets->size(); i++){
                const xAOD::Jet* jet = jets->at(i);
                if (std::abs(jet->eta())>=2.5) continue;

                //Highest pT all jets
                if (!jet1) jet1 = jet;
                else if (!jet2) jet2 = jet;

                // Highest b-jets
                if ( !m_isBtag.empty() && m_isBtag.get(*jet, sys) ){
                    nBjets++;
                    if (!bJet1) bJet1 = jet;
                    else if (!bJet2 && jet != bJet1 ) bJet2 = jet;

                }
            }
            // Set H->bb candidates 
            const xAOD::Jet* hjet1 = nullptr; // higher pt
            const xAOD::Jet* hjet2 = nullptr; // lower pt

            // always set to leading jets
            if (jet1 && jet2){ 
                hjet1 = jet1;
                hjet2 = jet2;
            }
            
            //select leading 2 b-jet 
            if (nBjets >= 2) {
                hjet1 = bJet1;
                hjet2 = bJet2;
            }

            //select leading 1 b-jet with leading light-jet
            if ( nBjets == 1 ){
                if (hjet1 != bJet1) hjet2 = bJet1;
            }
            if (hjet1 && hjet2 && nBjets >0) {
                HiggsJetCandidates->push_back(hjet1);
                HiggsJetCandidates->push_back(hjet2);
            }
            ATH_CHECK(m_HiggsjetOutHandle.record(std::move(HiggsJetCandidates), sys));
        }//sys

        return StatusCode::SUCCESS;
    }
}