/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration   
*/

#include "TriggerDecoratorAlg.h"


namespace ttHH
{
  TriggerDecoratorAlg::TriggerDecoratorAlg(const std::string &name,
					   ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator) 
  {

  }

  StatusCode TriggerDecoratorAlg::initialize()
  {
    ATH_CHECK(m_eventInfoKey.initialize());

    m_yearKey = "EventInfo.dataTakingYear";
    ATH_CHECK(m_yearKey.initialize());

    // make trigger decorators
    for (const auto& trig : m_triggers){

      // CP alg should convert trigger names
      std::string modifiedTrigName = trig;
      std::replace(modifiedTrigName.begin(), modifiedTrigName.end(), '-', '_');
      std::replace(modifiedTrigName.begin(), modifiedTrigName.end(), '.', 'p');

      std::string triggerDecorName = "trigPassed_"+modifiedTrigName;
      SG::ReadDecorHandleKey< xAOD::EventInfo > triggerDecorKey = "EventInfo." + triggerDecorName;

      m_triggerdecoKeys.emplace(trig, triggerDecorKey);
      ATH_CHECK(m_triggerdecoKeys.at(trig).initialize());
    }

    for (const auto& [channel, name] : m_triggerChannels){
      SG::WriteDecorHandleKey<xAOD::EventInfo> flag;
      flag = "EventInfo.ttHH_pass_trigger_"+name;
      m_pass_DecorKey.emplace(channel, flag);
      ATH_CHECK(m_pass_DecorKey.at(channel).initialize());
    }

    return StatusCode::SUCCESS;
  }

  StatusCode TriggerDecoratorAlg::execute(const EventContext& ctx) const
  {
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey,ctx);
    ATH_CHECK (eventInfo.isValid());

    trigReadDecoMap triggerdecos;
    for (const auto& [name, key] : m_triggerdecoKeys){
      triggerdecos.emplace(name, key);
    }

    passWriteDecoMap pass_decos;
    for (const auto& [channel, key] : m_pass_DecorKey){
      pass_decos.emplace(channel, key);
      pass_decos.at(channel)(*eventInfo) = false;
    }

    SG::ReadDecorHandle<xAOD::EventInfo, unsigned int> year(m_yearKey);

    std::vector<std::string> singLepTrigPaths = m_triggerMap.at(year(*eventInfo)).at(ttHH::SINGLEP);
    std::vector<std::string> diLepTrigPaths = m_triggerMap.at(year(*eventInfo)).at(ttHH::DILEP);
    std::vector<std::string> bjetTrigPaths = m_triggerMap.at(year(*eventInfo)).at(ttHH::BJET);

    if (!singLepTrigPaths.empty()) evaluateTriggerCuts(eventInfo.cptr(), singLepTrigPaths, triggerdecos, pass_decos, ttHH::SINGLEP);
    if (!diLepTrigPaths.empty()) evaluateTriggerCuts(eventInfo.cptr(), diLepTrigPaths, triggerdecos, pass_decos, ttHH::DILEP);
    if (!bjetTrigPaths.empty()) evaluateTriggerCuts(eventInfo.cptr(), bjetTrigPaths, triggerdecos, pass_decos, ttHH::BJET);

