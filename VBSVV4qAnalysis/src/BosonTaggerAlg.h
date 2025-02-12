/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// BosonTaggerAlg.h
//
// Algorithm that will decorate taggers score using the JSSTaggersUtils
//
// Author: Antonio Giannini
///////////////////////////////////////////////////////////////////

#ifndef VBSVV4qANALYSIS_BOSONTAGGERALG_H
#define VBSVV4qANALYSIS_BOSONTAGGERALG_H

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>

#include <AthContainers/ConstDataVector.h>
#include <xAODJet/JetContainer.h>

#include "BoostedJetTaggers/JSSTaggerUtils.h"

namespace VBSVV4q{

    /// \brief An algorithm for counting containers
    class BosonTaggerAlg final : public AthHistogramAlgorithm {
        /// \brief The standard constructor
        public: 
            BosonTaggerAlg(const std::string &name, ISvcLocator *pSvcLocator);

            /// \brief Initialisation method, for setting up tools and other persistent
            /// configs
            StatusCode initialize() override;
            /// \brief Execute method, for actions to be taken in the event loop
            StatusCode execute() override;
            /// We use default finalize() -- this is for cleanup, and we don't do any

        private:

            SG::ReadHandleKey<xAOD::JetContainer> m_LargeRJetsHandle{ this, "LargeRJets", "", "Large R Jet container to read"};
            
            // boson tagger
            ToolHandle<IJSSTaggerUtils> m_MLTagger {this, "MLTagger", "", "Tagger Decorator Tool"};

    };
}
#endif
