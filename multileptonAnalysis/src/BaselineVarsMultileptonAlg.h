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

    CP::SysReadHandle<xAOD::JetContainer> m_kfJetHandle{
        this, "KFJets", "bb4lAnalysisKFJets_%SYS%","KF jet container to read"};

    CP::SysReadDecorHandle<float> m_KF_MBB{"KF_mbb_%SYS%", this};

    xAOD::JetFourMom_t m_jet1_uncorr, m_jet2_uncorr;
    xAOD::JetFourMom_t m_jet1_muonCorr, m_jet2_muonCorr;

    CP::SysReadDecorHandle<char> m_isBtag
        {this, "bTagWPDecorName", "", "Name of input decorator for b-tagging"};

    Gaudi::Property<std::vector<std::string>> m_floatVariables
          {this, "floatVariableList", {}, "Name list of floating variables"};

    Gaudi::Property<std::vector<std::string>> m_intVariables
          {this, "intVariableList", {}, "Name list of integer variables"};

    Gaudi::Property<bool> m_save_extrabb4l_vars
          { this, "save_extrabb4l_vars", false, "Save extra variables for bb4l studies?" };

    Gaudi::Property<bool> m_doKinematicFit{this, "doKF", false,
                                           "Enable KF jets retrieval"};

    CP::SysReadDecorHandle<int>
        m_nmuons{ this, "nmuons", "n_muons_%SYS%", "Number of muons from muon-in-jet correction"};

    StatusCode SaveExtrabb4lVars(const std::vector<const xAOD::Jet*> &Hbbjets,
                            const std::vector<const xAOD::Jet*> &KFJets,
                            const xAOD::EventInfo *event, const auto& sys);

    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_Fbranches;

    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;
    
 };
}
#endif
