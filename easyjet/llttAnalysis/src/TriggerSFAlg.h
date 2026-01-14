/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef LLTTANALYSIS_TRIGGERSFALG
#define LLTTANALYSIS_TRIGGERSFALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <AsgDataHandles/ReadDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>

#include "HllttChannels.h"

namespace HLLTT
{

  /// \brief An algorithm for counting containers
  class TriggerSFAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    TriggerSFAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::ElectronContainer>
      m_electronHandle{ this, "electrons", "llttAnalysisElectrons_%SYS%", "Electron container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
      m_muonHandle{ this, "muons", "llttAnalysisMuons_%SYS%", "Muon container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    CP::SysReadDecorHandle<bool> m_pass_trigger_SLT {this, "passTrigSLT", "", "pass trigger SLT"};
    CP::SysReadDecorHandle<bool> m_pass_trigger_DLT {this, "passTrigDLT", "", "pass trigger DLT"};

    Gaudi::Property<std::vector<std::string>> m_eleTrigSF
      {this, "eleTriggerSF", {}, "List of electron trigger SF"};
    std::unordered_map<std::string, CP::SysReadDecorHandle<float>> m_eleTriggerSF;

    Gaudi::Property<std::vector<std::string>> m_muonTrigSF
      {this, "muonTriggerSF", {}, "List of muon trigger SF"};
    std::unordered_map<std::string, CP::SysReadDecorHandle<float>> m_muonTriggerSF;

    CP::SysReadDecorHandle<bool> m_selected_el{"selected_el_%SYS%", this};
    CP::SysReadDecorHandle<bool> m_selected_mu {"selected_mu_%SYS%", this};
    
    CP::SysReadDecorHandle<unsigned int> m_year{this, "year", "dataTakingYear", ""};

    /// \brief Setup sys-aware output decorations
    CP::SysWriteDecorHandle<float> m_eventTriggerSF {"eventTriggerSF_%SYS%", this};

    void computeTriggerSF(const xAOD::EventInfo *event, const CP::SystematicSet& sys,
                          const xAOD::Electron* ele0, const xAOD::Muon* mu0);

    void getSingleMuTriggers(int year, std::vector<std::string>& single_mu_paths, std::string& single_mu_SF_path);

    void getSingleEleTriggers(int year, std::vector<std::string>& single_ele_paths, std::string& single_ele_SF_path);

  };
} // namespace HLLTT

#endif
