/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HZALLYYANALYSIS_FINALVARSHZALLYYALG
#define HZALLYYANALYSIS_FINALVARSHZALLYYALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODEgamma/PhotonContainer.h>

namespace HZALLYY
{
  
  /// \brief An algorithm for counting containers
  class BaselineVarsllyyAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
  public:
    BaselineVarsllyyAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any
    
  private:
    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};
    
    CP::SysReadHandle<xAOD::PhotonContainer>
      m_llyyphotonHandle{ this, "llyyphotons", "llyyAnalysisPhotons_%SYS%", "Photons container to read" };

     CP::SysReadHandle<xAOD::PhotonContainer>
      m_photonHandle{ this, "photons", "AnalysisPhotons_%SYS%", "Original Photons container to read" };
    
    Gaudi::Property<std::string> m_phWPName
      { this, "phWP", "", "Photon ID + Iso working point" };
    CP::SysReadDecorHandle<float> m_ph_SF{"", this};
   
    
    CP::SysReadHandle<xAOD::ElectronContainer>
      m_electronHandle{ this, "electrons", "AnalysisElectrons_%SYS%", "Original Electron container to read" };

    CP::SysReadHandle<xAOD::ElectronContainer>
      m_llyyelectronHandle{ this, "llyyelectrons", "llyyAnalysisElectrons_%SYS%", "Electron container to read" };
    
    CP::SysReadHandle<xAOD::MuonContainer>
      m_muonHandle{ this, "muons", "AnalysisMuons_%SYS%", "Original Muon container to read" };

     CP::SysReadHandle<xAOD::MuonContainer>
      m_llyymuonHandle{ this, "llyymuons", "llyyAnalysisMuons_%SYS%", "Muon container to read" };

     Gaudi::Property<bool> m_saveDummy_ele_SF
      {this, "saveDummyEleSF", false,
	  "To be used in case no recommendations are available"};

     Gaudi::Property<bool> m_saveDummy_ph_SF
      {this, "saveDummyPhotonSF", false,
	  "To be used in case no recommendations are available"};
     
    CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };
    
    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };
    
    Gaudi::Property<std::string> m_eleWPName
      { this, "eleWP", "","Electron ID + Iso working point" };
    CP::SysReadDecorHandle<float> m_ele_SF{"", this};
    CP::SysReadDecorHandle<int> m_ele_truthOrigin{"truthOrigin", this};
    CP::SysReadDecorHandle<int> m_ele_truthType{"truthType", this};
    
    Gaudi::Property<std::string> m_muWPName
      { this, "muonWP", "","Muon ID + Iso working point" };
    CP::SysReadDecorHandle<float> m_mu_SF{"", this};
    CP::SysReadDecorHandle<int> m_mu_truthOrigin{"truthOrigin", this};
    CP::SysReadDecorHandle<int> m_mu_truthType{"truthType", this};
    
    CP::SysReadDecorHandle<int> m_nmuons{"n_muons_%SYS%", this};

    Gaudi::Property<std::vector<std::string>> m_floatVariables
      {this, "floatVariableList", {}, "Name list of floating variables"};
    
    Gaudi::Property<std::vector<std::string>> m_intVariables
      {this, "intVariableList", {}, "Name list of integer variables"};
    
    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_Fbranches;
    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;
    
 };
}
#endif
