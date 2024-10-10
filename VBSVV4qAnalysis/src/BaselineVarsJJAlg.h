/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#ifndef VBSVV4qANALYSIS_BASELINEVARJJALG_H
#define VBSVV4qANALYSIS_BASELINEVARJJALG_H

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <AthContainers/ConstDataVector.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>

namespace VBSVV4q{

    /// \brief An algorithm for counting containers
    class BaselineVarsJJAlg final : public AthHistogramAlgorithm {
        /// \brief The standard constructor
        public: 
            BaselineVarsJJAlg(const std::string &name, ISvcLocator *pSvcLocator);

            /// \brief Initialisation method, for setting up tools and other persistent
            /// configs
            StatusCode initialize() override;
            /// \brief Execute method, for actions to be taken in the event loop
            StatusCode execute() override;
            /// We use default finalize() -- this is for cleanup, and we don't do any

        private:

        template<typename ParticleType>
            std::pair<int, int> truthOrigin(const ParticleType* particle);

        /// \brief Setup syst-aware input container handles
        CP::SysListHandle m_systematicsList {this};

        CP::SysReadHandle<xAOD::JetContainer> m_SmallRJetsHandle{ this, "SmallRJets", "", "Large R Jet container to read"};

        CP::SysReadHandle<xAOD::JetContainer> m_LargeRJetsHandle{ this, "LargeRJets", "", "Large R Jet container to read"};
        CP::SysReadHandle<xAOD::JetContainer> m_SigLargeRJetsHandle{ this, "SigLargeRJets", "", "Large R Jet container to read"};

        CP::SysReadHandle<xAOD::JetContainer> m_vbsjetHandle{ this, "vbsjets", "",   "VBS Jet container to read" };
        
        CP::SysReadHandle<xAOD::EventInfo>
        m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

        Gaudi::Property<bool> m_isMC { this, "isMC", false, "Is this simulation?" };

        CP::SysReadDecorHandle<char>  m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};
        //CP::SysReadDecorHandle<int> m_PCBT {this, "PCBTDecorName", "", "Name of pseudo-continuous b-tagging decorator"};

        CP::SysReadDecorHandle<float> m_GN2Xv01_phbb = {this, "phbb", "GN2Xv01_phbb", "GN2Xv01_phbb"};
        CP::SysReadDecorHandle<float> m_GN2Xv01_phcc = {this, "phcc", "GN2Xv01_phcc", "GN2Xv01_phcc"};
        CP::SysReadDecorHandle<float> m_GN2Xv01_pqcd = {this, "pqcd", "GN2Xv01_pqcd", "GN2Xv01_pqcd"};
        CP::SysReadDecorHandle<float> m_GN2Xv01_ptop = {this, "ptop", "GN2Xv01_ptop", "GN2Xv01_ptop"};        

        std::vector<std::string> m_JSS_list = {"ECF1", "ECF2", "ECF3", "Split12", "Split23", "Tau1_wta", "Tau2_wta", "Tau3_wta", "ZCut12",  "KtDR", "Angularity", "FoxWolfram0", "FoxWolfram2", "Aplanarity", "PlanarFlow", "Qw"};
        std::unordered_map<std::string, CP::SysReadDecorHandle<float>> m_JSS;

        Gaudi::Property<std::vector<std::string>> m_floatVariables {this, "floatVariableList", {}, "Name list of floating variables"};

        Gaudi::Property<std::vector<std::string>> m_intVariables {this, "intVariableList", {}, "Name list of integer variables"};

        /// \brief Setup sys-aware output decorations
        std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_Fbranches;

        std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;

    };
}
#endif