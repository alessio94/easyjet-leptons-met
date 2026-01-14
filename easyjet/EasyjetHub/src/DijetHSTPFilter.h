/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TruthParticleInformationAlg.h
//
// This is an algorithm that will return a boolean for filtering
// dijet events, required for dijet MC
//
// Author: Jared Little, JaeJin Hong
///////////////////////////////////////////////////////////////////

// Always protect against multiple includes!
#ifndef EASYJET_DIJETHSTPFILTER
#define EASYJET_DIJETHSTPFILTER

#include <AthenaBaseComps/AthAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>


namespace Easyjet
{
    /// \brief An algorithm for normalizing dijet MC
  class DijetHSTPFilter final : public AthAlgorithm
  {
    /// \brief The standard constructor
public:
    DijetHSTPFilter(const std::string &name,
                                ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{
        this, "EventInfoKey", "EventInfo", "EventInfo container to dump"
    };
    SG::ReadHandleKey<xAOD::JetContainer> m_TruthHSJetKey{
        this, "TruthHSJetKey", "AntiKt4TruthDressedWZJets",
        "the truth Hard-Scattered jets"
    };
    SG::ReadHandleKey<xAOD::JetContainer> m_TruthPUJetKey{
        this, "TruthPUJetKey", "InTimeAntiKt4TruthJets",
        "the truth in time Pileup jets"
    };
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_PassHSTPKey{
        this, "PassHSTP", "EventInfo.PassHSTP", "Pass HSTP Filter for dijet"
    };
  };
}

#endif
