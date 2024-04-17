/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

  TriggerDecoratorAlg:
  An alg that saves the event-level + tau-trigger matching info as decorations
  Required for antiTau definition in TauDecoratorAlg
*/


// Always protect against multiple includes!
#ifndef BBTT_TRIGGERDECORATORALG
#define BBTT_TRIGGERDECORATORALG

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

#include "HHbbttChannels.h"

namespace HHBBTT
{
  
  enum RunBooleans
  {
    is16PeriodA,
    is16PeriodB_D3,
    is16PeriodD4_end,
    is17PeriodB1_B4,
    is17PeriodB5_B7,
    is17PeriodB8_end,
    is18PeriodB_end,
    is18PeriodK_end,
    is22_75bunches,
    is23_75bunches,
    is23_400bunches,
    is23_first_2400bunches,
    l1topo_disabled,
    Count
  };
  
  class TriggerDecoratorAlg final : public AthReentrantAlgorithm
  {
    
  public:
    TriggerDecoratorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) const override;

  private:

    std::unordered_map<HHBBTT::TriggerChannel, std::string> m_triggerChannels =
      {
	{HHBBTT::SLT, "SLT"},
	{HHBBTT::LTT, "LTT"},
	{HHBBTT::ETT, "ETT"},
	{HHBBTT::ETT_4J12, "ETT_4J12"},
	{HHBBTT::MTT_2016, "MTT_2016"},
	{HHBBTT::MTT_high, "MTT_high"},
	{HHBBTT::MTT_low, "MTT_low"},
	{HHBBTT::STT, "STT"},
	{HHBBTT::DTT, "DTT"},
	{HHBBTT::DTT_2016, "DTT_2016"},
	{HHBBTT::DTT_4J12, "DTT_4J12"},
	{HHBBTT::DTT_L1Topo, "DTT_L1Topo"},
      };

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey
      { this, "event", "EventInfo", "EventInfo to read" };

    SG::ReadDecorHandleKey<xAOD::EventInfo> m_runNumberKey{
      this, "runNumberDecorKey", "EventInfo.runNumber", "Run number"};
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_rdmRunNumberKey{
      this, "RandomRunNumberDecorKey", "EventInfo.RandomRunNumber", "Random run number"};

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };
    Gaudi::Property<std::vector<int>> m_years
      { this, "Years", false, "which years are running" };

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

    std::unordered_map<HHBBTT::TriggerChannel,
      SG::WriteDecorHandleKey<xAOD::EventInfo> > m_pass_DecorKey;

    std::unordered_map<HHBBTT::TriggerChannel,
      SG::WriteDecorHandleKey<xAOD::MuonContainer> > m_mu_trigMatch_DecorKey;
    std::unordered_map<HHBBTT::TriggerChannel,
      SG::WriteDecorHandleKey<xAOD::ElectronContainer> > m_ele_trigMatch_DecorKey;
    std::unordered_map<HHBBTT::TriggerChannel,
      SG::WriteDecorHandleKey<xAOD::TauJetContainer> > m_tau_trigMatch_DecorKey;

    ToolHandle<Trig::IMatchingTool> m_matchingTool
      { this, "trigMatchingTool", "", "Trigger matching tool"};

    Gaudi::Property<bool> m_useDiTauTrigMatch
      { this, "diTauTrigMatch", true, "Run di-tau trigger matching" };
    
    void setRunNumberQuantities
      (unsigned int runNumber, int& year,
       std::unordered_map<HHBBTT::RunBooleans, bool>& runBoolMap) const;

    typedef std::unordered_map<std::string, SG::ReadDecorHandle<xAOD::EventInfo, bool> > trigReadDecoMap;
    typedef std::unordered_map<HHBBTT::TriggerChannel, SG::WriteDecorHandle<xAOD::EventInfo, bool> > passWriteDecoMap;
    typedef std::unordered_map<HHBBTT::TriggerChannel, SG::WriteDecorHandle<xAOD::MuonContainer, bool> > muTrigMatchWriteDecoMap;
    typedef std::unordered_map<HHBBTT::TriggerChannel, SG::WriteDecorHandle<xAOD::ElectronContainer, bool> > eleTrigMatchWriteDecoMap;
    typedef std::unordered_map<HHBBTT::TriggerChannel, SG::WriteDecorHandle<xAOD::TauJetContainer, bool> > tauTrigMatchWriteDecoMap;
    
    void checkSingleMuTriggers
      (int year, std::unordered_map<HHBBTT::RunBooleans, bool> runBoolMap,
       const xAOD::EventInfo* eventInfo, const trigReadDecoMap& triggerdecos,
       passWriteDecoMap& pass_decos,
       const xAOD::MuonContainer* muons,
       muTrigMatchWriteDecoMap& mu_trigMatchDecos) const;

    void checkSingleEleTriggers
      (int year, std::unordered_map<HHBBTT::RunBooleans, bool> runBoolMap,
       const xAOD::EventInfo* eventInfo, const trigReadDecoMap& triggerdecos,
       passWriteDecoMap& pass_decos,
       const xAOD::ElectronContainer* electrons,
       eleTrigMatchWriteDecoMap& ele_trigMatchDecos) const;

    void checkMuTauTriggers
      (int year, std::unordered_map<HHBBTT::RunBooleans, bool> runBoolMap,
       const xAOD::EventInfo* eventInfo, const trigReadDecoMap& triggerdecos,
       passWriteDecoMap& pass_decos,
       const xAOD::MuonContainer* muons,
       muTrigMatchWriteDecoMap& mu_trigMatchDecos,
       const xAOD::TauJetContainer* taus,
       tauTrigMatchWriteDecoMap& tau_trigMatchDecos) const;

    void checkEleTauTriggers
      (int year, std::unordered_map<HHBBTT::RunBooleans, bool> runBoolMap,
       const xAOD::EventInfo* eventInfo, const trigReadDecoMap& triggerdecos,
       passWriteDecoMap& pass_decos,
       const xAOD::ElectronContainer* electrons,
       eleTrigMatchWriteDecoMap& ele_trigMatchDecos,
       const xAOD::TauJetContainer* taus,
       tauTrigMatchWriteDecoMap& tau_trigMatchDecos) const;

    void checkSingleTauTriggers
      (int year, std::unordered_map<HHBBTT::RunBooleans, bool> runBoolMap,
       const xAOD::EventInfo* eventInfo, const trigReadDecoMap& triggerdecos,
       passWriteDecoMap& pass_decos,
       const xAOD::TauJetContainer* taus,
       tauTrigMatchWriteDecoMap& tau_trigMatchDecos) const;

    void checkDiTauTriggers
      (int year, std::unordered_map<HHBBTT::RunBooleans, bool> runBoolMap,
       const xAOD::EventInfo* eventInfo, const trigReadDecoMap& triggerdecos,
       passWriteDecoMap& pass_decos,
       const xAOD::TauJetContainer* taus,
       tauTrigMatchWriteDecoMap& tau_trigMatchDecos) const;

 
    
  };
}

#endif





