/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

*/

#ifndef VBSHIGGSANALYSIS_VBSJETSSELECTORALG_H
#define VBSHIGGSANALYSIS_VBSJETSSELECTORALG_H

#include <memory>

#include "AnaAlgorithm/AnaAlgorithm.h"

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>

#include <xAODJet/JetContainer.h>

#include <AthContainers/ConstDataVector.h>


namespace VBSHIGGS{

    class VBSJetsSelectorAlg final : public EL::AnaAlgorithm{
        /// \brief The standard constructor

        public:
            VBSJetsSelectorAlg(const std::string& name, ISvcLocator *pSvcLocator);

            /// \brief Initialisation method, for setting up tools and other persistent
            /// configs
            StatusCode initialize() override;
            /// \brief Execute method, for actions to be taken in the event loop
            StatusCode execute() override;

        private:
            /// \brief Setup syst-aware input container handles
            CP::SysListHandle m_systematicsList {this};

            CP::SysReadHandle<xAOD::JetContainer> m_jetHandle{ this, "jets", "vbshiggsAnalysisJets_%SYS%", "Jet container to read" };

            //VBS jets to write
            CP::SysWriteHandle<ConstDataVector<xAOD::JetContainer>> m_VBSjetOutHandle{ this, "VBSjetContainerOutKey", "vbshiggsAnalysisVBSJets_%SYS%", "VBS Jet container to write" };
            
            //non VBS jets to write
            CP::SysWriteHandle<ConstDataVector<xAOD::JetContainer>> m_NonVBsjetOutHandle{ this, "SignaljetContainerOutKey", "vbshiggsAnalysisSignalJets_%SYS%", "Non VBS Jet container to write" };
            
            // btagging
            CP::SysReadDecorHandle<char>  m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};

            // Large-R jet container (needed for analysis OR between large-small jets)
            CP::SysReadHandle<xAOD::JetContainer> m_vbsLRJetHandle{ this, "vbsLRJets", "vbshiggsAnalysisLargeJets_%SYS%", "Large R Jet container to read"};

            // Delta R cut (value taken based on Small-R - Large-R jet OR implemented in athena https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/Algorithms/AsgAnalysisAlgorithms/python/OverlapAnalysisConfig.py#L490 )
            Gaudi::Property<float> m_mindR{this, "minDR", 1., "Min Angular distance between large-R and small-R jets"};
    };
}

#endif 
