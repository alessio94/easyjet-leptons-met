/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef LLTTANALYSIS_HLLTTSELECTORALG
#define LLTTANALYSIS_HLLTTSELECTORALG

#include <memory>

#include "AnaAlgorithm/AnaAlgorithm.h"

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODTau/TauJetContainer.h>

#include <SystematicsHandles/SysFilterReporterParams.h>

#include "HllttChannels.h"

namespace HLLTT
{

  enum TriggerChannel
  {
    SLT = 0, 
    DLT = 1, 
  };

  enum Var
  {
    ele = 0, 
    mu = 1, 
    leadinglep = 2, 
    subleadinglep = 3,
  };

  /// \brief An algorithm for counting containers
  class HllttSelectorAlg final : public EL::AnaAlgorithm
  {
    /// \brief The standard constructor
public:
    HllttSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// \brief Finalisation method, for cleanup, final print out etc
    StatusCode finalize() override;

private:

    /// \brief Steerable properties
    Gaudi::Property<std::vector<std::string>> m_channel_names
      { this, "channel", {}, "Which channel to run" };

    std::vector<HLLTT::Channel> m_channels;

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };
    
    Gaudi::Property<bool> m_bypass
      { this, "bypass", false, "Run selector algorithm in pass-through mode" };

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::JetContainer>
      m_jetHandle{ this, "jets", "llttAnalysisJets_%SYS%", "Jet container to read" };
    
    CP::SysReadHandle<xAOD::ElectronContainer>
    m_electronHandle{ this, "electrons", "llttAnalysisElectrons_%SYS%", "Electron container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
    m_muonHandle{ this, "muons", "llttAnalysisMuons_%SYS%", "Muon container to read" };

    CP::SysReadHandle<xAOD::TauJetContainer>
    m_tauHandle{ this, "taus", "llttAnalysisTaus_%SYS%", "Tau container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    CP::SysReadDecorHandle<unsigned int> m_year
      {this, "year", "dataTakingYear", ""};

    Gaudi::Property<std::string> m_tauWPName
      { this, "tauWP", "", "Tau ID working point" };
    CP::SysReadDecorHandle<char> m_tauWPDecorHandle{"", this};

    CP::SysReadDecorHandle<char> 
    m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};

    std::unordered_map<std::string, CP::SysReadDecorHandle<bool> > m_triggerdecos;

    Gaudi::Property<std::string> m_eleWPName
      { this, "eleWP", "","Electron ID + Iso working point" };
    CP::SysReadDecorHandle<char> m_eleWPDecorHandle{"", this};
		      
    Gaudi::Property<std::string> m_muonWPName
      { this, "muonWP", "","Muon ID + Iso cuts" };
    CP::SysReadDecorHandle<char> m_muonWPDecorHandle{"", this};

    Gaudi::Property<std::vector<std::string>> m_triggers 
          {this, "triggerLists", {}, "Name list of trigger"};

    std::unordered_map<std::string, CP::SysWriteDecorHandle<bool> > m_Bbranches;
    std::vector<std::string> m_Bvarnames{      
      "pass_trigger_SLT", "pass_trigger_DLT", "pass_baseline_DLT", "pass_DLT",
	"pass_baseline_LepLep", "pass_LepLep", "pass_baseline_LepHad", "pass_LepHad",
	"pass_baseline_HadHad", "pass_HadHad", "pass_Looseele", "pass_Loosemuo"
    };


    CP::SysWriteDecorHandle<bool> m_selected_el {"selected_el_%SYS%", this};
    CP::SysWriteDecorHandle<bool> m_selected_mu {"selected_mu_%SYS%", this};
    CP::SysWriteDecorHandle<bool> m_selected_tau {"selected_tau_%SYS%", this};

    /// \brief Setup sys-aware output decorations
    CP::SysFilterReporterParams m_filterParams {this, "Hlltt selection"};
    
    /// \brief Internal variables

    bool trigPassed_SLT;
    bool trigPassed_DLT;
    bool N_LEPTONS_CUT_LEPLEP;
    bool N_LEPTONS_CUT_LEPHAD;
    bool N_LEPTONS_CUT_HADHAD;
    bool pass_baseline_LepLep;
    bool pass_LepLep;
    bool pass_baseline_LepHad;
    bool pass_LepHad;
    bool pass_baseline_HadHad;
    bool pass_HadHad;
    bool pass_baseline_DLT;
    bool pass_DLT;

    std::unordered_map<HLLTT::TriggerChannel, std::unordered_map<HLLTT::Var, float>> m_pt_threshold;

    void applyTriggerSelection(const xAOD::EventInfo* event, const CP::SystematicSet& sys);
    void applySLTTriggerSelection(const xAOD::EventInfo* event, const CP::SystematicSet& sys);
    void applyDiLepTriggerSelection(const xAOD::EventInfo* event, const CP::SystematicSet& sys);

  };
}

#endif
