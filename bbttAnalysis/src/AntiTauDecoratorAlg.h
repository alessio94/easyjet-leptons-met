/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BBTTANALYSIS_ANTITAUDECORATORALG
#define BBTTANALYSIS_ANTITAUDECORATORALG

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODTau/TauJetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>

#include "HHbbttEnums.h"

namespace HHBBTT
{

  /// \brief An algorithm for counting containers
  class AntiTauDecoratorAlg final : public AthReentrantAlgorithm
  {
    /// \brief The standard constructor
  public:
    AntiTauDecoratorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute(const EventContext& ctx) const override;
    /// We use default finalize() -- this is for cleanup, and we don't do any
    
 

  private:
    
    // Members for configurable properties
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey
      { this, "event", "EventInfo", "EventInfo to read" };
    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    SG::ReadDecorHandleKey<xAOD::EventInfo> m_yearKey;

    SG::ReadDecorHandleKey<xAOD::EventInfo> m_is2016_periodA_key;
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_is2016_periodB_D3_key;
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_is2022_75bunches_key;

    SG::ReadDecorHandleKey<xAOD::EventInfo> m_passSLTDecorKey;
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_passLTTDecorKey;
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_passSTTDecorKey;
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_passDTTDecorKey;

    // Taus
    SG::ReadHandleKey<xAOD::TauJetContainer> m_tausInKey
      { this, "tausIn", "", "containerName to read" };

    Gaudi::Property<std::string> m_tauIDWP_name
      { this, "tauIDWP", "", "Name of the Tau ID WP" };
    xAOD::TauJetParameters::IsTauFlag m_tauIDWP;

    Gaudi::Property<double> m_antiTauRNNThreshold
      { this, "antiTauRNNThreshold", 0.01,
	"Lower threshold of RNN score for Anti-Id taus" };
    
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_triggerMatchSTTKey;
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_triggerMatchLTTKey;
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_triggerMatchDTTKey;
    
    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_IDTauDecorKey;
    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_antiTauDecorKey;
    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_eventCategoryDecorKey;

    // Muons
    SG::ReadHandleKey<xAOD::MuonContainer> m_muonsInKey
      { this, "muonsIn", "", "containerName to read" };
    Gaudi::Property<std::string> m_muonIdDecorName
      { this, "muonIdDecorKey", "DFCommonMuonPassIDCuts",
	"Decoration for muon ID cuts" };
    Gaudi::Property<std::string> m_muonPreselDecorName
      { this, "muonPreselDecorKey", "DFCommonMuonPassPreselection",
	"Decoration for muon preselection" };
    
    SG::ReadDecorHandleKey<xAOD::MuonContainer> m_muonIdDecorKey;
    SG::ReadDecorHandleKey<xAOD::MuonContainer> m_muonPreselDecorKey;

    // Electrons
    SG::ReadHandleKey<xAOD::ElectronContainer> m_elesInKey
      { this, "elesIn", "", "containerName to read" };
    Gaudi::Property<std::string> m_eleIdDecorName
      { this, "eleIdDecorKey", "DFCommonElectronsLHTight",
	  "Decoration for electron ID working point" };

    SG::ReadDecorHandleKey<xAOD::ElectronContainer> m_eleIdDecorKey;

    void setThresholds
    (unsigned int year,
     bool is2016_periodA, bool is2016_periodB_D3,
     bool is2022_75bunches,
     std::unordered_map<HHBBTT::TriggerChannel, std::unordered_map<HHBBTT::Var, float>>& ptThresholdMap) const;
  };
}

#endif
