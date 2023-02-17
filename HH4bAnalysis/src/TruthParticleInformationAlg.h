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

    Gaudi::Property<bool> m_filter {
      this, "filter", false, "Use this as an event filter"
    };

    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{
        this, "EventInfoKey", "EventInfo", "EventInfo container to dump"};

    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleInfoInKey{
        this, "TruthParticleInformationKey", "",
        "the truth information collection to run on"};

    SG::WriteHandleKey<ConstDataVector<xAOD::TruthParticleContainer>>
        m_truthParticleInfoOutKey{
            this, "TruthParticleInformationOutKey", "",
            "Truth particle information container to write"};

    std::vector<SG::AuxElement::Decorator<float>> m_selectionTruthH1Decorators;

    std::vector<SG::AuxElement::Decorator<float>> m_selectionTruthH2Decorators;

    std::vector<SG::AuxElement::Decorator<std::vector<float>>>
        m_selectionTruthBFromH1Decorators;

    std::vector<SG::AuxElement::Decorator<std::vector<float>>>
        m_selectionTruthBFromH2Decorators;

    SG::AuxElement::Decorator<int> m_truth_H1_pdgId{"truth_H1_pdgId"};

    SG::AuxElement::Decorator<int> m_truth_H2_pdgId{"truth_H2_pdgId"};

    std::vector<std::string> m_truthH1Vars{"truth_H1_pt", "truth_H1_eta",
                                           "truth_H1_phi", "truth_H1_m"};

    std::vector<std::string> m_truthH2Vars{"truth_H2_pt", "truth_H2_eta",
                                           "truth_H2_phi", "truth_H2_m"};

    std::vector<std::string> m_truthBFromH1Vars{
        "truth_b_fromH1_pt", "truth_b_fromH1_eta", "truth_b_fromH1_phi",
        "truth_b_fromH1_m"};

    std::vector<std::string> m_truthBFromH2Vars{
        "truth_b_fromH2_pt", "truth_b_fromH2_eta", "truth_b_fromH2_phi",
        "truth_b_fromH2_m"};
  };
}

#endif
