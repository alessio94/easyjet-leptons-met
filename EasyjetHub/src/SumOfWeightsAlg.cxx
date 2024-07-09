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
    ATH_CHECK (m_mcEventWeightsKey.initialize());

    ATH_CHECK (m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK (m_generatorWeight.initialize(m_systematicsList, m_eventHandle));
    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize()); 

    ATH_CHECK (book (TH1F("SumOfWeights_special", "Sum of weights",2, 0.5, 2.5)));

    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      auto histIter = m_hist_sys.find (sys);
      if (histIter == m_hist_sys.end())
      {
        std::string name;
        ATH_CHECK (m_systematicsList.service().makeSystematicsName (name, m_histPattern, sys));

        std::string title = m_histTitle.value();
        if (!sys.empty())
          title += " (" + sys.name() + ")";
        ATH_CHECK (book (TH1F (name.c_str(), title.c_str(), 2, 0.5, 2.5)));

        m_hist_sys.insert (std::make_pair (sys, hist (name)));
 	m_total_mcEvent_sys.insert (std::make_pair (sys, 0));
        m_total_mcEventWeight_sys.insert (std::make_pair (sys, 0.0));
        m_total_mcEventWeight_squared_sys.insert (std::make_pair (sys, 0.0));
        histIter = m_hist_sys.find (sys);
        assert (histIter != m_hist_sys.end());

        histIter->second->GetXaxis()->SetBinLabel(1, "event count");
        histIter->second->GetXaxis()->SetBinLabel(2, "sum of weights");
      } 
    }

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
    m_total_mcEventWeight += eventWeights.at(m_weightIndex);
    m_total_mcEventWeight_squared += eventWeights.at(m_weightIndex)*eventWeights.at(m_weightIndex);

    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::EventInfo *evtInfo = nullptr;
      ATH_CHECK (m_eventHandle.retrieve (evtInfo, sys));
      auto weight = m_generatorWeight.get(*evtInfo,sys); // float. see PMGTruthWeightTool.h

      auto cIter = m_total_mcEvent_sys.find (sys);
      cIter->second += 1;
      auto wIter = m_total_mcEventWeight_sys.find (sys);
      wIter->second += weight;
      auto w2Iter = m_total_mcEventWeight_squared_sys.find (sys);
      w2Iter->second += weight*weight;
    }

    return StatusCode::SUCCESS;
  }

  StatusCode SumOfWeightsAlg::finalize()
  {
    hist("SumOfWeights_special")->SetBinContent(1,m_total_mcEvent);
    hist("SumOfWeights_special")->SetBinContent(2,m_total_mcEventWeight);
    hist("SumOfWeights_special")->SetBinError(2,std::sqrt(m_total_mcEventWeight_squared));

    // fill sys to hist here
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      auto histIter = m_hist_sys.find (sys);
      auto cIter = m_total_mcEvent_sys.find (sys);
      histIter->second->SetBinContent(1,cIter->second);
      auto wIter = m_total_mcEventWeight_sys.find (sys);
      histIter->second->SetBinContent(2,wIter->second);
      auto w2Iter = m_total_mcEventWeight_squared_sys.find (sys);
      histIter->second->SetBinError(2,std::sqrt(w2Iter->second));
    }


    return StatusCode::SUCCESS;

  }

}
