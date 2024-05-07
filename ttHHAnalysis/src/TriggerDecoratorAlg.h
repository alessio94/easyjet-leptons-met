/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

*/


// Always protect against multiple includes!
#ifndef TTHHANALYSIS_TRIGGERDECORATORALG
#define TTHHANALYSIS_TRIGGERDECORATORALG

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>


namespace ttHH
{

  enum TriggerChannel
  {
    BJET,
    SINGLEP,
    DILEP,
  };

  class TriggerDecoratorAlg final : public AthReentrantAlgorithm
  {
    
  public:
    TriggerDecoratorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) const override;

  private:

    std::unordered_map<ttHH::TriggerChannel, std::string> m_triggerChannels =
      {
        {ttHH::SINGLEP, "singlep"},
        {ttHH::DILEP, "dilep"},
      };

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey
      { this, "event", "EventInfo", "EventInfo to read" };

    SG::ReadDecorHandleKey<xAOD::EventInfo> m_yearKey;

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    Gaudi::Property<std::vector<std::string>> m_triggers
      { this, "triggerLists", {}, "Name list of trigger" };
    std::unordered_map<std::string, SG::ReadDecorHandleKey<xAOD::EventInfo> >
      m_triggerdecoKeys;
    std::unordered_map<ttHH::TriggerChannel,
      SG::WriteDecorHandleKey<xAOD::EventInfo> > m_pass_DecorKey;

    typedef std::unordered_map<std::string, SG::ReadDecorHandle<xAOD::EventInfo, bool> > trigReadDecoMap;
    typedef std::unordered_map<ttHH::TriggerChannel, SG::WriteDecorHandle<xAOD::EventInfo, bool> > passWriteDecoMap;

    void evaluateTriggerCuts(const xAOD::EventInfo* eventInfo, const std::vector<std::string> &Triggers,
                                                  const trigReadDecoMap& triggerdecos, passWriteDecoMap& pass_decos, 
                                                  ttHH::TriggerChannel flag) const;

    static const std::unordered_map<int, std::unordered_map<ttHH::TriggerChannel, std::vector<std::string>>> m_triggerMap;

  };
}

#endif

