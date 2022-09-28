/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// DiHiggsAnalysisAlg.h
//
// This is an algorithm that will dump variables into a tree.
//
// Author: Victor Ruelas<victor.hugo.ruelas.rivera@cern.ch>
///////////////////////////////////////////////////////////////////

// Always protect against multiple includes!
#ifndef HH4BANALYSIS_DIHIGGSANALYSISALG
#define HH4BANALYSIS_DIHIGGSANALYSISALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <AthContainers/AuxElement.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>

namespace HH4B
{

  /// \brief An algorithm for dumping variables
  class DiHiggsAnalysisAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    DiHiggsAnalysisAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    // decorators for the Higgs variables
    std::unordered_map<std::string, SG::AuxElement::Decorator<float>>
        m_diHiggs_decos;
    // Member variables for configuration
    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{
        this, "EventInfoKey", "EventInfo", "EventInfo container to dump"};

    SG::ReadHandleKey<xAOD::JetContainer> m_SmallJetKey{
        this, "SmallJetKey", "", "the small-R jet collection to run on"};

    SG::ReadHandleKey<xAOD::JetContainer> m_LargeJetKey{
        this, "LargeJetKey", "", "the large-R jet collection to run on"};

    // if we have MC
    bool m_isMC;
    // whether the algorithm should do one of the following analyses
    bool m_doResolvedAnalysis;
    bool m_doBoostedAnalysis;
    // btagging working points for 0.4 jets
    std::vector<std::string> m_btag_wps;
    // btagging working points for the variable radius track jets in 1.0 jets
    std::vector<std::string> m_vr_btag_wps;
    // list of to be used Higgs vars
    std::vector<std::string> m_higgsVars;
  };
}

#endif
