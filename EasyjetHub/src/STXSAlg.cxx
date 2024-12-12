/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Oleksii Kurdysh

//
// includes
//
#include "STXSAlg.h"

#include <AsgDataHandles/ReadHandle.h>
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
      m_twTools_prodmode = m_twTools->getProperty("ProdMode").toString();
    }

	m_HTXSBinDecorKey = m_EventInfoKey.key()+".HTXS_" + m_HTXSBin;
	ATH_CHECK(m_HTXSBinDecorKey.initialize());

	if (!m_twTools.empty()) {
	  m_HTXSWeightsDecorKey = m_EventInfoKey.key() + ".HTXS_" + m_HTXSWeights;
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
        std::vector<double> HTXS_w =  STXSWeights(*eventInfo,HTXS_Njets30,HTXS_Stage1,HTXS_pTH,HTXS_Stage1p2,HTXS_Stage1p2Fine);

        SG::WriteDecorHandle <xAOD::EventInfo, std::vector<double>> HTXSWeightsDecorHandle
            (m_HTXSWeightsDecorKey);
        HTXSWeightsDecorHandle(*eventInfo) = HTXS_w;
    }

    return StatusCode::SUCCESS;
  }

  std::tuple<int,int,float,int,int> STXSAlg::STXSInfo(
    const xAOD::EventInfo &eventInfo) const
  {
    int HTXS_Njets30 = -999;
    int HTXS_Stage1 = -999;
    float HTXS_pTH = -999.0;
    int HTXS_Stage1p2 = -999;
    int HTXS_Stage1p2Fine = -999;

    static const SG::AuxElement::ConstAccessor<int> acc_HTXS_Njets30("HTXS_Njets_pTjet30");
    static const SG::AuxElement::ConstAccessor<int> acc_HTXS_Stage1("HTXS_Stage1_Category_pTjet30");
    static const SG::AuxElement::ConstAccessor<float> acc_HTXS_pTH("HTXS_Higgs_pt"); // Needs to be in MeV
    static const SG::AuxElement::ConstAccessor<int> acc_HTXS_Stage1p2("HTXS_Stage1_2_Category_pTjet30");
    static const SG::AuxElement::ConstAccessor<int> acc_HTXS_Stage1p2Fine("HTXS_Stage1_2_Fine_Category_pTjet30");

    bool ok_acc_HTXS_Njets30 = acc_HTXS_Njets30.isAvailable(eventInfo);
    bool ok_acc_HTXS_Stage1 = acc_HTXS_Stage1.isAvailable(eventInfo);
    bool ok_acc_HTXS_pTH = acc_HTXS_pTH.isAvailable(eventInfo);
    bool ok_acc_HTXS_Stage1p2 = acc_HTXS_Stage1p2.isAvailable(eventInfo);
    bool ok_acc_HTXS_Stage1p2Fine = acc_HTXS_Stage1p2Fine.isAvailable(eventInfo);
    if (ok_acc_HTXS_Njets30 && ok_acc_HTXS_Stage1 && ok_acc_HTXS_pTH && ok_acc_HTXS_Stage1p2 && ok_acc_HTXS_Stage1p2Fine) {
      HTXS_Njets30 = acc_HTXS_Njets30(eventInfo);
      HTXS_Stage1 = acc_HTXS_Stage1(eventInfo);
      HTXS_pTH = acc_HTXS_pTH(eventInfo);
      HTXS_Stage1p2 = acc_HTXS_Stage1p2(eventInfo);
      HTXS_Stage1p2Fine = acc_HTXS_Stage1p2Fine(eventInfo);
    }
    ATH_MSG_DEBUG("++++ found HTXS_pTH (MeV) " << HTXS_pTH << " and bin " << HTXS_Stage1p2);
    return {HTXS_Njets30,HTXS_Stage1,HTXS_pTH,HTXS_Stage1p2,HTXS_Stage1p2Fine};
  }


  std::vector<double> STXSAlg::STXSWeights(
    const xAOD::EventInfo &eventInfo, int HTXS_Njets30, int HTXS_Stage1, float HTXS_pTH, int HTXS_Stage1p2, int HTXS_Stage1p2Fine)
  {
    std::vector<double> HTXS_w;
    TruthWeightTools::HiggsWeights hw = m_twTools->getHiggsWeights(HTXS_Njets30, HTXS_pTH, HTXS_Stage1, HTXS_Stage1p2, HTXS_Stage1p2Fine, &eventInfo);
    if (m_twTools_prodmode == "ggF") HTXS_w = hw.ggF_scheme;
    else if (m_twTools_prodmode == "VBF") HTXS_w = hw.qq2Hqq_scheme;
    else if ((m_twTools_prodmode == "qqZH") || (m_twTools_prodmode == "WH")) HTXS_w = hw.qq2Hll_scheme;
    else if (m_twTools_prodmode == "ggZH") HTXS_w = hw.gg2Hll_scheme;
    else if (m_twTools_prodmode == "ttH") HTXS_w = hw.ttH_scheme;
    for (const auto &w: HTXS_w) {
      ATH_MSG_DEBUG("++++ found HTXS weight (printing array) " << w);
    }
		
    return HTXS_w;
  }


}
