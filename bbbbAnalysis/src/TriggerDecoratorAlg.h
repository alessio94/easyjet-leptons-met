/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

  TriggerDecoratorAlg:
  An alg that saves the event-level + tau-trigger matching info as decorations
  Required for antiTau definition in TauDecoratorAlg
*/


// Always protect against multiple includes!
#ifndef HH4BANALYSIS_TRIGGERDECORATORALG
#define HH4BANALYSIS_TRIGGERDECORATORALG

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>

namespace HH4B
{
  enum TriggerChannel
  {
    SINGB,
    DIB_SINGJ,
    DIB_HT,
    DIB_DIJ,
    SB,
    J75,
    J80,
  };
  
  class TriggerDecoratorAlg final : public AthReentrantAlgorithm
  {
    
  public:
    TriggerDecoratorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) const override;

  private:

    const std::unordered_map<HH4B::TriggerChannel, std::string> m_triggerChannels =
      {
        {HH4B::SINGB, "1b"},
        {HH4B::DIB_SINGJ, "2b1j"},
        {HH4B::DIB_HT, "2bHT"},
        {HH4B::DIB_DIJ, "2b2j"},
        {HH4B::J75, "j75"},
        {HH4B::J80, "j80"}
      };

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey
      { this, "event", "EventInfo", "EventInfo to read" };

    SG::ReadDecorHandleKey<xAOD::EventInfo> m_yearKey;

    SG::ReadHandleKey<xAOD::JetContainer> m_jetsKey
      { this, "jets", "", "Jet container" };

    Gaudi::Property<std::vector<std::string>> m_triggers
      { this, "triggerLists", {}, "Name list of trigger" };
    std::unordered_map<std::string, SG::ReadDecorHandleKey<xAOD::EventInfo> >
      m_triggerdecoKeys;

    std::unordered_map<HH4B::TriggerChannel, SG::WriteDecorHandleKey<xAOD::EventInfo> > 
      m_pass_DecorKey;

    SG::WriteDecorHandleKey<xAOD::EventInfo> m_bucketDecoratorKey{"EventInfo.bucket"};

    typedef std::unordered_map<std::string, SG::ReadDecorHandle<xAOD::EventInfo, bool> > trigReadDecoMap;
    typedef std::unordered_map<HH4B::TriggerChannel, SG::WriteDecorHandle<xAOD::EventInfo, bool> > passWriteDecoMap;

    void evaluateTriggerCuts(const xAOD::EventInfo* eventInfo, const std::vector<std::string> &Triggers,
                                                  const trigReadDecoMap& triggerdecos, passWriteDecoMap& pass_decos, 
                                                  HH4B::TriggerChannel flag) const;

    void evaluateTriggerBuckets(const xAOD::EventInfo* eventInfo, 
                                const SG::ReadDecorHandle<xAOD::EventInfo, unsigned int>& year, 
                                SG::ReadHandle<xAOD::JetContainer> jets, 
                                passWriteDecoMap& pass_decos, 
                                SG::WriteDecorHandle<xAOD::EventInfo, int>& m_bucketDecorator) const;
    
    const std::unordered_map<int, std::unordered_map<HH4B::TriggerChannel, std::vector<std::string>>> m_triggerMap = {
        {2016, {
            {HH4B::SINGB, {
                "HLT_j225_bmv2c2060_split"
            }},
            {HH4B::DIB_SINGJ, {
                "HLT_j100_2j55_bmv2c2060_split"
            }},
            {HH4B::DIB_HT, {
                "HLT_2j35_bmv2c2060_split_2j35_L14J15.0ETA25"
            }},
            {HH4B::DIB_DIJ, {
                "HLT_2j35_bmv2c2060_split_2j35_L14J15.0ETA25"
            }}
        }},
        {2017, {
            {HH4B::SINGB, {
                "HLT_j225_gsc300_bmv2c1070_split"
            }},
            {HH4B::DIB_SINGJ, {
                "HLT_j110_gsc150_boffperf_split_2j35_gsc55_bmv2c1070_split_L1J85_3J30"
            }},
            {HH4B::DIB_HT, {
                "HLT_2j35_gsc55_bmv2c1050_split_ht300_L1HT190-J15s5.ETA21"
            }},
            {HH4B::DIB_DIJ, {
                "HLT_2j15_gsc35_bmv2c1040_split_2j15_gsc35_boffperf_split_L14J15.0ETA25"
            }}
        }},
        {2018, {
            {HH4B::SINGB, {
                "HLT_j225_gsc300_bmv2c1070_split"
            }},
            {HH4B::DIB_SINGJ, {
                "HLT_j110_gsc150_boffperf_split_2j45_gsc55_bmv2c1070_split_L1J85_3J30"
            }},
            {HH4B::DIB_HT, {
                "HLT_2j45_gsc55_bmv2c1050_split_ht300_L1HT190-J15s5.ETA21"
            }},
            {HH4B::DIB_DIJ, {
                "HLT_2j35_bmv2c1060_split_2j35_L14J15.0ETA25"
            }}
        }},
        {2022, {
            {HH4B::J80, {
                "HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25"
            }}
        }},
        {2023, {
            {HH4B::J75, {
                "HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25"
            }},
            {HH4B::J80, {
                "HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25"
            }}
        }}
    };

  };
}

#endif