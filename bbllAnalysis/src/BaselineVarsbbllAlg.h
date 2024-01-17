/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HHBBLLANALYSIS_FINALVARSBBLLALG
#define HHBBLLANALYSIS_FINALVARSBBLLALG

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <AthContainers/ConstDataVector.h>

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <FourMomUtils/xAODP4Helpers.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMissingET/MissingETContainer.h>

namespace HHBBLL
{

  /// \brief An algorithm for counting containers
  class BaselineVarsbbllAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    BaselineVarsbbllAlg(const std::string &name, ISvcLocator *pSvcLocator);

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
      m_jetHandle{ this, "jets", "",   "Jet container to read" };
    
    CP::SysReadHandle<xAOD::ElectronContainer>
    m_electronHandle{ this, "electrons", "",   "Electron container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
    m_muonHandle{ this, "muons", "",   "Muon container to read" };

    CP::SysReadHandle<xAOD::MissingETContainer>
    m_metHandle{ this, "met", "AnalysisMET",   "MET container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    Gaudi::Property<std::string> m_eleWPName
      { this, "eleWP", "","Electron ID + Iso working point" };
    CP::SysReadDecorHandle<float> m_ele_recoSF{"", this};
    CP::SysReadDecorHandle<float> m_ele_idSF{"", this};
    CP::SysReadDecorHandle<float> m_ele_isoSF{"", this};

    Gaudi::Property<std::string> m_muWPName
      { this, "muonWP", "","Muon ID + Iso working point" };
    CP::SysReadDecorHandle<float> m_mu_recoSF{"", this};
    CP::SysReadDecorHandle<float> m_mu_isoSF{"", this};

    CP::SysReadDecorHandle<char> 
    m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};


    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float> > m_Fbranches;
    std::vector<std::string> m_Fvarnames{
    "Electron1_pt", "Electron1_eta", "Electron1_phi", "Electron1_E",
    "Electron2_pt", "Electron2_eta", "Electron2_phi", "Electron2_E",
    "Muon1_pt", "Muon1_eta", "Muon1_phi", "Muon1_E",
    "Muon2_pt", "Muon2_eta", "Muon2_phi", "Muon2_E",
    "Lepton1_pt", "Lepton1_eta", "Lepton1_phi", "Lepton1_E",
    "Lepton2_pt", "Lepton2_eta", "Lepton2_phi", "Lepton2_E",
    "mee", "pTee", "dRee", "Etaee", "Phiee", 
    "mmumu", "pTmumu", "dRmumu", "Etamumu", "Phimumu",
    "memu", "pTemu","dRemu", "Etaemu", "Phiemu",
    "mll", "pTll",
    "Jet_b1_pt", "Jet_b1_eta", "Jet_b1_phi", "Jet_b1_E",
    "Jet_b2_pt", "Jet_b2_eta", "Jet_b2_phi", "Jet_b2_E",
    "mbb", "pTbb", "dRbb", "Etabb", "Phibb",
    "Jet1_pt", "Jet1_eta", "Jet1_phi", "Jet1_E",
    "Jet2_pt", "Jet2_eta", "Jet2_phi", "Jet2_E",
    };

    std::vector<std::string> m_Fvarnames_MC{
      "Electron1_effSF", "Electron2_effSF",
      "Muon1_effSF", "Muon2_effSF",
      "Lepton1_effSF", "Lepton2_effSF",	
    };


   std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;
   std::vector<std::string> m_Ivarnames{
    "Lepton1_charge", "Lepton1_pdgid",
    "Lepton2_charge", "Lepton2_pdgid",
   "nBJets", "nMuons", "nElectrons", "nJets",
   };
    
 };
}
#endif
