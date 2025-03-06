/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HHBBLLANALYSIS_LEPTONVARSBBLLALG
#define HHBBLLANALYSIS_LEPTONVARSBBLLALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>

namespace HHBBLL
{

  class LeptonVarsbbllAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
  public:
    LeptonVarsbbllAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    StatusCode execute() override;
  private:
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::ElectronContainer>
      m_bbllElectronHandle{ this, "bbllElectrons", "bbllAnalysisElectrons_%SYS%", "Electron container to read" };
    CP::SysReadHandle<xAOD::ElectronContainer>
      m_electronHandle{ this, "electrons", "AnalysisElectrons_%SYS%", "Original electron container to read" };
    CP::SysReadHandle<xAOD::MuonContainer>
      m_bbllMuonHandle{ this, "bbllMuons", "bbllAnalysisMuons_%SYS%", "Muon container to read" };
    CP::SysReadHandle<xAOD::MuonContainer>
      m_muonHandle{ this, "muons", "AnalysisMuons_%SYS%", "Original muon container to read" };

    Gaudi::Property<std::string> m_eleWPName
      { this, "eleWP", "","Electron ID + Iso working point" };
    CP::SysReadDecorHandle<float> m_ele_SF{"", this};
    CP::SysReadDecorHandle<int> m_ele_truthOrigin{"truthOrigin", this};
    CP::SysReadDecorHandle<int> m_ele_truthType{"truthType", this};
    Gaudi::Property<bool> m_saveDummy_ele_SF
      {this, "saveDummyEleSF", false,
	  "To be used in case no recommendations are not available"};

    Gaudi::Property<std::string> m_muWPName
      { this, "muonWP", "","Muon ID + Iso working point" };
    CP::SysReadDecorHandle<float> m_mu_SF{"", this};
    CP::SysReadDecorHandle<int> m_mu_truthOrigin{"truthOrigin", this};
    CP::SysReadDecorHandle<int> m_mu_truthType{"truthType", this};

    CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    Gaudi::Property<std::vector<std::string>> m_floatVariables
          {this, "floatVariableList", {}, "Name list of floating variables"};

    Gaudi::Property<std::vector<std::string>> m_intVariables
          {this, "intVariableList", {}, "Name list of integer variables"};

    Gaudi::Property<bool> m_save_extra_vars
      { this, "save_extra_vars", false, "Compute quantities which may be useful." };

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    Gaudi::Property<bool> m_doSystematics
      { this, "doSystematics", false, "Run on all systematics" };

    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>>
        m_Fbranches;

    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;

  };
}
#endif
