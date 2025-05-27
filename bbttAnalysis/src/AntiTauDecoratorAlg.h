/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BBTTANALYSIS_ANTITAUDECORATORALG
#define BBTTANALYSIS_ANTITAUDECORATORALG

#include "AnaAlgorithm/AnaAlgorithm.h"

#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SelectionHelpers/SysWriteSelectionHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODTau/TauJetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>

#include "HHbbttEnums.h"

namespace HHBBTT
{

  /// \brief An algorithm for counting containers
  class AntiTauDecoratorAlg final : public EL::AnaAlgorithm
  {
    /// \brief The standard constructor
  public:
    AntiTauDecoratorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

  private:

    CP::SysListHandle m_systematicsList {this};
    
    // Members for configurable properties
    CP::SysReadHandle<xAOD::EventInfo> m_eventHandle
      { this, "event", "EventInfo", "EventInfo to read" };

    CP::SysReadDecorHandle<unsigned int> m_year{"dataTakingYear", this};

    CP::SysReadDecorHandle<bool> m_is2016_periodA{"is2016_periodA", this};
    CP::SysReadDecorHandle<bool> m_is2016_periodB_D3{"is2016_periodB_D3", this};
    CP::SysReadDecorHandle<bool> m_is2022_75bunches{"is2022_75bunches", this};

    CP::SysReadDecorHandle<bool> m_passSLT{"pass_trigger_SLT", this};
    CP::SysReadDecorHandle<bool> m_passLTT{"pass_trigger_LTT", this};
    CP::SysReadDecorHandle<bool> m_passSTT{"pass_trigger_STT", this};
    CP::SysReadDecorHandle<bool> m_passDTT{"pass_trigger_DTT", this};
    CP::SysReadDecorHandle<bool> m_passDBT{"pass_trigger_DBT", this};

    // Taus
    CP::SysReadHandle<xAOD::TauJetContainer>
      m_tauHandle{ this, "taus", "", "Tau container to read" };

    CP::SysReadSelectionHandle m_tauBaselineSelection {
      this, "tauBaselineSelection", "", "the preselection to be applied for all taus"
    };

    Gaudi::Property<std::string> m_tauIDWP_name
      { this, "tauIDWP", "", "Name of the Tau ID WP" };
    xAOD::TauJetParameters::IsTauFlag m_tauRNNWP;
    bool m_useGNTau = false;
    CP::SysReadDecorHandle<char> m_GNTau_sel{"", this};
    CP::SysReadDecorHandle<float> m_GNTau_score{"GNTauScoreSigTrans_v0prune", this};

    Gaudi::Property<double> m_antiTauScoreThreshold
      { this, "antiTauScoreThreshold", 0.,
	"Lower threshold of RNN/GNTau score for Anti-Id taus" };

    CP::SysReadDecorHandle<bool> m_triggerMatchSTT{"trigMatch_STT", this};
    CP::SysReadDecorHandle<bool> m_triggerMatchLTT{"trigMatch_LTT", this};
    CP::SysReadDecorHandle<bool> m_triggerMatchDTT{"trigMatch_DTT", this};

    CP::SysWriteSelectionHandle m_IDTau{
      this, "IDTauSelection", "", "decoration name for the ID taus"};
    CP::SysWriteSelectionHandle m_antiTau{
      this, "AntiTauSelection", "", "decoration name for the anti taus"};

    // Muons
    CP::SysReadHandle<xAOD::MuonContainer>
      m_muonHandle{ this, "muons", "", "Muon container to read" };

    CP::SysReadSelectionHandle m_muonSelection {
      this, "muonSelection", "", "the selection to be applied for muons"
    };

    // Electrons
    CP::SysReadHandle<xAOD::ElectronContainer>
      m_electronHandle{ this, "electrons", "",   "Electron container to read" };

    CP::SysReadSelectionHandle m_electronSelection {
      this, "electronSelection", "", "the selection to be applied for muons"
    };

    void setThresholds
    (unsigned int year,
     bool is2016_periodA, bool is2016_periodB_D3,
     bool is2022_75bunches,
     std::unordered_map<HHBBTT::TriggerChannel, std::unordered_map<HHBBTT::Var, float>>& ptThresholdMap) const;
  };
}

#endif
