/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "KappaReweightingAlg.h"

#include "AthContainers/AuxElement.h"
#include <AthContainers/ConstDataVector.h>

namespace HHHBBBBTT
{

    KappaReweightingAlg::KappaReweightingAlg(const std::string &name,
        ISvcLocator *pSvcLocator)
    : AthHistogramAlgorithm(name, pSvcLocator)
    {

    }

    StatusCode KappaReweightingAlg::initialize()
    {

        // Reweight tool
        ANA_CHECK( m_wgtHandle.retrieve());
        ANA_MSG_DEBUG("Retrieved tool: " << m_wgtHandle);

        ATH_CHECK (m_EventHandleKey.initialize());
        
        for (const std::string &var : m_reweightVars){
            ATH_MSG_DEBUG("initializing float variable: " << var);
            m_Fbranches.emplace(std::piecewise_construct,
                std::forward_as_tuple(var),
                std::forward_as_tuple(this, var, "EventInfo." + var));
            ATH_CHECK(m_Fbranches.at(var).initialize());
        };

        return StatusCode::SUCCESS;
    }

    StatusCode KappaReweightingAlg::execute()
    {

        SG::ReadHandle<xAOD::EventInfo> event(m_EventHandleKey);
        ATH_CHECK(event.isValid());

        for (const std::string &var : m_reweightVars){
            SG::WriteDecorHandle<xAOD::EventInfo, double> 
                whandle(m_Fbranches.at(var));
            whandle(*event) = m_wgtHandle->getWeight(event.cptr(), var);
        };

        return StatusCode::SUCCESS;
    }
}