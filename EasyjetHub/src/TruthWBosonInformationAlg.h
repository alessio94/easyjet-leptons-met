/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TruthParticleInformationAlg.h
//
// This is an algorithm that will calculate the ttbar parton history
//
// Author: Jordy Degens, Osama Karkout
///////////////////////////////////////////////////////////////////

// Always protect against multiple includes!
#ifndef HHANALYSIS_TRUTHWBOSONINFORMATIONALG
#define HHANALYSIS_TRUTHWBOSONINFORMATIONALG

#include "AthContainers/ConstDataVector.h"
#include <AthContainers/AuxElement.h>
#include <AthenaBaseComps/AthAlgorithm.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODTruth/TruthParticleContainer.h>
#include <AsgDataHandles/WriteDecorHandle.h>


namespace Easyjet
{
    /// \brief An algorithm for dumping variables
  class TruthWBosonInformationAlg final : public AthAlgorithm
  {
    /// \brief The standard constructor
public:
    TruthWBosonInformationAlg(const std::string &name,
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

    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthBosonParticleInKey{
        this, "TruthBosonParticleInKey", "",
        "the truth Standard Model particles container to run on"};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_nWLepKey{
        this, "nWLep", "EventInfo.nWLep", "Number of leptonic W decays"};

};
}

#endif