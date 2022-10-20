/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TruthParticleInformationAlg.h
//
// This is an algorithm that will dump variables into a tree.
//
// Author: Victor Ruelas<victor.hugo.ruelas.rivera@cern.ch>
///////////////////////////////////////////////////////////////////

// Always protect against multiple includes!
#ifndef HH4BANALYSIS_TRUTHPARTICLEINFORMATIONALG
#define HH4BANALYSIS_TRUTHPARTICLEINFORMATIONALG

#include <AthenaBaseComps/AthAlgorithm.h>

#include "AthContainers/ConstDataVector.h"
#include <AthContainers/AuxElement.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODTruth/TruthParticleContainer.h>

// Class definition

namespace HH4B
{

  /// \brief An algorithm for dumping variables
  class TruthParticleInformationAlg final : public AthAlgorithm
  {
    /// \brief The standard constructor
public:
    TruthParticleInformationAlg(const std::string &name,
                                ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    StatusCode
    recordTruthParticleInformation(const xAOD::TruthParticleContainer &,
                                   const xAOD::EventInfo &) const;

    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{
        this, "EventInfoKey", "EventInfo", "EventInfo container to dump"};

    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleInfoInKey{
        this, "TruthParticleInformationKey", "",
        "the truth information collection to run on"};

    SG::WriteHandleKey<ConstDataVector<xAOD::TruthParticleContainer>>
        m_truthParticleInfoOutKey{
            this, "TruthParticleInformationOutKey", "",
            "Truth particle information container to write"};

    bool m_isMC;

    std::vector<SG::AuxElement::Decorator<float>> m_selectionTruthHDecorators;

    std::vector<SG::AuxElement::Decorator<float>> m_selectionTruthSDecorators;

    std::vector<SG::AuxElement::Decorator<std::vector<float>>>
        m_selectionTruthBFromHDecorators;

    std::vector<SG::AuxElement::Decorator<std::vector<float>>>
        m_selectionTruthBFromSDecorators;

    std::vector<std::string> m_truthHVars{"truth_H_pt", "truth_H_eta",
                                          "truth_H_phi", "truth_H_m"};

    std::vector<std::string> m_truthSVars{"truth_S_pt", "truth_S_eta",
                                          "truth_S_phi", "truth_S_m"};

    std::vector<std::string> m_truthBFromHVars{
        "truth_b_fromH_pt", "truth_b_fromH_eta", "truth_b_fromH_phi",
        "truth_b_fromH_m"};

    std::vector<std::string> m_truthBFromSVars{
        "truth_b_fromS_pt", "truth_b_fromS_eta", "truth_b_fromS_phi",
        "truth_b_fromS_m"};
  };
}

#endif
