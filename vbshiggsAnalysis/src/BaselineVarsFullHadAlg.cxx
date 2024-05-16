/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "BaselineVarsFullHadAlg.h"
#include "AthContainers/AuxElement.h"
#include "TLorentzVector.h"


namespace VBSHIGGS{
    BaselineVarsFullHadAlg::BaselineVarsFullHadAlg(const std::string &name,
                                            ISvcLocator *pSvcLocator)
    : AthHistogramAlgorithm(name, pSvcLocator){

    }

    StatusCode BaselineVarsFullHadAlg::initialize(){
        ATH_MSG_INFO("*********************************\n");
        ATH_MSG_INFO("       FullHadBaselineVarsAlg    \n");
        ATH_MSG_INFO("*********************************\n");

        // Read syst-aware input handles
        ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
        ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

       
        if (!m_isBtag.empty()) {
            ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
        }

        // Intialise syst-aware output decorators
        for (const std::string &var : m_floatVariables) {
            CP::SysWriteDecorHandle<float> whandle{var+"_%SYS%", this};
            m_Fbranches.emplace(var, whandle);
            ATH_CHECK (m_Fbranches.at(var).initialize(m_systematicsList, m_eventHandle));
        }

        for (const std::string &var : m_intVariables){
            ATH_MSG_DEBUG("initializing integer variable: " << var);
            CP::SysWriteDecorHandle<int> whandle{var+"_%SYS%", this};
            m_Ibranches.emplace(var, whandle);
            ATH_CHECK(m_Ibranches.at(var).initialize(m_systematicsList, m_eventHandle));
        };

        // Intialise syst list (must come after all syst-aware inputs and outputs)
        ATH_CHECK (m_systematicsList.initialize());
        return StatusCode::SUCCESS;
    }

    StatusCode BaselineVarsFullHadAlg::execute(){
        // Loop over all systs
        for (const auto& sys : m_systematicsList.systematicsVector()){

            // Retrieve inputs
            const xAOD::EventInfo *event = nullptr;
            ANA_CHECK (m_eventHandle.retrieve (event, sys));

            const xAOD::JetContainer *jets = nullptr;
            ANA_CHECK (m_jetHandle.retrieve (jets, sys));

            
            for (const std::string &string_var: m_floatVariables) {
                m_Fbranches.at(string_var).set(*event, -99., sys);
            }

            for (const auto& var: m_intVariables) {
                m_Ibranches.at(var).set(*event, -99, sys);
            }

        }
        return StatusCode::SUCCESS;
    } 
}