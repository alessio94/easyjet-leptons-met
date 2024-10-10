/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

*/

#ifndef VBSVV4qANALYSIS_SignalJetsSelectorAlg
#define VBSVV4qANALYSIS_SignalJetsSelectorAlg
#include <memory>

#include "AnaAlgorithm/AnaAlgorithm.h"

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>

#include <xAODJet/JetContainer.h>

#include <AthContainers/ConstDataVector.h>

namespace VBSVV4q{
    class SignalJetsSelectorAlg final : public EL::AnaAlgorithm{
        /// \brief The standard constructor
        public:
            SignalJetsSelectorAlg(const std::string& name, ISvcLocator *pSvcLocator);

            /// \brief Initialisation method, for setting up tools and other persistent
            /// configs
            StatusCode initialize() override;
            /// \brief Execute method, for actions to be taken in the event loop
            StatusCode execute() override;

        private:
            /// \brief Setup syst-aware input container handles
            CP::SysListHandle m_systematicsList {this};

            CP::SysReadHandle<xAOD::JetContainer> m_SmallRJetsHandle{ this, "SmallRJets", "", "Jet container to read"};
            CP::SysReadHandle<xAOD::JetContainer> m_LargeRJetsHandle{ this, "LargeRJets", "", "Jet container to read"};

            // signal jets to write
            CP::SysWriteHandle<ConstDataVector<xAOD::JetContainer>> m_SignalLargeRJetsOutHandle{ this, "SigLargeRJets", "", "Signal Jets candidates container to write"};
            
            // it will be useful for resolved
            CP::SysReadDecorHandle<char>  m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};

    };
}
#endif