/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "SumOfWeightsAlg.h"
#include <AsgDataHandles/ReadDecorHandle.h>

namespace Easyjet
{

  SumOfWeightsAlg::SumOfWeightsAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
  }


  StatusCode SumOfWeightsAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("      SumOfWeightsAlg      \n");
    ATH_MSG_INFO("*********************************\n");


    ATH_CHECK (m_eventInfoKey.initialize());
    ATH_CHECK (m_mcEventWeightsKey.initialize());
 
    ATH_CHECK (book (TH1F("SumOfWeights", "Sum of weights",2, 0.5, 2.5)));

    return StatusCode::SUCCESS;
  }


  StatusCode SumOfWeightsAlg::execute()
  {

    // Retrive inputs
    SG::ReadHandle<xAOD::EventInfo> event(m_eventInfoKey);
    ATH_CHECK (event.isValid());

    SG::ReadDecorHandle<xAOD::EventInfo, std::vector<float>> mcEventWeightsHandle(m_mcEventWeightsKey);
    std::vector<float> eventWeights = mcEventWeightsHandle(*event);

    m_total_mcEvent += 1;
    m_total_mcEventWeight += eventWeights.at(0);
    m_total_mcEventWeight_squared += eventWeights.at(0)*eventWeights.at(0);

    return StatusCode::SUCCESS;
  }

  StatusCode SumOfWeightsAlg::finalize()
  {
    hist("SumOfWeights")->SetBinContent(1,m_total_mcEvent);
    hist("SumOfWeights")->SetBinContent(2,m_total_mcEventWeight);
    hist("SumOfWeights")->SetBinError(2,std::sqrt(m_total_mcEventWeight_squared));

    return StatusCode::SUCCESS;

  }

}
