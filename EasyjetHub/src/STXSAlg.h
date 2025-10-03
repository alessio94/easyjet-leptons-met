/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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

  enum STXSProdMode {
    UnDef = -1,
    ggF = 0,
    VBF = 1,
    qqZH = 2,
    WH = 3,
    ggZH = 4,
    ttH = 5
  };


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

    std::tuple<int,int,float,int,int,int,int> STXSInfo(const xAOD::EventInfo &eventInfo) const;

    std::vector<double> STXSWeights(const xAOD::EventInfo &eventInfo, int HTXS_Njets30, int HTXS_Stage1, float HTXS_pTH, int HTXS_Stage1p2, int HTXS_Stage1p2Fine);

    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{
        this, "EventInfoKey", "EventInfo", "EventInfo container to dump"};

    ToolHandle<TruthWeightTools::HiggsWeightTool> m_twTools{
        this, "twTools", "", "portal to HiggsWeightTool giving STXS uncert"};
    STXSProdMode m_prodmode = STXSProdMode::UnDef;

    SG::ReadDecorHandleKey<xAOD::EventInfo> m_HTXS_pTH_Key;
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_HTXS_Stage1_pTjet30_Key;
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_HTXS_Njets_pTjet30_Key;
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_HTXS_Stage1p2_pTjet30_Key;
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_HTXS_Stage1p2Fine_pTjet30_Key;
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_HTXS_Stage1p2_pTjet25_Key;
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_HTXS_Stage1p2Fine_pTjet25_Key;

    SG::WriteDecorHandleKey<xAOD::EventInfo> m_HTXSBin_pTjet30_DecorKey;
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_HTXSFineBin_pTjet30_DecorKey;
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_HTXSBin_pTjet25_DecorKey;
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_HTXSFineBin_pTjet25_DecorKey;
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_HTXSWeightsDecorKey;

    Gaudi::Property<bool> m_has_STXS{
      this, "has_STXS", false, "Is this a sample identified in .cfg as the one that should have STXS bin info?"};

  };
}

#endif
