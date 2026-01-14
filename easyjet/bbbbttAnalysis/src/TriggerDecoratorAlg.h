/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

  TriggerDecoratorAlg:
  An alg that saves the event-level + tau-trigger matching info as decorations
  Required for antiTau definition in TauDecoratorAlg
*/


// Always protect against multiple includes!
#ifndef BBBBTT_TRIGGERDECORATORALG
#define BBBBTT_TRIGGERDECORATORALG

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODTau/TauJetContainer.h>

#include "TriggerMatchingTool/IMatchingTool.h"

#include "HHHbbbbttEnums.h"

namespace HHHBBBBTT
{
  
  class TriggerDecoratorAlg final : public AthReentrantAlgorithm
  {
    
  public:
    TriggerDecoratorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) const override;

  private:

    const std::unordered_map<HHHBBBBTT::TriggerChannel, std::string> m_triggerChannels =
      {
	{HHHBBBBTT::SLT, "SLT"},
	{HHHBBBBTT::LTT, "LTT"},
	{HHHBBBBTT::ETT, "ETT"},
	{HHHBBBBTT::ETT_4J12, "ETT_4J12"},
	{HHHBBBBTT::MTT_2016, "MTT_2016"},
	{HHHBBBBTT::MTT_high, "MTT_high"},
	{HHHBBBBTT::MTT_low, "MTT_low"},
	{HHHBBBBTT::STT, "STT"},
	{HHHBBBBTT::DTT, "DTT"},
	{HHHBBBBTT::DTT_2016, "DTT_2016"},
	{HHHBBBBTT::DTT_4J12, "DTT_4J12"},
	{HHHBBBBTT::DTT_L1Topo, "DTT_L1Topo"},
	{HHHBBBBTT::DTT_4J12_delayed, "DTT_4J12_delayed"},
	{HHHBBBBTT::DTT_L1Topo_delayed, "DTT_L1Topo_delayed"},
	{HHHBBBBTT::DBT, "DBT"},
  {HHHBBBBTT::BTT, "BTT"},
	{HHHBBBBTT::trigMatch_Tau35, "Tau35"},
	{HHHBBBBTT::trigMatch_Tau25, "Tau25"}
      };

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey
      { this, "event", "EventInfo", "EventInfo to read" };

    SG::ReadDecorHandleKey<xAOD::EventInfo> m_yearKey;

    const std::unordered_map<HHHBBBBTT::RunBooleans, std::string> m_runBooleans =
      {
	{HHHBBBBTT::is16PeriodA, "is2016_periodA"},
	{HHHBBBBTT::is16PeriodB_D3, "is2016_periodB_D3"},
	{HHHBBBBTT::is16PeriodD4_end, "is2016_periodD4_end"},
	{HHHBBBBTT::is17PeriodB1_B4, "is2017_periodB1_B4"},
	{HHHBBBBTT::is17PeriodB5_B7, "is2017_periodB5_B7"},
	{HHHBBBBTT::is17PeriodB8_end, "is2017_periodB8_end"},
	{HHHBBBBTT::is18PeriodB_end, "is2018_periodB_end"},
	{HHHBBBBTT::is18PeriodK_end, "is2018_periodK_end"},
	{HHHBBBBTT::is22_75bunches, "is2022_75bunches"},
	{HHHBBBBTT::is23_75bunches, "is2023_75bunches"},
	{HHHBBBBTT::is23_400bunches, "is2023_400bunches"},
	{HHHBBBBTT::is23_from1200bunches, "is2023_from1200bunches"},
	{HHHBBBBTT::is23_first_2400bunches, "is2023_first_2400bunches"},
	{HHHBBBBTT::is24_75bunches, "is2024_75bunches"},
	{HHHBBBBTT::is24_400bunches_periodD, "is2024_400bunches_periodD"},
	{HHHBBBBTT::is24_400bunches_periodL, "is2024_400bunches_periodL"},
	{HHHBBBBTT::l1topo_disabled, "l1TopoDisabled"},
      };
    std::map<HHHBBBBTT::RunBooleans, SG::ReadDecorHandleKey<xAOD::EventInfo>> m_runBooleans_key;

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    SG::ReadHandleKey<xAOD::MuonContainer> m_muonsKey
      { this, "muons", "", "Muon container" };

    SG::ReadHandleKey<xAOD::ElectronContainer> m_elesKey
      { this, "electrons", "", "Electrong container" };
    
    SG::ReadHandleKey<xAOD::TauJetContainer> m_tausKey
      { this, "taus", "", "Tau container" };

    Gaudi::Property<std::vector<std::string>> m_triggers 
      { this, "triggerLists", {}, "Name list of trigger" };
    std::unordered_map<std::string, SG::ReadDecorHandleKey<xAOD::EventInfo> >
      m_triggerdecoKeys;

    std::unordered_map<HHHBBBBTT::TriggerChannel,
      SG::WriteDecorHandleKey<xAOD::EventInfo> > m_pass_DecorKey;

