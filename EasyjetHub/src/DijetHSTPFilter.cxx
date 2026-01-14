/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Jared Little, JaeJin Hong

//
// includes
//
#include "DijetHSTPFilter.h"

#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>


//
// method implementations
//
namespace Easyjet
{
  DijetHSTPFilter ::DijetHSTPFilter(
    const std::string &name, ISvcLocator *pSvcLocator)
    : AthAlgorithm(name, pSvcLocator) { }

  StatusCode DijetHSTPFilter ::initialize()
  {
    ATH_MSG_DEBUG("Initializing " << name());
    ATH_CHECK(m_EventInfoKey.initialize());
    ATH_CHECK(m_TruthHSJetKey.initialize());
    ATH_CHECK(m_TruthPUJetKey.initialize());
    ATH_CHECK(m_PassHSTPKey.initialize());

    return StatusCode::SUCCESS;

  }


  StatusCode DijetHSTPFilter ::execute(){

    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    ATH_CHECK(eventInfo.isValid());

    SG::ReadHandle<xAOD::JetContainer> truthHS_jets(
        m_TruthHSJetKey);
    ATH_CHECK(truthHS_jets.isValid());
    SG::ReadHandle<xAOD::JetContainer> truthPU_jets(
        m_TruthPUJetKey);
    ATH_CHECK(truthPU_jets.isValid());

    bool pass_HSTP_filter = true;

    // Extract the pT of the leading jet that defines the hardness. The jet containers should always be pT sorted
    double pT_j1_truthPU = 0;
    double pT_j1_truthHS = 5000; // In the rare case of no HS truth jets in the event, assume it is close to the 5 GeV threshold
    if ( truthHS_jets->size() ) pT_j1_truthHS = truthHS_jets->at(0)->pt(); // Hardest HS truth jet
    if ( truthPU_jets->size() ) pT_j1_truthPU = truthPU_jets->at(0)->pt(); // Hardest PU truth jet

    // Now see if we pass the filter
    pass_HSTP_filter = pT_j1_truthHS > pT_j1_truthPU;

    // Update the user-defined variable in EventInfo
    SG::WriteDecorHandle<xAOD::EventInfo, bool> HSTPFilterDecorHandle(m_PassHSTPKey);
    HSTPFilterDecorHandle(*eventInfo) = pass_HSTP_filter;

    return StatusCode::SUCCESS;

  }

}
