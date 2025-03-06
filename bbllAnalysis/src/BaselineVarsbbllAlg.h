/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HHBBLLANALYSIS_FINALVARSBBLLALG
#define HHBBLLANALYSIS_FINALVARSBBLLALG

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
    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::MissingETContainer>
    m_metHandle{ this, "met", "AnalysisMET_%SYS%", "MET container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    Gaudi::Property<std::vector<std::string>> m_floatVariables
          {this, "floatVariableList", {}, "Name list of floating variables"};

    Gaudi::Property<std::vector<std::string>> m_intVariables
          {this, "intVariableList", {}, "Name list of integer variables"};

    CP::SysReadDecorHandle<float>
    m_met_sig {this, "METSignificance", "significance", "Met Significance"};

    Gaudi::Property<bool> m_save_extra_vars
      { this, "save_extra_vars", false, "Compute quantities which may be useful." };

    CP::SysReadDecorHandle<float> m_Lepton1_pt
      { this, "Lepton1_pt", "Lepton1_pt_%SYS%", "Lepton 1 pT decoration" };
    CP::SysReadDecorHandle<float> m_Lepton1_eta 
      { this, "Lepton1_eta", "Lepton1_eta_%SYS%", "Lepton 1 eta decoration" };
    CP::SysReadDecorHandle<float> m_Lepton1_phi 
      { this, "Lepton1_phi", "Lepton1_phi_%SYS%", "Lepton 1 phi decoration" };
    CP::SysReadDecorHandle<float> m_Lepton1_E 
      { this, "Lepton1_E", "Lepton1_E_%SYS%", "Lepton 1 Energy decoration" };
    
    CP::SysReadDecorHandle<float> m_Lepton2_pt 
      { this, "Lepton2_pt", "Lepton2_pt_%SYS%", "Lepton 2 pT decoration" };
    CP::SysReadDecorHandle<float> m_Lepton2_eta 
      { this, "Lepton2_eta", "Lepton2_eta_%SYS%", "Lepton 2 eta decoration" };
    CP::SysReadDecorHandle<float> m_Lepton2_phi 
      { this, "Lepton2_phi", "Lepton2_phi_%SYS%", "Lepton 2 phi decoration" };
    CP::SysReadDecorHandle<float> m_Lepton2_E 
      { this, "Lepton2_E", "Lepton2_E_%SYS%", "Lepton 2 Energy decoration" };

    CP::SysReadDecorHandle<int> m_nLeptons
      { this, "nPhotons", "nLeptons_%SYS%", "Number of leptons decoration" };

    CP::SysReadDecorHandle<float> m_Jet_b1_pt 
      { this, "Jet_b1_pt", "Jet_b1_pt_%SYS%", "b-jet 1 pT decoration" };
    CP::SysReadDecorHandle<float> m_Jet_b1_eta 
      { this, "Jet_b1_eta", "Jet_b1_eta_%SYS%", "b-jet 1 eta decoration" };
    CP::SysReadDecorHandle<float> m_Jet_b1_phi 
      { this, "Jet_b1_phi", "Jet_b1_phi_%SYS%", "b-jet 1 phi decoration" };
    CP::SysReadDecorHandle<float> m_Jet_b1_E 
      { this, "Jet_b1_E", "Jet_b1_E_%SYS%", "b-jet 1 Energy decoration" };
    
    CP::SysReadDecorHandle<float> m_Jet_b2_pt 
      { this, "Jet_b2_pt", "Jet_b2_pt_%SYS%", "b-jet 2 pT decoration" };
    CP::SysReadDecorHandle<float> m_Jet_b2_eta 
      { this, "Jet_b2_eta", "Jet_b2_eta_%SYS%", "b-jet 2 eta decoration" };
    CP::SysReadDecorHandle<float> m_Jet_b2_phi 
      { this, "Jet_b2_phi", "Jet_b2_phi_%SYS%", "b-jet 2 phi decoration" };
    CP::SysReadDecorHandle<float> m_Jet_b2_E 
      { this, "Jet_b2_E", "Jet_b2_E_%SYS%", "b-jet 2 Energy decoration" };

    CP::SysReadDecorHandle<int> m_bjets
      { this, "nBJets", "nBJets_%SYS%", "Number of bjets decoration" };


    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>>
        m_Fbranches;

    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;
    
 };
}
#endif
