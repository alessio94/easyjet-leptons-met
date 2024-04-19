/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

  TauDecoratorAlg:
  An alg that copies tau information to aux decorations so can be
  output branch.
*/



// Always protect against multiple includes!
#ifndef EASYJET_TAUDECORATORALG
#define EASYJET_TAUDECORATORALG

#include <vector>
#include <utility>
#include <string>
#include <unordered_map>

#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <xAODEventInfo/EventInfo.h>

#include <xAODTau/TauJetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>

#include "ThresholdHelper.h"

namespace Easyjet
{

  /// \brief An algorithm for counting containers
  class TauDecoratorAlg final : public AthReentrantAlgorithm
  {
    /// \brief The standard constructor
public:
    TauDecoratorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute(const EventContext& ctx) const override;
    /// We use default finalize() -- this is for cleanup, and we don't do any
    
 

private:
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_passSLTDecorKey;
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_passLTTDecorKey;
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_passSTTDecorKey;
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_passDTTDecorKey;
    
    // Members for configurable properties
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey
      { this, "event", "EventInfo", "EventInfo to read" };
    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    SG::ReadDecorHandleKey<xAOD::EventInfo> m_yearKey;

    SG::ReadDecorHandleKey<xAOD::EventInfo> m_is2016_periodA_key;
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_is2016_periodB_D3_key;
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_is2022_75bunches_key;

    // Muons
    SG::ReadHandleKey<xAOD::MuonContainer> m_muonsInKey{
      this, "muonsIn", "", "containerName to read"
    };
    Gaudi::Property<std::string> m_muonIdDecorName
      { this, "muonIdDecorKey", "DFCommonMuonPassIDCuts",
	  "Decoration for muon ID cuts" };
    SG::ReadDecorHandleKey<xAOD::MuonContainer> m_muonIdDecorKey;

    Gaudi::Property<std::string> m_muonPreselDecorName
      { this, "muonPreselDecorKey", "DFCommonMuonPassPreselection",
	  "Decoration for muon preselection" };
    SG::ReadDecorHandleKey<xAOD::MuonContainer> m_muonPreselDecorKey;

    // Electrons
    SG::ReadHandleKey<xAOD::ElectronContainer> m_elesInKey{
      this, "elesIn", "", "containerName to read"
    };

    Gaudi::Property<std::string> m_eleIdDecorName
      { this, "eleIdDecorKey", "DFCommonElectronsLHTight",
	  "Decoration for electron ID working point" };
    SG::ReadDecorHandleKey<xAOD::ElectronContainer> m_eleIdDecorKey;
    
    // Taus
    SG::ReadHandleKey<xAOD::TauJetContainer> m_tausInKey{
      this, "tausIn", "", "containerName to read"
    };

    Gaudi::Property<std::string> m_nProngDecorName{
      this, "nProngDecorKey", "nProng", "Decoration for nProng"
    };
    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_nProngDecorKey;

    Gaudi::Property<std::string> m_decayModeDecorName{
      this, "decayMode", "decayMode", "Decoration for decayMode"
    };
    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_decayModeDecorKey;

    Gaudi::Property<std::string> m_truthTypeDecorName{
      this, "truthTypeDecorKey", "truthType", "Decoration for truthType"
    };
    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_truthTypeDecorKey;

    Gaudi::Property<std::string> m_IDTauDecorName{
      this, "idTauDecorKey", "isIDTau", "Decoration for ID taus"
    };
    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_IDTauDecorKey;

    Gaudi::Property<bool> m_doAntiTauDecor
      { this, "doAntiTauDecor", false, "Add anti-tau decoration" };
    Gaudi::Property<std::string> m_antiTauDecorName{
      this, "antiTauDecorKey", "isAntiTau", "Decoration for anti-taus"
    };
    Gaudi::Property<double> m_antiTauRNNThreshold{
      this, "antiTauRNNThreshold", 0.01, "Lower threshold of RNN score for Anti-Id taus"
    };
    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_antiTauDecorKey;

    Gaudi::Property<std::string> m_tauIDWP_name {this, "tauIDWP", "", "Name of the Tau ID WP"};
    xAOD::TauJetParameters::IsTauFlag m_tauIDWP;

    Gaudi::Property<std::string> m_eventCategoryDecorName{
      this, "eventCategoryDecorName", "antiTauEventCategory", "Decoration for (anti-Id) taus in hadhad events, 0 is for no anti-Tau, 1 for anti-Tau in lephad and 2 for anti-Tau in hadhad"
    };
    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_eventCategoryDecorKey;


    Gaudi::Property<std::string> m_triggerMatchSTTDecorName{
      this, "triggerMatchSTTDecorName", "trigMatch_STT", "Decoration for STT trigger matched (anti-Id) taus" //or maybe general objects?
    };
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_triggerMatchSTTKey;

    Gaudi::Property<std::string> m_triggerMatchLTTDecorName{
      this, "triggerMatchLTTDecorName", "trigMatch_LTT", "Decoration for LTT trigger matched (anti-Id) taus" //or maybe general objects?
    };
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_triggerMatchLTTKey;

    Gaudi::Property<std::string> m_triggerMatchDTTDecorName{
      this, "triggerMatchDTTDecorName", "trigMatch_DTT", "Decoration for DTT trigger matched (anti-Id) taus" //or maybe general objects?
    };
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_triggerMatchDTTKey;

    void setThresholds
    (unsigned int year,
     bool is2016_periodA, bool is2016_periodB_D3,
     bool is2022_75bunches,
     std::unordered_map<Easyjet::TriggerChannel, std::unordered_map<Easyjet::Var, float>>& ptThresholdMap) const;
  };
}

#endif
