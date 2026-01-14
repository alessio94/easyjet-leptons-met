/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Antonio Giannini

#ifndef VBFTaggerAlgSys_H
#define VBFTaggerAlgSys_H

#include "VBFTagger/IVBFTagger.h"

#include "AnaAlgorithm/AnaAlgorithm.h"
#include <AthContainers/ConstDataVector.h>
#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

/// \brief an algorithm for calling \ref IVBFTagger

class VBFTaggerAlgSys final : public EL::AnaAlgorithm
{

    public:
        /// \brief the standard constructor
        VBFTaggerAlgSys(const std::string& name, ISvcLocator* pSvcLocator);

        StatusCode initialize() override;

        StatusCode execute() override;

    private:
        /// \brief the tool
        ToolHandle<IVBFTagger> m_vbftagger{
            this, "VBFTagger", "VBFTagger", "Tool implementing the VBF RNN score retrieve"};

        /// \brief Setup syst-aware input container handles
        CP::SysListHandle m_systematicsList {this};

        CP::SysReadHandle<xAOD::EventInfo> m_EventInfoKey{ 
            this, "EventInfoKey", "EventInfo", "EventInfo container to read"};

        CP::SysReadHandle<xAOD::JetContainer> m_containerAllJetsKey{ 
            this, "containerAllJetsKey", "", "containerName to read"};

        CP::SysReadHandle<xAOD::JetContainer> m_containerSigJetsKey{ 
            this, "containerSigJetsKey", "", "containerName to read"};
        CP::SysReadHandle<xAOD::JetContainer> m_containerSigLargeRJetsKey{ 
            this, "containerSigLargeRJetsKey", "", "containerName to read"};

        /// \brief Setup syst-aware output decorator handles
        CP::SysWriteDecorHandle<float> m_RNNScoreDec{
            this, "RNNScoreDec", "", "decorator name for RNN score"};
        CP::SysWriteDecorHandle<float> m_nRNNJetsDec{
            this, "nRNNJetsDec", "", "decorator name for nRNN jets"};
        CP::SysWriteHandle<ConstDataVector<xAOD::JetContainer>> m_RNNJetsDec{
            this, "RNNJetsDec", "", "rnn jets container to write"};

        /// \brief configuration
        Gaudi::Property<float> m_pTCut{this, "pTCut", 30.e3, "Minimum pT of jets to be used"};
        Gaudi::Property<float> m_nMaxJets{this, "nMaxJets", 2, "Maximum number of jets to be used"};
        Gaudi::Property<float> m_DRCut{this, "DRCut", 1.4, "DR thr between large-R and small-R jets"};
        Gaudi::Property<bool> m_OnlyFirstLargeRJet{this, "OnlyFirstLargeRJet", false, "Consider only the first large-R jet for the removal"};
        Gaudi::Property<std::string> m_DecTag{this, "DecTag", "", "Additional tag for decorator"};

};
#endif

