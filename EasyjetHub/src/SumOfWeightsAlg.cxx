/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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

    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK (m_generatorWeight.initialize(m_systematicsList, m_eventHandle));
    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize()); 


    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      std::string name;
      ATH_CHECK (m_systematicsList.service().makeSystematicsName (name, m_histPattern, sys));

      std::string title = m_histTitle.value();
      if (!sys.empty())
        title += " (" + sys.name() + ")";
      ATH_CHECK (book (TH1F (name.c_str(), title.c_str(), 2, 0.5, 2.5)));

      m_syst_info_vec.emplace_back();
      SystInfo& syst_info = m_syst_info_vec.back();
      syst_info.hist.reset(hist(name));
      syst_info.hist->GetXaxis()->SetBinLabel(1, "event count");
      syst_info.hist->GetXaxis()->SetBinLabel(2, "sum of weights"); 
    }

    return StatusCode::SUCCESS;
  }


  StatusCode SumOfWeightsAlg::execute()
  {

    // Retrive inputs
    SG::ReadHandle<xAOD::EventInfo> event(m_eventInfoKey);
    ATH_CHECK (event.isValid());

    for (size_t iSyst{0}; const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::EventInfo *evtInfo = nullptr;
      ATH_CHECK (m_eventHandle.retrieve (evtInfo, sys));
      auto weight = m_generatorWeight.get(*evtInfo,sys); // float. see PMGTruthWeightTool.h

      SystInfo& syst_info = m_syst_info_vec[iSyst];
      syst_info.total_mcEvents += 1;
      syst_info.total_mcEventWeight += weight;
      syst_info.total_mcEventWeight_squared += weight*weight;
      ++iSyst;
    }

    return StatusCode::SUCCESS;
  }

  StatusCode SumOfWeightsAlg::finalize()
  {

    // fill sys to hist here
    for (const SystInfo& syst_info : m_syst_info_vec)
    {
      syst_info.hist->SetBinContent(1,syst_info.total_mcEvents);
      syst_info.hist->SetBinContent(2,syst_info.total_mcEventWeight);
      syst_info.hist->SetBinError(2,std::sqrt(syst_info.total_mcEventWeight_squared));
    }


    return StatusCode::SUCCESS;

  }

}
