/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef MONOJETANALYSIS_FINALVARSMONOJETALG
#define MONOJETANALYSIS_FINALVARSMONOJETALG

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

namespace MONOJET
{

  /// \brief An algorithm for counting containers
  class BaselineVarsMonojetAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    BaselineVarsMonojetAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};
    
    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::JetContainer>
      m_jetHandle{ this, "jets", "MonojetAnalysisJets_%SYS%",   "Jet container to read" };

    CP::SysReadHandle<xAOD::JetContainer>
    m_largejetHandle{ this, "largejets", "MonojetAnalysisLargeJets_%SYS%", "Jet container to read" };

    CP::SysReadHandle<xAOD::ElectronContainer>
    m_electronHandle{ this, "electrons", "MonojetAnalysisElectrons_%SYS%",   "Electron container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
    m_muonHandle{ this, "muons", "MonojetAnalysisMuons_%SYS%",   "Muon container to read" };

    CP::SysReadHandle<xAOD::TauJetContainer>
    m_tauHandle{ this, "taus", "MonojetAnalysisTaus_%SYS%", "Tau container to read" };

    CP::SysReadHandle<xAOD::MissingETContainer>
    m_metHandle{ this, "met", "AnalysisMET_%SYS%",   "MET container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    Gaudi::Property<std::string> m_eleWPName
      { this, "eleWP", "","Electron ID + Iso working point" };
    CP::SysReadDecorHandle<float> m_ele_SF{"", this};

    Gaudi::Property<std::string> m_muWPName
      { this, "muonWP", "","Muon ID + Iso working point" };

    Gaudi::Property<std::string> m_tauWPName
      { this, "tauWP", "","tau ID working point" };
    CP::SysReadDecorHandle<float> m_tau_SF{"", this};

    CP::SysReadDecorHandle<char>
    m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};
    
    CP::SysReadDecorHandle<int> 
    m_PCBT {this, "PCBTDecorName", "", "Name of pseudo-continuous b-tagging decorator"};

    Gaudi::Property<std::vector<std::string>> m_btagWPs
          {this, "btagWPs", {}, "List of WPs to count bjets with"};

    CP::SysReadDecorHandle<int> m_truthFlav{ this, "truthFlav", "HadronConeExclTruthLabelID", "Jet truth flavor" };

    CP::SysReadDecorHandle<float> m_GN2Xv01_phbb{this, "phbb", "GN2Xv01_phbb", "GN2Xv01_phbb"};
    CP::SysReadDecorHandle<float> m_GN2Xv01_phcc{this, "phcc", "GN2Xv01_phcc", "GN2Xv01_phcc"};
    CP::SysReadDecorHandle<float> m_GN2Xv01_pqcd{this, "pqcd", "GN2Xv01_pqcd", "GN2Xv01_pqcd"};
    CP::SysReadDecorHandle<float> m_GN2Xv01_ptop{this, "ptop", "GN2Xv01_ptop", "GN2Xv01_ptop"};
	
    CP::SysReadDecorHandle<char> m_TightClean{this,"tightclean", "DFCommonJets_jetClean_TightBad","Tight cleaning flag"};

    // jss
    std::vector<std::string> m_JSS_list = {"ECF1", "ECF2", "ECF3", "Split12", "Split23", "Tau1_wta", "Tau2_wta", "Tau3_wta", "Qw"};
    std::unordered_map<std::string, CP::SysReadDecorHandle<float>> m_JSS;


    Gaudi::Property<std::vector<std::string>> m_floatVariables
          {this, "floatVariableList", {}, "Name list of floating variables"};

    Gaudi::Property<std::vector<std::string>> m_intVariables
          {this, "intVariableList", {}, "Name list of integer variables"};

    CP::SysReadDecorHandle<float> m_METSig 
      {this, "METSignificance", "significance_%SYS%", "Met Significance"};

    Gaudi::Property<float> m_max_eta_central_jet
      {this, "max_eta_central_jet", 2.5, "Max pseudorapidity central jets / Min pseudorapidity forward jets"};

    Gaudi::Property<int> m_Small_R_jet_amount 
      {this, "Small_R_jet_amount", 1, "Number of Small R jets to keep in ntuple"};

    Gaudi::Property<int> m_Large_R_jet_amount 
      {this, "Large_R_jet_amount", 1, "Number of Large R jets to keep in ntuple"};

    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_Fbranches;
    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;

 };
}
#endif