    return StatusCode::SUCCESS;
  }

  void TriggerDecoratorAlg::evaluateTriggerCuts(const xAOD::EventInfo* eventInfo, const std::vector<std::string> &Triggers,
                                                  const trigReadDecoMap& triggerdecos, passWriteDecoMap& pass_decos, 
                                                  ttHH::TriggerChannel flag) const {

    for (const std::string &trigger : Triggers)
    {
      bool pass = triggerdecos.at(trigger)(*eventInfo);
      if (pass) {
        pass_decos.at(flag)(*eventInfo) = true;
        break;
      }
    }
  }

  const std::unordered_map<int, std::unordered_map<ttHH::TriggerChannel, std::vector<std::string>>> TriggerDecoratorAlg::m_triggerMap = {
        {2015, {
            {ttHH::BJET, {
		"HLT_2j35_btight_2j35_L13J25.0ETA23"
	    }},
            {ttHH::SINGLEP, {
                "HLT_e24_lhmedium_L1EM20VH",
                "HLT_e60_lhmedium",
                "HLT_e120_lhloose",
                "HLT_mu20_iloose_L1MU15",
                "HLT_mu50"
            }},
            {ttHH::DILEP, {
                "HLT_2e12_lhvloose_L12EM10VH",
                "HLT_e17_lhloose_mu14",
                "HLT_e7_lhmedium_mu24",
                "HLT_2mu10",
                "HLT_mu18_mu8noL1"
            }}
        }},
        {2016, {
            {ttHH::BJET, {
                "HLT_2j35_bmv2c2060_split_2j35_L14J15.0ETA25",
                "HLT_j100_2j55_bmv2c2060_split"
            }},
            {ttHH::SINGLEP, {
                "HLT_e26_lhtight_nod0_ivarloose",
                "HLT_e60_lhmedium_nod0",
                "HLT_e140_lhloose_nod0",
                "HLT_mu26_ivarmedium",
                "HLT_mu50"
            }},
            {ttHH::DILEP, {
                "HLT_e17_lhloose_nod0_mu14",
                "HLT_e7_lhmedium_nod0_mu24",
                "HLT_2e17_lhvloose_nod0",
                "HLT_2mu14",
                "HLT_mu22_mu8noL1"
            }}
        }},
        {2017, {
            {ttHH::BJET, {
                "HLT_j110_gsc150_boffperf_split_2j35_gsc55_bmv2c1070_split_L1J85_3J30",
                "HLT_2j15_gsc35_bmv2c1040_split_2j15_gsc35_boffperf_split_L14J15.0ETA25"
            }},
            {ttHH::SINGLEP, {
                "HLT_e26_lhtight_nod0_ivarloose",
                "HLT_e60_lhmedium_nod0",
                "HLT_e140_lhloose_nod0",
                "HLT_mu26_ivarmedium",
                "HLT_mu50"
            }},
            {ttHH::DILEP, {
                "HLT_2e24_lhvloose_nod0",
                "HLT_e17_lhloose_nod0_mu14",
                "HLT_e7_lhmedium_nod0_mu24",
                "HLT_2mu14",
                "HLT_mu22_mu8noL1"
            }}
        }},
        {2018, {
            {ttHH::BJET, {
                "HLT_j110_gsc150_boffperf_split_2j45_gsc55_bmv2c1070_split_L1J85_3J30",
                "HLT_2j35_bmv2c1060_split_2j35_L14J15.0ETA25"
            }},
            {ttHH::SINGLEP, {
                "HLT_e26_lhtight_nod0_ivarloose",
                "HLT_e60_lhmedium_nod0",
                "HLT_e140_lhloose_nod0",
                "HLT_mu26_ivarmedium",
                "HLT_mu50"
            }},
            {ttHH::DILEP, {
                "HLT_2e24_lhvloose_nod0",
                "HLT_2e24_lhvloose_nod0_L12EM15VHI",
                "HLT_e17_lhloose_nod0_mu14",
                "HLT_e7_lhmedium_nod0_mu24",
                "HLT_2mu14",
                "HLT_mu22_mu8noL1"
            }}
        }},
        {2022, {
            {ttHH::BJET, {
                "HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bdl1d82_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25",
                "HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25",
                "HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b85_L1MU8F_2J15_J20",
                "HLT_2j35c_020jvt_bdl1d60_2j35c_020jvt_pf_ftf_presel2j25XX2j25b85_L14J15p0ETA25",
                "HLT_j150_2j55_0eta290_020jvt_bdl1d70_pf_ftf_preselj80XX2j45b90_L1J85_3J30"
            }},
            {ttHH::SINGLEP, {
                "HLT_e26_lhtight_ivarloose_L1EM22VHI",
                "HLT_e60_lhmedium_L1EM22VHI",
                "HLT_e140_lhloose_L1EM22VHI",
                "HLT_e300_etcut_L1EM22VHI",
                "HLT_mu24_ivarmedium_L1MU14FCH",
                "HLT_mu50_L1MU14FCH",
            }},
            {ttHH::DILEP, {
                "HLT_2e17_lhvloose_L12EM15VHI",
                "HLT_mu22_mu8noL1_L1MU14FCH",
                "HLT_e17_lhloose_mu14_L1EM15VH_MU8F",
                "HLT_2e17_lhvloose_L12EM15VHI",
                "HLT_2e24_lhvloose_L12EM20VH",
                "HLT_mu22_mu8noL1_L1MU14FCH",
                "HLT_e7_lhmedium_mu24_L1MU14FCH",
                "HLT_e17_lhloose_mu14_L1EM15VH_MU8F",
                "HLT_e26_lhmedium_mu8noL1_L1EM22VHI",
                "HLT_e24_lhvloose_2e12_lhvloose_L1EM20VH_3EM10VH",
                "HLT_2mu10_l2mt_L1MU10BOM",
                "HLT_2mu14_L12MU8F",
                "HLT_mu20_ivarmedium_mu8noL1_L1MU14FCH",
            }}
        }},
        {2023, {
            {ttHH::BJET, {
                "HLT_j150_2j55_0eta290_020jvt_bgn170_pf_ftf_preselj80XX2j45b90_L1J85_3J30",
                "HLT_j140_2j50_0eta290_020jvt_bgn170_pf_ftf_preselj80XX2j45b90_L1J85_3J30",
                "HLT_2j35c_020jvt_bgn160_2j35c_020jvt_pf_ftf_presel2j25XX2j25b85_L14J15p0ETA25",
                "HLT_2j45_0eta290_020jvt_bgn160_2j45_pf_ftf_presel2j25XX2j25b85_L14J15p0ETA25",
                "HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn182_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25",
                "HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25",
                "HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1MU8F_2J15_J20"
            }},
            {ttHH::SINGLEP, {
                "HLT_e26_lhtight_ivarloose_L1eEM26M",
                "HLT_e60_lhmedium_L1eEM26M",
                "HLT_e140_lhloose_L1eEM26M",
                "HLT_mu24_ivarmedium_L1MU14FCH",
                "HLT_mu50_L1MU14FCH"
            }},
            {ttHH::DILEP, {
                "HLT_2e17_lhvloose_L12eEM18M",
                "HLT_2e17_lhvloose_L12EM15VHI",
                "HLT_2e24_lhvloose_L12EM20VH",
                "HLT_2e24_lhvloose_L12eEM24L",
                "HLT_mu22_mu8noL1_L1MU14FCH",
                "HLT_e7_lhmedium_mu24_L1MU14FCH",
                "HLT_e17_lhloose_mu14_L1EM15VH_MU8F",
                "HLT_e26_lhmedium_mu8noL1_L1EM22VHI",
                "HLT_2mu10_l2mt_L1MU10BOM",
                "HLT_2mu14_L12MU8F",
                "HLT_mu20_ivarmedium_mu8noL1_L1MU14FCH",               
            }}
        }}
    };
}
