/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AthContainers/AuxElement.h"
#include "BaselineVarsMultileptonAlg.h"
#include <AthContainers/ConstDataVector.h>

#include "TLorentzVector.h"
#include "EasyjetHub/MT2_ROOT.h"

namespace MULTILEPTON
{
  BaselineVarsMultileptonAlg::BaselineVarsMultileptonAlg(const std::string &name,
                                           ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {

  }

  StatusCode BaselineVarsMultileptonAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("   BaselineVarsMultileptonAlg    \n");
    ATH_MSG_INFO("*********************************\n");

    // Read syst-aware input handles
    ATH_CHECK (m_jetHandle.initialize(m_systematicsList));
    ATH_CHECK (m_electronHandle.initialize(m_systematicsList));
    ATH_CHECK (m_muonHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));

    if(m_isMC){
      m_ele_SF = CP::SysReadDecorHandle<float>("el_effSF_"+m_eleWPName+"_%SYS%", this);
    }
    ATH_CHECK (m_ele_SF.initialize(m_systematicsList, m_electronHandle, SG::AllowEmpty));

    if(m_isMC){
      m_mu_SF = CP::SysReadDecorHandle<float>("muon_effSF_"+m_muWPName+"_%SYS%", this);
    }
    ATH_CHECK (m_mu_SF.initialize(m_systematicsList, m_muonHandle, SG::AllowEmpty));

    // Intialise syst-aware output decorators
    for (const std::string &var : m_floatVariables) {
      CP::SysWriteDecorHandle<float> whandle{var+"_%SYS%", this};
      m_Fbranches.emplace(var, whandle);
      ATH_CHECK (m_Fbranches.at(var).initialize(m_systematicsList, m_eventHandle));
    }

    for (const std::string &var : m_intVariables){
      CP::SysWriteDecorHandle<int> whandle{var+"_%SYS%", this};
      m_Ibranches.emplace(var, whandle);
      ATH_CHECK(m_Ibranches.at(var).initialize(m_systematicsList, m_eventHandle));
    };

    if (!m_isBtag.empty()) {
      ATH_CHECK (m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    ATH_CHECK (m_met_sig.initialize(m_systematicsList, m_metHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());


    return StatusCode::SUCCESS;
  }

  StatusCode BaselineVarsMultileptonAlg::execute()
  {
    return StatusCode::SUCCESS;
  }

}


