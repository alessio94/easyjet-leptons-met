/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef ZTAUTAUCALIB_FINALVARSXBBCALIBALG
#define ZTAUTAUCALIB_FINALVARSXBBCALIBALG

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <AsgDataHandles/ReadDecorHandle.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <string>
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODTau/TauJetContainer.h>
#include <xAODEgamma/PhotonContainer.h>

#include "TriggerMatchingTool/IMatchingTool.h"

namespace XBBCALIB
{

  /// \brief An algorithm for counting containers
  class BaselineVarsZtautauCalibAlg final : public AthReentrantAlgorithm
  {
    /// \brief The standard constructor
public:
    BaselineVarsZtautauCalibAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute(const EventContext& ctx) const override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

    
private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::JetContainer>
    m_lRjetHandle{ this, "lrjets", "XbbCalibLRJets_%SYS%",   "Large-R jet container to read" };

    CP::SysReadHandle<xAOD::TauJetContainer>
    m_tauHandle{ this, "taus", "XbbCalibTaus_%SYS%",   "Tau container to read" };

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


    Gaudi::Property<std::map<std::string, std::vector<std::string>>> m_triggerlist
          {this, "triggerlist", {}, "Trigger list for each year"};


    ToolHandle<Trig::IMatchingTool> m_matchingTool { this, "trigMatchingTool", "", "Trigger matching tool"};
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey { this, "event", "EventInfo", "EventInfo to read" };
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_yearKey;
  
    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_Fbranches;
    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;

    void getTriggers(int year, const std::basic_string<char>& key_word, std::vector<std::string>& triggers) const;

  };
}

#endif
