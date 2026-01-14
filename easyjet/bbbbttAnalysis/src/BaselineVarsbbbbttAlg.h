/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef BBBBTTANALYSIS_FINALVARSBBTTALG
#define BBBBTTANALYSIS_FINALVARSBBTTALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <AsgDataHandles/ReadDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODTau/TauJetContainer.h>

#include "HHHbbbbttEnums.h"

namespace HHHBBBBTT
{

  /// \brief An algorithm for counting containers
  class BaselineVarsbbbbttAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    BaselineVarsbbbbttAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:

    typedef std::unordered_map<HHHBBBBTT::RunBooleans, SG::ReadDecorHandle<xAOD::EventInfo, bool> > runBoolReadDecoMap;
    typedef std::unordered_map<HHHBBBBTT::Booleans, CP::SysReadDecorHandle<bool>> categoryReadDecoMap;

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::JetContainer>
      m_bbbbttJetHandle{ this, "bbbbttJets", "bbbbttAnalysisJets_%SYS%", "Jet container to read" };
    
    CP::SysReadHandle<xAOD::ElectronContainer>
      m_bbbbttElectronHandle{ this, "bbbbttElectrons", "bbbbttAnalysisElectrons_%SYS%", "Electron container to read" };
    CP::SysReadHandle<xAOD::ElectronContainer>
      m_electronHandle{ this, "electrons", "AnalysisElectrons_%SYS%", "Original electron container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
      m_bbbbttMuonHandle{ this, "bbbbttMuons", "bbbbttAnalysisMuons_%SYS%", "Muon container to read" };
    CP::SysReadHandle<xAOD::MuonContainer>
      m_muonHandle{ this, "muons", "AnalysisMuons_%SYS%", "Original muon container to read" };

    CP::SysReadHandle<xAOD::TauJetContainer>
      m_bbbbttTauHandle{ this, "bbbbttTaus", "bbbbttAnalysisTaus_%SYS%", "Tau container to read" };
    CP::SysReadHandle<xAOD::TauJetContainer>
      m_tauHandle{ this, "taus", "AnalysisTaus_%SYS%", "Original tau container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    CP::SysReadDecorHandle<float> 
      m_mmc_pt { this, "mmc_pt", "mmc_pt_%SYS%", "MMC pt key"};
    CP::SysReadDecorHandle<float> 
      m_mmc_eta { this, "mmc_eta", "mmc_eta_%SYS%", "MMC eta key"};
    CP::SysReadDecorHandle<float> 
      m_mmc_phi { this, "mmc_phi", "mmc_phi_%SYS%", "MMC phi key"};
    CP::SysReadDecorHandle<float> 
      m_mmc_m { this, "mmc_m", "mmc_m_%SYS%", "MMC mass key"};

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    Gaudi::Property<bool> m_doMMC
      {this, "doMMC", true, "Run with MMC?"};

    Gaudi::Property<bool> m_useNonIsoLeptons
      { this, "useNonIsoLeptons", false, "use NonIso lepton wps" };

    Gaudi::Property<std::vector<std::string>> m_eleWPNames
      { this, "eleWPs", {},"Electron ID + Iso working points" };
    Gaudi::Property<std::vector<std::string>> m_muonWPNames
      { this, "muonWPs", {},"Muon ID + Iso working points" };

    typedef std::unordered_map<HHHBBBBTT::LepSelWpDeco, CP::SysReadDecorHandle<float>> leptonSfDecoMap;
    leptonSfDecoMap m_ele_SF_decoMap;
    leptonSfDecoMap m_muon_SF_decoMap;

    Gaudi::Property<std::string> m_tauWPName
      { this, "tauWP", "","Tau ID working point" };
    CP::SysReadDecorHandle<float> m_tau_effSF{"", this};

    // tauID and anti-tauID decorators
    CP::SysReadDecorHandle<char> m_IDTau{"isIDTau", this};
    CP::SysReadDecorHandle<char> m_antiTau{"isAntiTau", this};

    CP::SysReadDecorHandle<char> m_EleRNNLoose{"EleRNNLoose_v1", this};
    CP::SysReadDecorHandle<char> m_EleRNNMedium{"EleRNNMedium_v1", this};
    CP::SysReadDecorHandle<char> m_EleRNNTight{"EleRNNTight_v1", this};

    CP::SysReadDecorHandle<int>
      m_truthTypeTau{ this, "truthTypeTau", "truthType", "Tau truth type" };

    CP::SysReadDecorHandle<int>
      m_tauTruthJetLabel { this, "tauTruthJetLabel", "tauTruthJetLabel", "Decoration tauTruthJetLabel" };

    CP::SysReadDecorHandle<bool> 
      m_selected_el { this, "selected_el", "selected_el_%SYS%", "Name of input decorator for selected el"};
    CP::SysReadDecorHandle<bool> 
      m_selected_el_isIso { this, "selected_el_isIso", "selected_el_isIso_%SYS%", "Input deco for selected el+iso"};
    
    CP::SysReadDecorHandle<bool> 
      m_selected_mu { this, "selected_mu", "selected_mu_%SYS%", "Name of input decorator for selected mu"};
    CP::SysReadDecorHandle<bool> 
      m_selected_mu_isIso { this, "selected_mu_isIso", "selected_mu_isIso_%SYS%", "Input deco for selected mu+iso"};
    
    CP::SysReadDecorHandle<bool> 
      m_selected_tau { this, "selected_tau", "selected_tau_%SYS%", "Name of input decorator for selected tau"};
    
    CP::SysReadDecorHandle<char> 
      m_isBtag {this, "bTagWPDecorName", "", "Name of input decorator for b-tagging"};
    Gaudi::Property<std::vector<std::string>> m_PCBTnames
      {this, "PCBTDecorList", {}, "Name list of pseudo-continuous b-tagging decorator"};
    std::unordered_map<std::string, CP::SysReadDecorHandle<int>> m_PCBTs;

    CP::SysReadDecorHandle<int>
      m_truthFlav{ this, "truthFlav", "HadronConeExclTruthLabelID", "Jet truth flavor" };
    CP::SysReadDecorHandle<int> m_nmuons{"n_muons_%SYS%", this};

    CP::SysReadDecorHandle<unsigned int> m_year{this, "year", "dataTakingYear", ""};

    Gaudi::Property<std::vector<std::string>> m_floatVariables
      {this, "floatVariableList", {}, "Name list of floating variables"};

    Gaudi::Property<std::vector<std::string>> m_intVariables
      {this, "intVariableList", {}, "Name list of integer variables"};

    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>>
      m_Fbranches;

    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;

    void fillLeptonSfDecoMap(const std::vector<std::string>& wpNames,
			     leptonSfDecoMap& decoMap);
  };
} // namespace HHHBBBBTT

#endif

