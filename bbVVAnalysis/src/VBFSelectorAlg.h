/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Celine Stauch

// Always protect against multiple includes!
#ifndef BBVVANALYSIS_VBFSELECTORALG
#define BBVVANALYSIS_VBFSELECTORALG

#include "AnaAlgorithm/AnaAlgorithm.h"

#include <AthenaKernel/Units.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysFilterReporterParams.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODTau/TauJetContainer.h>
#include <xAODMissingET/MissingETContainer.h>
#include "TriggerMatchingTool/IMatchingTool.h"
#include <EasyjetHub/CutManager.h>

#include "HHbbVVEnums.h"


class CutManager;

namespace HHBBVV
{
  /// \brief An algorithm for counting containers
  class VBFSelectorAlg final : public EL::AnaAlgorithm
  {
    /// \brief The standard constructor
public:
    VBFSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// \brief Finalisation method, for cleanup, final print out etc
    StatusCode finalize() override;

private:

    void distJetClassification(const xAOD::JetContainer& lrjets, const xAOD::Jet *&Hbb, const xAOD::Jet *&Whad,
                               const TLorentzVector& lepton, const CP::SystematicSet& sys);

    void massJetClassification(const xAOD::JetContainer& lrjets, const xAOD::Jet *&Hbb, const xAOD::Jet *&Whad,
                             const CP::SystematicSet& sys);
                             
    void signalBtagging(const xAOD::Jet *&Hbb, const xAOD::Jet *&Whad, const CP::SystematicSet& sys, bool& HBB_BTAG, bool& WHAD_BTAG);
    
    void VBFMassJetClassification(const xAOD::JetContainer& lrjets, const xAOD::Jet *&Whad, const CP::SystematicSet& sys);

    void VBFJetSelectionboosted(const xAOD::JetContainer& jets, const xAOD::Jet *&fwjet1, const xAOD::Jet *&fwjet2, const xAOD::Jet *&Hbb, const xAOD::Jet *&Whad);

    void VBFJetSelectionsplitboosted(const xAOD::JetContainer& jets, const xAOD::Jet *&fwjet1,const xAOD::Jet *&fwjet2 , const xAOD::Jet *&Whad);

    StatusCode initialiseCutflow();

    const std::vector<std::string> m_STANDARD_CUTS{
          // add more standard cuts
          "AT_LEAST_TWO_LRJETS",
          "EXACTLY_ONE_LEPTON", // For 1lep channels
          "DR_CUT", // DR btw lepton and WHad. dR < 1.0 to define boosted 1-lep. dR > 1.0 for split-boosted 1-lep
          "HBB_PT", // Hbb_lrjet pass pt > 500 GeV cut
          "HBB_BTAG", // Hbb_Btag pass the Btag WP cut
          "1VBF_JETS",
          "2VBF_JETS",
          "SIGNAL_LEPTON",
          "NO_TAU",
          "PASS_TRIGGER"
      };

    /// \brief Steerable properties
    Gaudi::Property<std::vector<std::string>> m_channel_names
      { this, "channel", {}, "Which channel to run" };

    std::vector<HHBBVV::Channel> m_channels;

    Gaudi::Property<bool> m_bypass
      { this, "bypass", false, "Run selector algorithm in pass-through mode" };

    /// \brief Setup syst-aware input container handles
    
    Gaudi::Property<bool> m_useTriggerSel
      { this, "useTriggerSelections", true, "Apply trigger-related selections" };


    CP::SysListHandle m_systematicsList {this};
    CutManager m_bbVVCuts;

    CP::SysReadHandle<xAOD::JetContainer>
      m_jetHandle{ this, "jets", "bbVVAnalysisJets_%SYS%", "Jet container to read" };

    CP::SysReadHandle<xAOD::JetContainer>
      m_lrjetHandle{ this, "lrjets", "bbVVAnalysisLRJets_%SYS%", "Large-R jet container to read" };

    Gaudi::Property<std::vector<std::string>> m_GN2X_wps
      { this, "GN2X_WPs", {}, "GN2X_hbb_wps from the bbVV config" };
    CP::SysReadDecorHandle<int> m_Pass_GN2X{"", this}; 
    
