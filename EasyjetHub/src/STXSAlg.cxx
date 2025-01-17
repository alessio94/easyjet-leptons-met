/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Oleksii Kurdysh

//
// includes
//
#include "STXSAlg.h"

#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

#include "TruthWeightTools/HiggsWeightTool.h"

//
// method implementations
//
namespace Easyjet
{
  STXSAlg ::STXSAlg(
    const std::string &name, ISvcLocator *pSvcLocator)
    : AthAlgorithm(name, pSvcLocator) { }

  StatusCode STXSAlg ::initialize()
  {
    ATH_MSG_DEBUG("Initialising " << name());

    ATH_CHECK(m_EventInfoKey.initialize());

    ATH_CHECK(m_twTools.retrieve( DisableTool{m_twTools.empty()} ));
    ATH_MSG_DEBUG("+++ twTools empty? " << m_twTools.empty());
    if (!m_twTools.empty()) {
      std::string twTools_prodmode = m_twTools->getProperty("ProdMode").toString();
      if(twTools_prodmode=="ggF") m_prodmode = STXSProdMode::ggF;
      else if(twTools_prodmode=="VBF") m_prodmode = STXSProdMode::VBF;
      else if(twTools_prodmode=="qqZH") m_prodmode = STXSProdMode::qqZH;
      else if(twTools_prodmode=="WH") m_prodmode = STXSProdMode::WH;
      else if(twTools_prodmode=="ggZH") m_prodmode = STXSProdMode::ggZH;
      else if(twTools_prodmode=="ttH") m_prodmode = STXSProdMode::ttH;
    }

    m_HTXS_Njets30_Key = m_EventInfoKey.key()+".HTXS_Njets_pTjet30";
    m_HTXS_Stage1_Key = m_EventInfoKey.key()+".HTXS_Stage1_Category_pTjet30";
    m_HTXS_pTH_Key = m_EventInfoKey.key()+".HTXS_Higgs_pt";
    m_HTXS_Stage1p2_Key = m_EventInfoKey.key()+".HTXS_Stage1_2_Category_pTjet30";
    m_HTXS_Stage1p2Fine_Key = m_EventInfoKey.key()+".HTXS_Stage1_2_Fine_Category_pTjet30";
    ATH_CHECK(m_HTXS_Njets30_Key.initialize());
    ATH_CHECK(m_HTXS_Stage1_Key.initialize());
    ATH_CHECK(m_HTXS_pTH_Key.initialize());
    ATH_CHECK(m_HTXS_Stage1p2_Key.initialize());
    ATH_CHECK(m_HTXS_Stage1p2Fine_Key.initialize());

    m_HTXSBinDecorKey = m_EventInfoKey.key()+".HTXS_Category_Stage1_2_pTjet30";
    ATH_CHECK(m_HTXSBinDecorKey.initialize());

    if (!m_twTools.empty()) {
      m_HTXSWeightsDecorKey = m_EventInfoKey.key() + ".HTXS_Weights_Stage1_2_pTjet30";
      ATH_CHECK(m_HTXSWeightsDecorKey.initialize());
    }

    return StatusCode::SUCCESS;
  }


  StatusCode STXSAlg ::execute()
  {
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    ATH_CHECK(eventInfo.isValid());

    int HTXS_Njets30;
    int HTXS_Stage1;
    float HTXS_pTH;
    int HTXS_Stage1p2;
    int HTXS_Stage1p2Fine;
    std::tie(HTXS_Njets30,HTXS_Stage1,HTXS_pTH,HTXS_Stage1p2,HTXS_Stage1p2Fine) = STXSInfo(*eventInfo);

    SG::WriteDecorHandle<xAOD::EventInfo, int> HTXSBinDecorHandle
        (m_HTXSBinDecorKey);
    HTXSBinDecorHandle(*eventInfo) = HTXS_Stage1p2;

    if(!m_twTools.empty()) {
        std::vector<double> HTXS_w = STXSWeights(*eventInfo,HTXS_Njets30,HTXS_Stage1,HTXS_pTH,HTXS_Stage1p2,HTXS_Stage1p2Fine);

        SG::WriteDecorHandle <xAOD::EventInfo, std::vector<double>> HTXSWeightsDecorHandle
            (m_HTXSWeightsDecorKey);
        HTXSWeightsDecorHandle(*eventInfo) = HTXS_w;
    }

    return StatusCode::SUCCESS;
  }

  std::tuple<int,int,float,int,int> STXSAlg::STXSInfo(
    const xAOD::EventInfo &eventInfo) const
  {
    SG::ReadDecorHandle<xAOD::EventInfo, int> HTXS_Njets30(m_HTXS_Njets30_Key);
    SG::ReadDecorHandle<xAOD::EventInfo, int> HTXS_Stage1(m_HTXS_Stage1_Key);
    SG::ReadDecorHandle<xAOD::EventInfo, float> HTXS_pTH(m_HTXS_pTH_Key);
    SG::ReadDecorHandle<xAOD::EventInfo, int> HTXS_Stage1p2(m_HTXS_Stage1p2_Key);
    SG::ReadDecorHandle<xAOD::EventInfo, int> HTXS_Stage1p2Fine(m_HTXS_Stage1p2Fine_Key);

    ATH_MSG_DEBUG("++++ found HTXS_pTH (MeV) " << HTXS_pTH << " and bin " << HTXS_Stage1p2);
    return {HTXS_Njets30(eventInfo), HTXS_Stage1(eventInfo), HTXS_pTH(eventInfo),
      HTXS_Stage1p2(eventInfo), HTXS_Stage1p2Fine(eventInfo)};
  }


  std::vector<double> STXSAlg::STXSWeights(
    const xAOD::EventInfo &eventInfo, int HTXS_Njets30, int HTXS_Stage1, float HTXS_pTH, int HTXS_Stage1p2, int HTXS_Stage1p2Fine)
  {
    std::vector<double> HTXS_w;
    TruthWeightTools::HiggsWeights hw = m_twTools->getHiggsWeights(HTXS_Njets30, HTXS_pTH, HTXS_Stage1, HTXS_Stage1p2, HTXS_Stage1p2Fine, &eventInfo);
    if (m_prodmode == STXSProdMode::ggF) HTXS_w = hw.ggF_scheme;
    else if (m_prodmode == STXSProdMode::VBF) HTXS_w = hw.qq2Hqq_scheme;
    else if (m_prodmode == STXSProdMode::qqZH ||
	     m_prodmode == STXSProdMode::WH) HTXS_w = hw.qq2Hll_scheme;
    else if (m_prodmode == STXSProdMode::ggZH) HTXS_w = hw.gg2Hll_scheme;
    else if (m_prodmode == STXSProdMode::ttH) HTXS_w = hw.ttH_scheme;
    for (const auto &w: HTXS_w) {
      ATH_MSG_DEBUG("++++ found HTXS weight (printing array) " << w);
    }
		
    return HTXS_w;
  }


}
