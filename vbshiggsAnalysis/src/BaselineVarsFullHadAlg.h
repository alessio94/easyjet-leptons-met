/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#ifndef VBSHIGGSANALYSIS_BASELINEVARFULLHADALG_H
#define VBSHIGGSANALYSIS_BASELINEVARFULLHADALG_H
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <AthContainers/ConstDataVector.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMissingET/MissingETContainer.h>

namespace VBSHIGGS{
    /// \brief An algorithm for counting containers
    class BaselineVarsFullHadAlg final : public AthHistogramAlgorithm {
        /// \brief The standard constructor
        public: 
            BaselineVarsFullHadAlg(const std::string &name, ISvcLocator *pSvcLocator);

            /// \brief Initialisation method, for setting up tools and other persistent
            /// configs
            StatusCode initialize() override;
            /// \brief Execute method, for actions to be taken in the event loop
            StatusCode execute() override;
            /// We use default finalize() -- this is for cleanup, and we don't do any

        private:
        /// \brief Setup syst-aware input container handles
        CP::SysListHandle m_systematicsList {this};

        CP::SysReadHandle<xAOD::EventInfo> m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

        CP::SysReadHandle<xAOD::JetContainer> m_jetHandle{ this, "jets", "",   "Jet container to read" };
        
        CP::SysReadHandle<xAOD::ElectronContainer> m_electronHandle{ this, "electrons", "",   "Electron container to read" };

        CP::SysReadHandle<xAOD::MuonContainer> m_muonHandle{ this, "muons", "",   "Muon container to read" };

        CP::SysReadDecorHandle<char>  m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};

        Gaudi::Property<bool> m_isMC { this, "isMC", false, "Is this simulation?" };
        
        Gaudi::Property<std::vector<std::string>> m_floatVariables {this, "floatVariableList", {}, "Name list of floating variables"};

        Gaudi::Property<std::vector<std::string>> m_intVariables {this, "intVariableList", {}, "Name list of integer variables"};

        /// \brief Setup sys-aware output decorations
        std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_Fbranches;

        std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;
        
    };
}
#endif