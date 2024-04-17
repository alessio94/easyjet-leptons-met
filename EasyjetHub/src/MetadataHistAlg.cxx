/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MetadataHistAlg.h"

namespace Easyjet {

  MetadataHistAlg::MetadataHistAlg(const std::string &name,
                                ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode MetadataHistAlg::initialize() 
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("      MetadataHistAlg      \n");
    ATH_MSG_INFO("*********************************\n");

    if (m_dataType.value().empty()) {
      ATH_MSG_ERROR("Missing data type information");
      return StatusCode::FAILURE;
    }
    if (m_mcCampaign.value().empty()) {
      ATH_MSG_ERROR("Missing mc campaign");
      return StatusCode::FAILURE;
    }
    if (m_mcChannelNumber.value().empty()) {
      ATH_MSG_ERROR("Missing mc channel number");
      return StatusCode::FAILURE;
    }

    ATH_CHECK (book (TH1I("metadata", "Sample metadata", 3, 0.5, 3.5)));

    return StatusCode::SUCCESS;
  }

  StatusCode MetadataHistAlg::execute()
  {
    return StatusCode::SUCCESS;
  }

  StatusCode MetadataHistAlg::finalize()
  {

    // Extracting the desired part of the string
    std::string mcCampaignValue = m_mcCampaign.value();
    size_t dotPos = mcCampaignValue.find('.');
    if (dotPos != std::string::npos) {
        mcCampaignValue = mcCampaignValue.substr(dotPos + 1);
    }
    // Replace MC by mc
    mcCampaignValue.replace(0, 2, "mc");

    // Please keep the bin order unchanged
    hist("metadata")->GetXaxis()->SetBinLabel(1, m_dataType.value().c_str());
    hist("metadata")->GetXaxis()->SetBinLabel(2, mcCampaignValue.c_str());
    hist("metadata")->GetXaxis()->SetBinLabel(3, m_mcChannelNumber.value().c_str());

    return StatusCode::SUCCESS;
  }
}
