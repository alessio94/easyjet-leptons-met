/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Kira Abeling, JaeJin Hong

// Always protect against multiple includes!
#ifndef BBVVANALYSIS_HHBBVVSELECTORALG
#define BBVVANALYSIS_HHBBVVSELECTORALG

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
#include <xAODMissingET/MissingETContainer.h>
#include <EasyjetHub/CutManager.h>

#include "HHbbVVEnums.h"


class CutManager;

namespace HHBBVV
{
  /// \brief An algorithm for counting containers
  class HHbbVVSelectorAlg final : public EL::AnaAlgorithm
  {
    /// \brief The standard constructor
public:
    HHbbVVSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// \brief Finalisation method, for cleanup, final print out etc
    StatusCode finalize() override;

    float Tau42(const xAOD::Jet* lrjet, const CP::SystematicSet& sys);

    void distJetClassification(const xAOD::JetContainer& lrjets, const xAOD::Jet *&Hbb, const xAOD::Jet *&Whad,
                               const TLorentzVector& lepton, const CP::SystematicSet& sys);

    void massJetClassification(const xAOD::JetContainer& lrjets, const xAOD::Jet *&Hbb, const xAOD::Jet *&Whad,
      	                       const CP::SystematicSet& sys);

    void SubjetnessJetClassification(const xAOD::JetContainer& lrjets, const xAOD::Jet *&Hbb, const xAOD::Jet *&Whad,
      	                       const CP::SystematicSet& sys); // Select WHad first, then select Hbb whose mass is closest to H

    void splitboostedJetClassification(const xAOD::JetContainer& lrjets, const xAOD::Jet *&Hbb, const xAOD::Jet *&Whad,
                               const xAOD::Jet *&Whad2, const CP::SystematicSet& sys); //temporary method for running split-boosted

    void signalBtagging(const xAOD::Jet *&Hbb, const xAOD::Jet *&Whad, const CP::SystematicSet& sys, bool& HBB_BTAG, bool& WHAD_BTAG);

    void signalBtagging(const xAOD::Jet *&Hbb, const xAOD::Jet *&Whad, const xAOD::Jet *&Whad2, const CP::SystematicSet& sys, bool& HBB_BTAG, bool& WHAD_BTAG, bool& WHAD2_BTAG);

    const std::vector<std::string> m_STANDARD_CUTS{
          // add more standard cuts
          "LEAD_LRJ_PT", // Leading lrjet pt > 500 GeV
          "AT_LEAST_TWO_LRJETS",
          "AT_LEAST_THREE_LRJETS", // For 0-lep split-boosted
          "EXACTLY_ONE_LEPTON", // For 1lep channels
          "VETO_SIGNAL_LEPTON", // For 0lep channels
          "DR_CUT", // DR btw lepton and WHad. dR < 1.0 to define boosted 1-lep. dR > 1.0 for split-boosted 1-lep
          "HBB_PT", // Hbb_lrjet pass pt > 500 GeV cut
          "HBB_BTAG" // Hbb_Btag pass the Btag WP cut
      };



private:

    /// \brief Steerable properties
    Gaudi::Property<std::vector<std::string>> m_channel_names
      { this, "channel", {}, "Which channel to run" };

    std::vector<HHBBVV::Channel> m_channels;

    Gaudi::Property<bool> m_bypass
      { this, "bypass", false, "Run selector algorithm in pass-through mode" };

    /// \brief Setup syst-aware input container handles

    CP::SysListHandle m_systematicsList {this};
    CutManager m_bbVVCuts;

    CP::SysReadHandle<xAOD::JetContainer>
      m_jetHandle{ this, "jets", "bbVVAnalysisJets_%SYS%", "Jet container to read" };

    CP::SysReadHandle<xAOD::JetContainer>
      m_lrjetHandle{ this, "lrjets", "bbVVAnalysisLRJets_%SYS%", "Large-R jet container to read" };
    
    CP::SysReadDecorHandle<float> m_Tau2_wta = {this, "tau2", "Tau2_wta", "Tau2_wta"};
    CP::SysReadDecorHandle<float> m_Tau4_wta = {this, "tau4", "Tau4_wta", "Tau4_wta"};

    Gaudi::Property<std::vector<std::string>> m_GN2X_wps
      { this, "GN2X_WPs", {}, "GN2X_hbb_wps from the bbVV config" };
    CP::SysReadDecorHandle<bool> m_Pass_GN2X{"", this}; // Select the most loose WP in HHbbVVSelectorAlg

    CP::SysReadDecorHandle<bool> 
    m_Pass_GN2X_FlatMassQCDEff_0p58 {this, "GN2X_PassFlatMassQCDEff_0p58", "GN2X_select_FlatMassQCDEff_0p58", "GN2X_select_FlatMassQCDEff_0p58 selection"};
    
    CP::SysReadHandle<xAOD::ElectronContainer>
    m_electronHandle{ this, "electrons", "bbVVAnalysisElectrons_%SYS%", "Electron container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
    m_muonHandle{ this, "muons", "bbVVAnalysisMuons_%SYS%", "Muon container to read" };

    CP::SysReadHandle<xAOD::MissingETContainer>
    m_metHandle{ this, "met", "AnalysisMET_%SYS%", "MET container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    CP::SysReadDecorHandle<char> 
    m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};

    Gaudi::Property<std::string> m_eleWPName
      { this, "eleWP", "","Electron ID + Iso working point" };
    CP::SysReadDecorHandle<char> m_eleWPDecorHandle{"", this};

    Gaudi::Property<std::string> m_muonWPName
      { this, "muonWP", "","Muon ID + Iso cuts" };
    CP::SysReadDecorHandle<char> m_muonWPDecorHandle{"", this};


    /// \brief Setup sys-aware output decorations
    CP::SysWriteDecorHandle<bool> m_pass_sr {"pass_bbVV_sr_%SYS%", this};

    CP::SysFilterReporterParams m_filterParams {this, "HHbbVV selection"};

    CP::SysWriteDecorHandle<bool> m_selected_el {"selected_el_%SYS%", this};
    CP::SysWriteDecorHandle<bool> m_selected_mu {"selected_mu_%SYS%", this};

    CP::SysWriteDecorHandle<bool> m_Whad {"Whad_%SYS%", this};
    CP::SysWriteDecorHandle<bool> m_Whad2 {"Whad2_%SYS%", this};
    CP::SysWriteDecorHandle<bool> m_Hbb {"Hbb_%SYS%", this};
    
    /// \brief Internal variables
    // TODO: implement internal and relevant bbVV variables here
    bool m_run_lep = false;
    bool m_run_had = false;

    // counter vars (for debugging)
    int Jet_evt = 0; // Need to decide whether bbVV needs AT_LEAST_TWO_JETS
    long long int n_evt = 0;

    const double m_Wmass = 80.379 * Athena::Units::GeV; // This is the W mass used in earlier study
    const double m_Hmass = 125.09 * Athena::Units::GeV; // This is the H mass used in earlier study
    const double m_Smass = 300.0 * Athena::Units::GeV; // This is the S mass used in the SH sample


    Gaudi::Property<std::vector<std::string>> m_inputCutList{this, "cutList", {}};
    bool m_saveCutFlow = true;

    CP::SysWriteDecorHandle<bool> m_passallcuts {"PassAllCuts_%SYS%", this};
    
  };
}

#endif
