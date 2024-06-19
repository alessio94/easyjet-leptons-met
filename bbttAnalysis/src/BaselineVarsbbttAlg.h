/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef BBTTANALYSIS_FINALVARSBBTTALG
#define BBTTANALYSIS_FINALVARSBBTTALG

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
#include <xAODMissingET/MissingETContainer.h>

#include "HHbbttEnums.h"

namespace HHBBTT
{

  /// \brief An algorithm for counting containers
  class BaselineVarsbbttAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    BaselineVarsbbttAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:

    typedef std::unordered_map<HHBBTT::RunBooleans, SG::ReadDecorHandle<xAOD::EventInfo, bool> > runBoolReadDecoMap;
    typedef std::unordered_map<HHBBTT::Booleans, CP::SysReadDecorHandle<bool>> categoryReadDecoMap;

    void computeTriggerSF(const xAOD::EventInfo *event, const CP::SystematicSet& sys,
			  const runBoolReadDecoMap& runBoolDecos,
			  const xAOD::Electron* ele0, const xAOD::Muon* mu0,
			  const xAOD::TauJet* tau0, const xAOD::TauJet* tau1);

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::JetContainer>
      m_jetHandle{ this, "jets", "bbttAnalysisJets_%SYS%", "Jet container to read" };
    
    CP::SysReadHandle<xAOD::ElectronContainer>
      m_electronHandle{ this, "electrons", "bbttAnalysisElectrons_%SYS%", "Electron container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
      m_muonHandle{ this, "muons", "bbttAnalysisMuons_%SYS%", "Muon container to read" };

    CP::SysReadHandle<xAOD::TauJetContainer>
      m_tauHandle{ this, "taus", "bbttAnalysisTaus_%SYS%", "Tau container to read" };

    CP::SysReadHandle<xAOD::MissingETContainer>
      m_metHandle{ this, "met", "AnalysisMET_%SYS%", "MET container to read" };

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

    Gaudi::Property<std::string> m_eleWPName
      { this, "eleWP", "","Electron ID + Iso working point" };
    CP::SysReadDecorHandle<float> m_ele_SF{"", this};

    Gaudi::Property<std::vector<std::string>> m_eleTrigSF
      {this, "eleTriggerSF", {}, "List of electron trigger SF"};
    std::unordered_map<std::string, CP::SysReadDecorHandle<float>> m_eleTriggerSF;

    Gaudi::Property<std::string> m_muWPName
      { this, "muonWP", "","Muon ID + Iso working point" };
    CP::SysReadDecorHandle<float> m_mu_SF{"", this};

    Gaudi::Property<std::vector<std::string>> m_muonTrigSF
      {this, "muonTriggerSF", {}, "List of muon trigger SF"};
    std::unordered_map<std::string, CP::SysReadDecorHandle<float>> m_muonTriggerSF;

    Gaudi::Property<std::string> m_tauWPName
      { this, "tauWP", "","Tau ID working point" };
    CP::SysReadDecorHandle<float> m_tau_effSF{"", this};

    Gaudi::Property<std::vector<std::string>> m_tauTrigSF
      {this, "tauTriggerSF", {}, "List of tau trigger SF"};
    std::unordered_map<std::string, CP::SysReadDecorHandle<float>> m_tauTriggerSF;

    // tauID and anti-tauID decorators
    CP::SysReadDecorHandle<char> m_IDTau{"isIDTau", this};
    CP::SysReadDecorHandle<char> m_antiTau{"isAntiTau", this};

    CP::SysReadDecorHandle<int>
      m_truthTypeTau{ this, "truthTypeTau", "truthType", "Tau truth type" };

    CP::SysReadDecorHandle<int>
      m_tauTruthJetLabel { this, "tauTruthJetLabel", "tauTruthJetLabel", "Decoration tauTruthJetLabel" };

    CP::SysReadDecorHandle<bool> 
      m_selected_el { this, "selected_el", "selected_el_%SYS%", "Name of input decorator for selected el"};
    CP::SysReadDecorHandle<bool> 
      m_selected_mu { this, "selected_mu", "selected_mu_%SYS%", "Name of input decorator for selected mu"};
    CP::SysReadDecorHandle<bool> 
      m_selected_tau { this, "selected_tau", "selected_tau_%SYS%", "Name of input decorator for selected tau"};
    
    CP::SysReadDecorHandle<char> 
      m_isBtag {this, "bTagWPDecorName", "", "Name of input decorator for b-tagging"};
    Gaudi::Property<std::vector<std::string>> m_PCBTnames
      {this, "PCBTDecorList", {}, "Name list of pseudo-continuous b-tagging decorator"};
    std::unordered_map<std::string, CP::SysReadDecorHandle<int>> m_PCBTs;

    CP::SysReadDecorHandle<int>
      m_truthFlav{ this, "truthFlav", "HadronConeExclTruthLabelID", "Jet truth flavor" };

    CP::SysReadDecorHandle<unsigned int> m_year{this, "year", "dataTakingYear", ""};

    const std::unordered_map<HHBBTT::RunBooleans, std::string> m_runBooleans =
      {
	{HHBBTT::is16PeriodA, "is2016_periodA"},
	{HHBBTT::is16PeriodB_D3, "is2016_periodB_D3"},
	{HHBBTT::is16PeriodD4_end, "is2016_periodD4_end"},
	{HHBBTT::is17PeriodB1_B4, "is2017_periodB1_B4"},
	{HHBBTT::is17PeriodB5_B7, "is2017_periodB5_B7"},
	{HHBBTT::is17PeriodB8_end, "is2017_periodB8_end"},
	{HHBBTT::is18PeriodB_end, "is2018_periodB_end"},
	{HHBBTT::is18PeriodK_end, "is2018_periodK_end"},
	{HHBBTT::is22_75bunches, "is2022_75bunches"},
	{HHBBTT::is23_75bunches, "is2023_75bunches"},
	{HHBBTT::is23_400bunches, "is2023_400bunches"},
	{HHBBTT::is23_from1200bunches, "is2023_from1200bunches"},
	{HHBBTT::is23_first_2400bunches, "is2023_first_2400bunches"},
	{HHBBTT::l1topo_disabled, "l1TopoDisabled"},
      };
    std::map<HHBBTT::RunBooleans, SG::ReadDecorHandleKey<xAOD::EventInfo>> m_runBooleans_key;

    categoryReadDecoMap m_categoryBranches;
    std::unordered_map < HHBBTT::Booleans, std::string > m_boolnames{
      {HHBBTT::pass_baseline_SLT, "pass_baseline_SLT"},
      {HHBBTT::pass_baseline_LTT, "pass_baseline_LTT"},
      {HHBBTT::pass_baseline_STT, "pass_baseline_STT"},
      {HHBBTT::pass_baseline_DTT, "pass_baseline_DTT"},
      {HHBBTT::pass_ZCR, "pass_ZCR"},
      {HHBBTT::pass_TopEMuCR, "pass_TopEMuCR"}
    };

    Gaudi::Property<bool> m_storeHighLevelVariables
      { this, "storeHighLevelVariables", false, "Flag to store high level variables in output" };

    Gaudi::Property<std::vector<std::string>> m_floatVariables
      {this, "floatVariableList", {}, "Name list of floating variables"};

    Gaudi::Property<std::vector<std::string>> m_intVariables
      {this, "intVariableList", {}, "Name list of integer variables"};

    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>>
      m_Fbranches;

    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;

  };
} // namespace HHBBTT

#endif

