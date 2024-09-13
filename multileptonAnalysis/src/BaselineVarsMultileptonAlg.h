/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HHBBLLANALYSIS_FINALVARSBBLLALG
#define HHBBLLANALYSIS_FINALVARSBBLLALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODTau/TauJetContainer.h>
#include <xAODMissingET/MissingETContainer.h>

namespace MULTILEPTON
{


  /// \brief An algorithm for counting containers
  class BaselineVarsMultileptonAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    BaselineVarsMultileptonAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    
    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::JetContainer>
        m_jetHandle{ this, "jets", "hhmlAnalysisJets_%SYS%", "Jet container to read" };

    CP::SysReadHandle<xAOD::ElectronContainer>
        m_electronHandle{ this, "electrons", "hhmlAnalysisElectrons_%SYS%", "Electron container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
        m_muonHandle{ this, "muons", "hhmlAnalysisMuons_%SYS%", "Muon container to read" };

    CP::SysReadHandle<xAOD::TauJetContainer>
        m_tauHandle{ this, "taus", "hhmlAnalysisTaus_%SYS%", "Tau container to read" };

    CP::SysReadHandle<xAOD::MissingETContainer>
        m_metHandle{ this, "met", "AnalysisMET_%SYS%", "MET container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
        m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    Gaudi::Property<bool> m_isMC
        { this, "isMC", false, "Is this simulation?" };

    Gaudi::Property<std::string> m_eleWPName
        { this, "eleWP", "","Electron ID + Iso working point" };

    Gaudi::Property<std::string> m_muWPName
        { this, "muonWP", "","Muon ID + Iso working point" };

    Gaudi::Property<std::string> m_tauWPName
        { this, "tauWP", "","Tau ID working point" };

    CP::SysReadDecorHandle<char> m_isBtag 
        {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};

    Gaudi::Property<unsigned int> m_leptonAmount
        { this, "leptonAmount", 4, "Number of leptons required" };

    Gaudi::Property<unsigned int> m_tauAmount
        { this, "tauAmount", 3, "Number of taus required" };

    Gaudi::Property<unsigned int> m_jetAmount
        { this, "jetAmount", 4, "Number of jets required" };

    Gaudi::Property<unsigned int> m_lightJetAmount
        { this, "lightJetAmount", 2, "Number of light jets required" };

    Gaudi::Property<unsigned int> m_bJetAmount
        { this, "bJetAmount", 2, "Number of b-jets required" };

    Gaudi::Property<std::vector<std::string>> m_floatVariables
          {this, "floatVariableList", {}, "Name list of floating variables"};

    Gaudi::Property<std::vector<std::string>> m_floatVectorVariables
          {this, "floatVectorVariableList", {}, "Name list of vectors of floating variables"};

    Gaudi::Property<std::vector<std::string>> m_intVariables
          {this, "intVariableList", {}, "Name list of integer variables"};

    Gaudi::Property<std::vector<std::string>> m_charVectorVariables
          {this, "charVectorVariableList", {}, "Name list of vectors of character variables"};

    CP::SysReadDecorHandle<float> m_met_sig
          {this, "METSignificance", "significance", "Met Significance"};

    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_Fbranches;

    std::unordered_map<std::string, CP::SysWriteDecorHandle<std::vector<float>>> m_FVbranches;

    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;

    std::unordered_map<std::string, CP::SysWriteDecorHandle<std::vector<char>>> m_CVbranches;
 };
}
#endif
