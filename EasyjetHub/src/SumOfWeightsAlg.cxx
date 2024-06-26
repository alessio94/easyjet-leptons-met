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
    m_total_mcEventWeight += eventWeights.at(m_weightIndex);
    m_total_mcEventWeight_squared += eventWeights.at(m_weightIndex)*eventWeights.at(m_weightIndex);

    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::EventInfo *evtInfo = nullptr;
      ATH_CHECK (m_eventHandle.retrieve (evtInfo, sys));
      auto weight = m_generatorWeight.get(*evtInfo,sys); // float. see PMGTruthWeightTool.h


      auto histIter = m_hist.find (sys);
      if (histIter == m_hist.end())
      {
        std::string name;
        ATH_CHECK (m_systematicsList.service().makeSystematicsName (name, m_histPattern, sys));

        std::string title = m_histTitle.value();
        if (!sys.empty())
          title += " (" + sys.name() + ")";
        ATH_CHECK (book (TH1F (name.c_str(), title.c_str(), 2, 0.5, 2.5)));

        m_hist.insert (std::make_pair (sys, hist (name)));
        histIter = m_hist.find (sys);
        assert (histIter != m_hist.end());

        histIter->second->GetXaxis()->SetBinLabel(1, "event count");
        histIter->second->GetXaxis()->SetBinLabel(2, "sum of weights");
      }

      histIter->second->Fill (1);
      histIter->second->Fill (2,weight);
    }

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
