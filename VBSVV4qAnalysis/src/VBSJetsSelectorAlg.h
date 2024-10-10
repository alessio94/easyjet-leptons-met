/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

*/

#ifndef VBSVV4qANALYSIS_VBSJETSSELECTORALG_H
#define VBSVV4qANALYSIS_VBSJETSSELECTORALG_H

#include <memory>

#include "AnaAlgorithm/AnaAlgorithm.h"

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>

#include <xAODJet/JetContainer.h>

#include <AthContainers/ConstDataVector.h>


namespace VBSVV4q{

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

            CP::SysReadHandle<xAOD::JetContainer> m_smallRjetHandle{ this, "smallRjets", "VBSVV4qAnalysisJets_%SYS%", "Jet container to read" };
            CP::SysReadHandle<xAOD::JetContainer> m_largeRjetHandle{ this, "largeRjets", "VBSVV4qAnalysisLargeJets_%SYS%", "Jet container to read" };

            //VBS jets to write
            CP::SysWriteHandle<ConstDataVector<xAOD::JetContainer>> m_VBSjetOutHandle{ this, "VBSJetsContainerOutKey", "", "VBS Jet container to write" };
            
            //non VBS jets to write
            CP::SysWriteHandle<ConstDataVector<xAOD::JetContainer>> m_NonVBSjetOutHandle{ this, "noVBSJetsContainerOutKey", "", "Non VBS Jet container to write" };
            
            CP::SysReadDecorHandle<char> m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};

            Gaudi::Property<std::string> m_TagJetsCriteria { this, "TagJetsCriteria", "", "Criteria to use to select tag jets" };

    };
}

#endif 
