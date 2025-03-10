/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef ZBBYCALIB_FINALVARSXBBCALIBALG
#define ZBBYCALIB_FINALVARSXBBCALIBALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/PhotonContainer.h>

namespace XBBCALIB
{

  /// \brief An algorithm for counting containers
  class BaselineVarsZbbyCalibAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    BaselineVarsZbbyCalibAlg(const std::string &name, ISvcLocator *pSvcLocator);

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
    m_jetHandle{ this, "jets", "XbbCalibJets_%SYS%",   "Jet container to read" };

    CP::SysReadHandle<xAOD::JetContainer>
    m_lrjetHandle{ this, "lrjets", "ZcandLRJets_%SYS%",   "Large-R jet container to read" };

    CP::SysReadHandle<xAOD::PhotonContainer>
    m_photonHandle{ this, "photons", "XbbCalibPhotons_%SYS%", "Photon container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    Gaudi::Property<std::vector<std::string>> m_floatVariables
          {this, "floatVariableList", {}, "Name list of floating variables"};

    Gaudi::Property<std::vector<std::string>> m_intVariables
          {this, "intVariableList", {}, "Name list of integer variables"};

    //GN2X
    Gaudi::Property<std::vector<std::string>> m_GN2X_wps
      { this, "GN2X_WPs", {}, "GN2X_hbb_wps from the Zbb+y config" };
    std::vector<CP::SysReadDecorHandle<int>> m_GN2X_wp_Handles;

    CP::SysReadDecorHandle<float> m_WTag_score{"", this};
    CP::SysReadDecorHandle<float> m_GN2Xv01_phbb{"GN2Xv01_phbb", this};
    CP::SysReadDecorHandle<float> m_GN2Xv01_phcc{"GN2Xv01_phcc", this};
    CP::SysReadDecorHandle<float> m_GN2Xv01_pqcd{"GN2Xv01_pqcd", this};
    CP::SysReadDecorHandle<float> m_GN2Xv01_ptop{"GN2Xv01_ptop", this};

    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_Fbranches;

    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;

  };
}

#endif
