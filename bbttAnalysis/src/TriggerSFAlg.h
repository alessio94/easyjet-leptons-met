/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef BBTTANALYSIS_TRIGGERSFALG
#define BBTTANALYSIS_TRIGGERSFALG

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

#include "HHbbttEnums.h"

namespace HHBBTT
{

  /// \brief An algorithm for counting containers
  class TriggerSFAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    TriggerSFAlg(const std::string &name, ISvcLocator *pSvcLocator);

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
      m_electronHandle{ this, "electrons", "AnalysisElectrons_%SYS%", "Original electron container to read" };
    CP::SysReadHandle<xAOD::ElectronContainer>
      m_bbttEleHandle{ this, "bbttElectrons", "bbttAnalysisElectrons_%SYS%", "bbtt electron container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
      m_muonHandle{ this, "muons", "AnalysisMuons_%SYS%", "Original muon container to read" };
    CP::SysReadHandle<xAOD::MuonContainer>
      m_bbttMuonHandle{ this, "bbttMuons", "bbttAnalysisMuons_%SYS%", "bbtt muon container to read" };

    CP::SysReadHandle<xAOD::TauJetContainer>
      m_tauHandle{ this, "taus", "AnalysisTaus_%SYS%", "Original tau container to read" };
    CP::SysReadHandle<xAOD::TauJetContainer>
      m_bbttTauHandle{ this, "bbttTaus", "bbttAnalysisTaus_%SYS%", "bbtt tau container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    Gaudi::Property<std::vector<std::string>> m_eleTrigSF
      {this, "eleTriggerSF", {}, "List of electron trigger SF"};
    std::unordered_map<std::string, CP::SysReadDecorHandle<float>> m_eleTriggerSF;

    Gaudi::Property<std::vector<std::string>> m_muonTrigSF
      {this, "muonTriggerSF", {}, "List of muon trigger SF"};
    std::unordered_map<std::string, CP::SysReadDecorHandle<float>> m_muonTriggerSF;

    Gaudi::Property<std::vector<std::string>> m_tauTrigSF
      {this, "tauTriggerSF", {}, "List of tau trigger SF"};
    std::unordered_map<std::string, CP::SysReadDecorHandle<float>> m_tauTriggerSF;

    CP::SysReadDecorHandle<bool> m_selected_el{"selected_el_%SYS%", this};
    CP::SysReadDecorHandle<bool> m_selected_mu {"selected_mu_%SYS%", this};
    CP::SysReadDecorHandle<bool> m_selected_tau {"selected_tau_%SYS%", this};
    
    CP::SysReadDecorHandle<unsigned int> m_year{this, "year", "dataTakingYear", ""};
    Gaudi::Property<std::vector<unsigned int>> m_SF_year{ this, "SFyears", {}, "List of years to read jet trigger SF files"};

    // calibration map
    std::unordered_map<int, TH2D*> m_L1_jetTriggerSFMap;
    std::unordered_map<int, TH2D*> m_HLT_jetTriggerSFMap;

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
      {HHBBTT::pass_TopEMuCR, "pass_TopEMuCR"},
      {HHBBTT::pass_AntiIsoLepHad, "pass_AntiIsoLepHad"}
    };

    /// \brief Setup sys-aware output decorations
    CP::SysWriteDecorHandle<float> m_eventTriggerSF {"eventTriggerSF_%SYS%", this};

  };
} // namespace HHBBTT

#endif