    std::unordered_map<HHHBBBBTT::TriggerChannel,
      SG::WriteDecorHandleKey<xAOD::MuonContainer> > m_mu_trigMatch_DecorKey;
    std::unordered_map<HHHBBBBTT::TriggerChannel,
      SG::WriteDecorHandleKey<xAOD::ElectronContainer> > m_ele_trigMatch_DecorKey;
    std::unordered_map<HHHBBBBTT::TriggerChannel,
      SG::WriteDecorHandleKey<xAOD::TauJetContainer> > m_tau_trigMatch_DecorKey;

    ToolHandle<Trig::IMatchingTool> m_matchingTool
      { this, "trigMatchingTool", "", "Trigger matching tool"};

    Gaudi::Property<bool> m_useDiTauTrigMatch
      { this, "diTauTrigMatch", true, "Run di-tau trigger matching" };

    typedef std::unordered_map<HHHBBBBTT::RunBooleans, SG::ReadDecorHandle<xAOD::EventInfo, bool> > runBoolReadDecoMap;
    typedef std::unordered_map<std::string, SG::ReadDecorHandle<xAOD::EventInfo, bool> > trigReadDecoMap;
    typedef std::unordered_map<HHHBBBBTT::TriggerChannel, SG::WriteDecorHandle<xAOD::EventInfo, bool> > passWriteDecoMap;
    typedef std::unordered_map<HHHBBBBTT::TriggerChannel, SG::WriteDecorHandle<xAOD::MuonContainer, bool> > muTrigMatchWriteDecoMap;
    typedef std::unordered_map<HHHBBBBTT::TriggerChannel, SG::WriteDecorHandle<xAOD::ElectronContainer, bool> > eleTrigMatchWriteDecoMap;
    typedef std::unordered_map<HHHBBBBTT::TriggerChannel, SG::WriteDecorHandle<xAOD::TauJetContainer, bool> > tauTrigMatchWriteDecoMap;
    
    void checkSingleMuTriggers
      (int year, const xAOD::EventInfo* eventInfo,
       const runBoolReadDecoMap& runBoolDecos, const trigReadDecoMap& triggerdecos,
       passWriteDecoMap& pass_decos,
       const xAOD::MuonContainer* muons,
       muTrigMatchWriteDecoMap& mu_trigMatchDecos) const;

    void checkSingleEleTriggers
      (int year, const xAOD::EventInfo* eventInfo,
       const runBoolReadDecoMap& runBoolDecos, const trigReadDecoMap& triggerdecos,
       passWriteDecoMap& pass_decos,
       const xAOD::ElectronContainer* electrons,
       eleTrigMatchWriteDecoMap& ele_trigMatchDecos) const;

    void checkMuTauTriggers
      (int year, const xAOD::EventInfo* eventInfo,
       const runBoolReadDecoMap& runBoolDecos, const trigReadDecoMap& triggerdecos,
       passWriteDecoMap& pass_decos,
       const xAOD::MuonContainer* muons,
       muTrigMatchWriteDecoMap& mu_trigMatchDecos,
       const xAOD::TauJetContainer* taus,
       tauTrigMatchWriteDecoMap& tau_trigMatchDecos) const;

    void checkEleTauTriggers
      (int year, const xAOD::EventInfo* eventInfo,
       const runBoolReadDecoMap& runBoolDecos, const trigReadDecoMap& triggerdecos,
       passWriteDecoMap& pass_decos,
       const xAOD::ElectronContainer* electrons,
       eleTrigMatchWriteDecoMap& ele_trigMatchDecos,
       const xAOD::TauJetContainer* taus,
       tauTrigMatchWriteDecoMap& tau_trigMatchDecos) const;

    void checkSingleTauTriggers
      (int year, const xAOD::EventInfo* eventInfo,
       const runBoolReadDecoMap& runBoolDecos, const trigReadDecoMap& triggerdecos,
       passWriteDecoMap& pass_decos,
       const xAOD::TauJetContainer* taus,
       tauTrigMatchWriteDecoMap& tau_trigMatchDecos) const;

    void checkDiTauTriggers
      (int year, const xAOD::EventInfo* eventInfo,
       const runBoolReadDecoMap& runBoolDecos, const trigReadDecoMap& triggerdecos,
       passWriteDecoMap& pass_decos,
       const xAOD::TauJetContainer* taus,
       tauTrigMatchWriteDecoMap& tau_trigMatchDecos) const;

     void checkDiBJetTriggers
      (int year, const xAOD::EventInfo* eventInfo,
       const runBoolReadDecoMap& runBoolDecos, const trigReadDecoMap& triggerdecos,
       passWriteDecoMap& pass_decos) const;

     void checkBjetTauTriggers
      (int year, const xAOD::EventInfo* eventInfo,
       const runBoolReadDecoMap& runBoolDecos, const trigReadDecoMap& triggerdecos,
       passWriteDecoMap& pass_decos) const;
    
  };
}

#endif





