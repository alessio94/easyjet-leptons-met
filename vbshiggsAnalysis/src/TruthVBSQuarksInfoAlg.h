/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TruthVBSQuarksInfoAlg.h
//
// This is an algorithm that will retrieve the VBS quarks in the MC sample
//
// Author: Antonio Giannini
///////////////////////////////////////////////////////////////////

// Always protect against multiple includes!
#ifndef HHANALYSIS_TRUTHVBSQUARKSINFOALG
#define HHANALYSIS_TRUTHVBSQUARKSINFOALG

#include <AthenaBaseComps/AthAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODTruth/TruthParticleContainer.h>
#include "AthContainers/ConstDataVector.h"

namespace VBSHIGGS
{
    /// \brief An algorithm for dumping variables
  class TruthVBSQuarksInfoAlg final : public AthAlgorithm
  {
    /// \brief The standard constructor
public:
    TruthVBSQuarksInfoAlg(const std::string &name,
                                ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:

    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{
        this, "EventInfoKey", "EventInfo", "EventInfo container to dump"};

    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleInKey{
        this, "TruthParticleInKey", "",
        "the truth Standard Model particles container to run on"};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_nVBSQuarksKey{
        this, "nVBSQuarks", "EventInfo.nVBSQuarks", "Number of VBS quarks"};

    SG::WriteHandleKey<ConstDataVector<xAOD::TruthParticleContainer>>
        m_VBSQuarksKey{this, "VBSQuarksKey", "VBSQuarks", "Truth VBS quarks information container to write"};

    SG::WriteDecorHandleKey<xAOD::EventInfo> m_VBSQuark1pTKey{
        this, "VBSQuark1_pT", "EventInfo.VBSQuark1_pT", "VBS quark1 pT"};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_VBSQuark1etaKey{
        this, "VBSQuark1_eta", "EventInfo.VBSQuark1_eta", "VBS quark1 eta"};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_VBSQuark1phiKey{
        this, "VBSQuark1_phi", "EventInfo.VBSQuark1_phi", "VBS quark1 phi"};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_VBSQuark1EKey{
        this, "VBSQuark1_E", "EventInfo.VBSQuark1_E", "VBS quark1 E"};

    SG::WriteDecorHandleKey<xAOD::EventInfo> m_VBSQuark2pTKey{
        this, "VBSQuark2_pT", "EventInfo.VBSQuark2_pT", "VBS quark2 pT"};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_VBSQuark2etaKey{
        this, "VBSQuark2_eta", "EventInfo.VBSQuark2_eta", "VBS quark2 eta"};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_VBSQuark2phiKey{
        this, "VBSQuark2_phi", "EventInfo.VBSQuark2_phi", "VBS quark2 phi"};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_VBSQuark2EKey{
        this, "VBSQuark2_E", "EventInfo.VBSQuark2_E", "VBS quark2 E"};

};
}

#endif
