/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HHBBYYANALYSIS_FINALVARSYYBBALG
#define HHBBYYANALYSIS_FINALVARSYYBBALG

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <AthContainers/ConstDataVector.h>

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <FourMomUtils/xAODP4Helpers.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODEgamma/PhotonContainer.h>
#include <xAODMissingET/MissingETContainer.h>

namespace HHBBYY
{

  /// \brief An algorithm for counting containers
  class BaselineVarsbbyyAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
  public:
    BaselineVarsbbyyAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

    float compute_Topness(const xAOD::JetContainer *jets);
    float* compute_EventShapes(std::unique_ptr<ConstDataVector<xAOD::JetContainer>> &bjets, const xAOD::PhotonContainer *photons);
    float compute_pTBalance(std::unique_ptr<ConstDataVector<xAOD::JetContainer>> &bjets, const xAOD::PhotonContainer *photons);

  private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::JetContainer>
    m_jetHandle{ this, "jets", "",   "Jet container to read" };

    CP::SysReadDecorHandle<char> 
    m_isBtag {this, "bTagWPDecorName", "", "Name of input decorator for b-tagging"};

    CP::SysReadDecorHandle<int> 
    m_PCBT {this, "PCBTDecorName", "", "Name of pseudo-continuous b-tagging decorator"};

    CP::SysReadHandle<xAOD::PhotonContainer>
    m_photonHandle{ this, "photons", "",   "Photons container to read" };

    CP::SysReadHandle<xAOD::ElectronContainer>
    m_electronHandle{ this, "electrons", "",   "Electron container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
    m_muonHandle{ this, "muons", "",   "Muon container to read" };

    Gaudi::Property<std::string> m_photonWPName
      { this, "photonWP", "","Photon ID + Iso working point" };
    CP::SysReadDecorHandle<float> m_ph_SF{"", this};

    CP::SysReadHandle<xAOD::MissingETContainer>
    m_metHandle{ this, "met", "",   "MET container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };
    
    Gaudi::Property<std::vector<std::string>> m_floatVariables
      {this, "floatVariableList", {}, "Name list of float variables"};
    
    Gaudi::Property<std::vector<std::string>> m_intVariables
      {this, "intVariableList", {}, "Name list of integer variables"};

    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_Fbranches;

    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;
    
  };
}
#endif
