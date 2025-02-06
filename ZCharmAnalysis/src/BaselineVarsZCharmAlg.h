/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef ZCHARMANALYSIS_BASELINEVARSZCHARMALG
#define ZCHARMANALYSIS_BASELINEVARSZCHARMALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMissingET/MissingETContainer.h>
#include "ZCharmEnums.h"

namespace ZCC
{

  /// \brief An algorithm for counting containers
  class BaselineVarsZCharmAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    BaselineVarsZCharmAlg(const std::string &name, ISvcLocator *pSvcLocator);

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
      m_ZCharmJetHandle{ this, "ZCharmJets", "ZCharmAnalysisJets_%SYS%", "Jet container to read" };
    
    CP::SysReadHandle<xAOD::JetContainer>
      m_ZCharmLRJetHandle{ this, "ZCharmLRJets", "ZCharmAnalysisLargeJets_%SYS%", "Jet container to read" };
    
    CP::SysReadHandle<xAOD::ElectronContainer>
      m_ZCharmElectronHandle{ this, "ZCharmElectrons", "ZCharmAnalysisElectrons_%SYS%", "Electron container to read" };
    CP::SysReadHandle<xAOD::ElectronContainer>
      m_electronHandle{ this, "electrons", "AnalysisElectrons_%SYS%", "Original electron container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
      m_ZCharmMuonHandle{ this, "ZCharmMuons", "ZCharmAnalysisMuons_%SYS%", "Muon container to read" };
    CP::SysReadHandle<xAOD::MuonContainer>
      m_muonHandle{ this, "muons", "AnalysisMuons_%SYS%", "Original muon container to read" };

    CP::SysReadHandle<xAOD::MissingETContainer>
    m_metHandle{ this, "met", "AnalysisMET_%SYS%", "MET container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    Gaudi::Property<std::string> m_eleWPName
      { this, "eleWP", "","Electron ID + Iso working point" };
    CP::SysReadDecorHandle<float> m_ele_SF{"", this};
    Gaudi::Property<bool> m_saveDummy_ele_SF
      {this, "saveDummyEleSF", false,
	  "To be used in case no recommendations are not available"};

    CP::SysReadDecorHandle<int> m_ele_truthOrigin{"truthOrigin", this};
    CP::SysReadDecorHandle<int> m_ele_truthType{"truthType", this};

    Gaudi::Property<std::string> m_muWPName
      { this, "muonWP", "","Muon ID + Iso working point" };
    CP::SysReadDecorHandle<float> m_mu_SF{"", this};
    CP::SysReadDecorHandle<int> m_mu_truthOrigin{"truthOrigin", this};
    CP::SysReadDecorHandle<int> m_mu_truthType{"truthType", this};

    CP::SysReadDecorHandle<char> 
    m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};
    
    Gaudi::Property<std::vector<std::string>> m_PCBTnames
      {this, "PCBTDecorList", {}, "Name list of pseudo-continuous b-tagging decorator"};
    std::unordered_map<std::string, CP::SysReadDecorHandle<int>> m_PCBTs;


    CP::SysReadDecorHandle<int> m_truthFlav{ this, "truthFlav", "HadronConeExclTruthLabelID", "Jet truth flavor" };

    CP::SysReadDecorHandle<float> m_GN2Xv01_phbb = {this, "phbb", "GN2Xv01_phbb", "GN2Xv01_phbb"};
    CP::SysReadDecorHandle<float> m_GN2Xv01_phcc = {this, "phcc", "GN2Xv01_phcc", "GN2Xv01_phcc"};
    CP::SysReadDecorHandle<float> m_GN2Xv01_pqcd = {this, "pqcd", "GN2Xv01_pqcd", "GN2Xv01_pqcd"};
    CP::SysReadDecorHandle<float> m_GN2Xv01_ptop = {this, "ptop", "GN2Xv01_ptop", "GN2Xv01_ptop"};

    Gaudi::Property<std::vector<std::string>> m_floatVariables
          {this, "floatVariableList", {}, "Name list of floating variables"};

    Gaudi::Property<std::vector<std::string>> m_intVariables
          {this, "intVariableList", {}, "Name list of integer variables"};

    CP::SysReadDecorHandle<float>
    m_METSig {this, "METSignificance", "significance", "Met Significance"};

    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>>
        m_Fbranches;

    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;
    
 };
}
#endif
