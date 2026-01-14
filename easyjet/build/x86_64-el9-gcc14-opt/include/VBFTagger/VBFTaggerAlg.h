/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Antonio Giannini

#ifndef VBFTaggerAlg_H
#define VBFTaggerAlg_H

#include "VBFTagger/IVBFTagger.h"

#include "AnaAlgorithm/AnaAlgorithm.h"
#include <AthContainers/ConstDataVector.h>
#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"

/// \brief an algorithm for calling \ref IVBFTagger

class VBFTaggerAlg final : public EL::AnaAlgorithm
{

    public:
        /// \brief the standard constructor
        VBFTaggerAlg(const std::string& name, ISvcLocator* pSvcLocator);

        StatusCode initialize() override;

        StatusCode execute() override;

    private:
        /// \brief the tool
        ToolHandle<IVBFTagger> m_vbftagger{
            this, "VBFTagger", "VBFTagger", "Tool implementing the VBF RNN score retrieve"};

        // key for event info container
        SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{
            this, "EventInfoKey", "EventInfo", "EventInfo container to dump"};

        // key for all jets
        SG::ReadHandleKey<ConstDataVector<xAOD::JetContainer>> m_containerAllJetsKey{
            this, "containerAllJetsKey", "", "containerName to read"};

        // key for signal jets
        SG::ReadHandleKey<ConstDataVector<xAOD::JetContainer>> m_containerSigJetsKey{
            this, "containerSigJetsKey", "", "containerName to read"};
        SG::ReadHandleKey<ConstDataVector<xAOD::JetContainer>> m_containerSigLargeRJetsKey{
            this, "containerSigLargeRJetsKey", "", "containerName to read"};

        // key for rnn jets
        SG::WriteHandleKey<ConstDataVector<xAOD::JetContainer>> m_RNNJetsDec{
            this, "RNNJetsDec", "", "rnn jets container to write"};

        /// \brief configuration
        Gaudi::Property<float> m_pTCut{this, "pTCut", 30.e3, "Minimum pT of jets to be used"};
        Gaudi::Property<float> m_nMaxJets{this, "nMaxJets", 2, "Maximum number of jets to be used"};
        Gaudi::Property<float> m_DRCut{this, "DRCut", 1.4, "DR thr between large-R and small-R jets"};
        Gaudi::Property<bool> m_OnlyFirstLargeRJet{this, "OnlyFirstLargeRJet", false, "Consider only the first large-R jet for the removal"};
        Gaudi::Property<std::string> m_DecTag{this, "DecTag", "", "Additional tag for decorator"};

};
#endif

