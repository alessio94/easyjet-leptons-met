/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#ifndef VBSHIGGSANALYSIS_BASELINEVARFULLLEPALG_H
#define VBSHIGGSANALYSIS_BASELINEVARFULLLEPALG_H
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
    class BaselineVarsFullLepAlg final : public AthHistogramAlgorithm {
        /// \brief The standard constructor
        public: 
            BaselineVarsFullLepAlg(const std::string &name, ISvcLocator *pSvcLocator);

            /// \brief Initialisation method, for setting up tools and other persistent
            /// configs
            StatusCode initialize() override;
            /// \brief Execute method, for actions to be taken in the event loop
            StatusCode execute() override;
            /// We use default finalize() -- this is for cleanup, and we don't do any

        private:

        /// \brief Setup syst-aware input container handles
        CP::SysListHandle m_systematicsList {this};

        CP::SysReadHandle<xAOD::JetContainer> m_signaljetHandle{ this, "signaljets", "vbshiggsAnalysisSignalJets_%SYS%", "Small R jets Jet container to read"};
              
        CP::SysReadHandle<xAOD::JetContainer> m_HCandHandle{ this, "higgsCandidates", "vbshiggsAnalysisHJets_%SYS%", "Higgs Candidates container to read"};

        CP::SysReadHandle<xAOD::JetContainer> m_vbsLRJetHandle{ this, "vbsLRJets", "vbshiggsAnalysisLargeJets_%SYS%", "Large R Jet container to read"};

        CP::SysReadHandle<xAOD::JetContainer> m_vbsJetHandle{ this, "vbsJets", "",   "VBS Jet container to read" };
        
        CP::SysReadHandle<xAOD::JetContainer> m_RNNjetBoostedHandle{ this, "RNNJets_boosted", "RNNJets_boosted_%SYS%", "input RNN jets container for boosted 20 GeV" };

        CP::SysReadHandle<xAOD::JetContainer> m_RNNjetResolvedHandle{ this, "RNNJets_resolved", "RNNJets_resolved_%SYS%", "input RNN jets container for resolved 20 GeV" };
        
        CP::SysReadHandle<xAOD::ElectronContainer> m_vbsElectronHandle{ this, "vbsElectrons", "vbshiggsAnalysisElectrons_%SYS%",   "Electron container to read" };
        CP::SysReadHandle<xAOD::ElectronContainer> m_electronHandle{ this, "electrons", "AnalysisElectrons_%SYS%", "Original electron container to read" };

        CP::SysReadHandle<xAOD::MuonContainer> m_vbsMuonHandle{ this, "vbsMuons", "vbshiggsAnalysisMuons_%SYS%",   "Muon container to read" };
        CP::SysReadHandle<xAOD::MuonContainer> m_muonHandle{ this, "muons", "AnalysisMuons_%SYS%", "Original muon container to read" };

        CP::SysReadHandle<xAOD::MissingETContainer> m_metHandle{ this, "met", "AnalysisMET_%SYS%",   "MET container to read" };

        CP::SysReadHandle<xAOD::EventInfo>
        m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

        Gaudi::Property<bool> m_isMC { this, "isMC", false, "Is this simulation?" };

        Gaudi::Property<std::string> m_eleWPName { this, "eleWP", "","Electron ID + Iso working point" };
        CP::SysReadDecorHandle<float> m_ele_SF{"", this};
        Gaudi::Property<bool> m_saveDummy_ele_SF
        {this, "saveDummyEleSF", false,
	  "To be used in case no recommendations are not available"};

        CP::SysReadDecorHandle<int> m_ele_truthOrigin{"truthOrigin", this};
        CP::SysReadDecorHandle<int> m_ele_truthType{"truthType", this};

        Gaudi::Property<std::string> m_muWPName { this, "muonWP", "","Muon ID + Iso working point" };
        CP::SysReadDecorHandle<float> m_mu_SF{"", this};
        CP::SysReadDecorHandle<int> m_mu_truthOrigin{"truthOrigin", this};
        CP::SysReadDecorHandle<int> m_mu_truthType{"truthType", this};

        CP::SysReadDecorHandle<char>  m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};

        CP::SysReadDecorHandle<int> m_PCBT {this, "PCBTDecorName", "", "Name of pseudo-continuous b-tagging decorator"};

        CP::SysReadDecorHandle<int> m_truthFlav { this, "truthFlav", "HadronConeExclTruthLabelID", "Jet truth flavor" };

        CP::SysReadDecorHandle<char> m_eleECIDS { this, "ElectronsECIDS", "DFCommonElectronsECIDS", "Charge ID Selector" };

        CP::SysReadDecorHandle<float> m_GN2Xv01_phbb = {this, "phbb", "GN2Xv01_phbb", "GN2Xv01_phbb"};
        CP::SysReadDecorHandle<float> m_GN2Xv01_phcc = {this, "phcc", "GN2Xv01_phcc", "GN2Xv01_phcc"};
        CP::SysReadDecorHandle<float> m_GN2Xv01_pqcd = {this, "pqcd", "GN2Xv01_pqcd", "GN2Xv01_pqcd"};
        CP::SysReadDecorHandle<float> m_GN2Xv01_ptop = {this, "ptop", "GN2Xv01_ptop", "GN2Xv01_ptop"};

        CP::SysReadDecorHandle<float>
        m_METSig {this, "METSignificance", "significance", "Met Significance"};
        

        Gaudi::Property<std::vector<std::string>> m_floatVariables {this, "floatVariableList", {}, "Name list of floating variables"};

        Gaudi::Property<std::vector<std::string>> m_intVariables {this, "intVariableList", {}, "Name list of integer variables"};

        /// \brief Setup sys-aware output decorations
        std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_Fbranches;

        std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;

        Gaudi::Property<bool> m_UseVBFRNN { this, "UseVBFRNN", false, "Not use VBS tagging jets or yes (i.e. use VBF-RNN jets or not)" };

    };
}
#endif
