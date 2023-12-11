/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
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
    "TWO_ISO_ELECTRONS", "TWO_ISO_MUONS", "EXACTLY_TWO_B_JETS", "Pass_ee", "Pass_mumu", "Mee", "Mmumu",
    "Leading_Electron_pt", "Leading_Electron_eta", "Leading_Electron_phi", "Leading_Electron_E", "Leading_Electron_SF",
    "Subleading_Electron_pt", "Subleading_Electron_eta", "Subleading_Electron_phi", "Subleading_Electron_E", "Subleading_Electron_SF",
    "Leading_Muon_pt", "Leading_Muon_eta", "Leading_Muon_phi", "Leading_Muon_E", "Leading_Muon_SF",
    "Subleading_Muon_pt", "Subleading_Muon_eta", "Subleading_Muon_phi", "Subleading_Muon_E", "Subleading_Muon_SF",
    "mee", "pTee", "dRee", "Etaee", "Phiee", "mmumu", "pTmumu", "dRmumu", "Etamumu", "Phimumu",
    "Jet_pt_b1", "Jet_eta_b1", "Jet_phi_b1", "Jet_E_b1", "Jet_pt_b2", "Jet_eta_b2", "Jet_phi_b2", "Jet_E_b2",
    "mbb", "pTbb", "dRbb", "Etabb", "Phibb",         
    "Leading_Jet_pt", "Leading_Jet_eta", "Leading_Jet_phi", "Leading_Jet_E", 
    "Subleading_Jet_pt", "Subleading_Jet_eta", "Subleading_Jet_phi", "Subleading_Jet_E",
    };

   std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;
   std::vector<std::string> m_Ivarnames{
   "nBJets", "nMuons", "nElectrons", "nJets",
   };
    
 };
}
#endif