    CP::SysReadHandle<xAOD::ElectronContainer>
    m_electronHandle{ this, "electrons", "bbVVAnalysisElectrons_%SYS%", "Electron container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
    m_muonHandle{ this, "muons", "bbVVAnalysisMuons_%SYS%", "Muon container to read" };

    CP::SysReadHandle<xAOD::TauJetContainer>
    m_tauHandle{ this, "taus", "bbVVAnalysisTauJets_%SYS%", "TauJets container to read" };

    CP::SysReadHandle<xAOD::MissingETContainer>
    m_metHandle{ this, "met", "AnalysisMET_%SYS%", "MET container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    CP::SysReadDecorHandle<char> 
    m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};
    
    CP::SysReadDecorHandle<unsigned int> m_year
      {this, "year", "dataTakingYear", ""};

    Gaudi::Property<std::string> m_eleWPName
      { this, "eleWP", "","Electron ID + Iso working point" };
    CP::SysReadDecorHandle<char> m_eleWPDecorHandle{"", this};

    Gaudi::Property<std::string> m_ele_TightTighTrackOnlyWPName
      { this, "ele_Tight_TighTrackOnly", "TightLH_TightTrackOnly_VarRad", "Electron tight ID + TightTrackOnly Iso working point" };
    CP::SysReadDecorHandle<char> m_ele_TightTighTrackOnlyDecorHandle{"", this};

    Gaudi::Property<std::string> m_muonWPName
      { this, "muonWP", "","Muon ID + Iso cuts" };
    CP::SysReadDecorHandle<char> m_muonWPDecorHandle{"", this};

    Gaudi::Property<std::string> m_mu_TightPflowLooseWPName
      { this, "mu_Tight_PflowLoose", "Tight_PflowLoose_VarRad", "Muon Tight ID + PflowLoose Iso working  point"};
    CP::SysReadDecorHandle<char> m_mu_TightPflowLooseDecorHandle{"", this};

    Gaudi::Property<std::string> m_tauWPName
      { this, "tauWP", "", "Tau ID working point" };
    CP::SysReadDecorHandle<char> m_tauWPDecorHandle{"", this};
    
    std::unordered_map<HHBBVV::TriggerChannel, std::string> m_triggerChannels =
      {
            {HHBBVV::SLT, "SLT"},
      };

    Gaudi::Property<std::vector<std::string>> m_triggers 
      { this, "triggerLists", {}, "Name list of trigger" };
    
    std::unordered_map<std::string, CP::SysReadDecorHandle<bool> > m_triggerdecos;
    
    ToolHandle<Trig::IMatchingTool> m_matchingTool
      { this, "trigMatchingTool", "", "Trigger matching tool"};
    
    
    /// \brief Setup sys-aware output decorations
    CP::SysWriteDecorHandle<bool> m_pass_sr {"pass_bbVV_sr_%SYS%", this};
    
    std::unordered_map<HHBBVV::TriggerChannel, std::unordered_map<HHBBVV::Var, float>> m_pt_threshold;

    CP::SysFilterReporterParams m_filterParams {this, "HHbbVV selection"};

    CP::SysWriteDecorHandle<bool> m_selected_el {"selected_el_%SYS%", this};
    CP::SysWriteDecorHandle<bool> m_selected_mu {"selected_mu_%SYS%", this};
    CP::SysWriteDecorHandle<bool> m_matched_el {"matched_el_%SYS%", this};
    CP::SysWriteDecorHandle<bool> m_matched_mu {"matched_mu_%SYS%", this};

    CP::SysWriteDecorHandle<bool> m_selected_tau {"selected_tau_%SYS%", this};

    CP::SysWriteDecorHandle<bool> m_Whad {"Whad_%SYS%", this};
    CP::SysWriteDecorHandle<bool> m_Hbb {"Hbb_%SYS%", this};
    
    /// \brief Internal variables
    // TODO: implement internal and relevant bbVV variables here
    bool m_run_VBF = false;
    // counter vars (for debugging)
    int Jet_evt = 0; // Need to decide whether bbVV needs AT_LEAST_TWO_JETS
    long long int n_evt = 0;

    const double m_Wmass = 80.379 * Athena::Units::GeV; // This is the W mass used in earlier study
    const double m_Hmass = 125.09 * Athena::Units::GeV; // This is the H mass used in earlier study


    Gaudi::Property<std::vector<std::string>> m_inputCutList{this, "cutList", {}};
    Gaudi::Property<bool> m_saveCutFlow{this, "saveCutFlow", true};

    CP::SysWriteDecorHandle<bool> m_passallcuts {"PassAllCuts_%SYS%", this};
    
    std::unordered_map < HHBBVV::Booleans, CP::SysWriteDecorHandle<bool> > m_Bbranches;
    std::unordered_map < HHBBVV::Booleans, bool > m_bools;
    std::unordered_map < HHBBVV::Booleans, std::string > m_boolnames{
        {HHBBVV::pass_trigger_SET, "pass_trigger_SET"},
        {HHBBVV::pass_trigger_SMT, "pass_trigger_SMT"},
        {HHBBVV::pass_trigger_SR, "pass_trigger_SR"},
        {HHBBVV::pass_trigger_SLT, "pass_trigger_SLT"},
        {HHBBVV::pass_trigger_LRT, "pass_trigger_LRT"},
    };

    void evaluateSingleLeptonTrigger(const xAOD::EventInfo* event, const xAOD::Electron* ele, const xAOD::Muon* mu, const CP::SystematicSet& sys);
    void evaluateLRJetTrigger(const xAOD::EventInfo *event, const xAOD::Jet *lrjet, const CP::SystematicSet& sys);
    void setThresholds(const xAOD::EventInfo* event, const CP::SystematicSet& sys);
    
  };
}

#endif
