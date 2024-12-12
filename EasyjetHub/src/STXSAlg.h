/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TruthParticleInformationAlg.h
//
// Saves STXS category and maybe uncertanties depending on category
// if prod mode is the one tool doesn't understand tool won't be supplied in python and this is what isEmpty() checks
//
// Author: Oleksii Kurdysh
///////////////////////////////////////////////////////////////////

// Always protect against multiple includes!
#ifndef HHANALYSIS_STXSALG
#define HHANALYSIS_STXSALG

#include <AthenaBaseComps/AthAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>

#include <xAODEventInfo/EventInfo.h>

#include "TruthWeightTools/HiggsWeightTool.h"

namespace Easyjet
{
    /// \brief An algorithm for STXS uncertanties
  class STXSAlg final : public AthAlgorithm
  {
    /// \brief The standard constructor
public:
    STXSAlg(const std::string &name,
        ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:

    std::tuple<int,int,float,int,int> STXSInfo(const xAOD::EventInfo &eventInfo) const;

    std::vector<double> STXSWeights(const xAOD::EventInfo &eventInfo, int HTXS_Njets30, int HTXS_Stage1, float HTXS_pTH, int HTXS_Stage1p2, int HTXS_Stage1p2Fine);

    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{
        this, "EventInfoKey", "EventInfo", "EventInfo container to dump"};

    ToolHandle<TruthWeightTools::HiggsWeightTool> m_twTools{
        this, "twTools", "", "portal to HiggsWeightTool giving STXS uncert"};
    std::string m_twTools_prodmode;

    SG::WriteDecorHandleKey<xAOD::EventInfo> m_HTXSBinDecorKey;
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_HTXSWeightsDecorKey;

    std::string m_HTXSBin = "Category_Stage1_2_pTjet30";
    std::string m_HTXSWeights = "Weights_Stage1_2_pTjet30";

    };
}

#endif
