/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TriggerUtils.h"

namespace HHBBTT
{

  void getSingleMuTriggers(int year, const xAOD::EventInfo* eventInfo,
			   const runBoolReadDecoMap& runBoolDecos,
			   std::vector<std::string>& single_mu_paths,
			   std::string& single_mu_SF_path){
    if(year==2015){
      single_mu_paths = {"HLT_mu20_iloose_L1MU15", "HLT_mu50"};
      single_mu_SF_path = "mu20_iloose_L1MU15_OR_mu40";
    }
    else if(2016<=year && year<=2018){
      single_mu_paths = {"HLT_mu26_ivarmedium", "HLT_mu50"};
      single_mu_SF_path = "mu26_ivarmedium_OR_mu50";
    }
    else if(2022<=year && year<=2023 &&
	    !runBoolDecos.at(HHBBTT::is22_75bunches)(*eventInfo) &&
	    !runBoolDecos.at(HHBBTT::is23_75bunches)(*eventInfo) &&
	    !runBoolDecos.at(HHBBTT::is23_400bunches)(*eventInfo)){
      single_mu_paths = {
	"HLT_mu24_ivarmedium_L1MU14FCH", "HLT_mu50_L1MU14FCH",
	"HLT_mu60_0eta105_msonly_L1MU14FCH", "HLT_mu60_L1MU14FCH",
	"HLT_mu80_msonly_3layersEC_L1MU14FCH"
      };
      // No 2023 SF available yet
      if(year==2022) single_mu_SF_path = "mu24_ivarmedium_L1MU14FCH_OR_mu50_L1MU14FCH";
    }
  }

  void getSingleEleTriggers(int year, const xAOD::EventInfo* eventInfo,
			    const runBoolReadDecoMap& runBoolDecos,
			    std::vector<std::string>& single_ele_paths,
			    std::string& single_ele_SF_path){
    if(year==2015){
      single_ele_paths = {
	"HLT_e24_lhmedium_L1EM20VH", "HLT_e60_lhmedium",
	"HLT_e120_lhloose"
      };
      single_ele_SF_path = "e24_lhmedium_L1EM20VH_OR_e60_lhmedium_OR_e120_lhloose";
    }
    else if(2016<=year && year<=2018){
      single_ele_paths = {
	"HLT_e26_lhtight_nod0_ivarloose", "HLT_e60_lhmedium_nod0",
	"HLT_e140_lhloose_nod0"
      };
      single_ele_SF_path = "e26_lhtight_nod0_ivarloose_OR_e60_lhmedium_nod0_OR_e140_lhloose_nod0";
    }
    else if(runBoolDecos.at(HHBBTT::is22_75bunches)(*eventInfo)){
      single_ele_paths = {
	"HLT_e17_lhvloose_L1EM15VHI", "HLT_e20_lhvloose_L1EM15VH",
	"HLT_e250_etcut_L1EM22VHI"
      };
    }
    else if(year==2022){
      single_ele_paths = {
	"HLT_e26_lhtight_ivarloose_L1EM22VHI", "HLT_e60_lhmedium_L1EM22VHI",
	"HLT_e140_lhloose_L1EM22VHI", "HLT_e300_etcut_L1EM22VHI"
      };
    }
    else if(runBoolDecos.at(HHBBTT::is23_75bunches)(*eventInfo)){
      single_ele_paths = {
	"HLT_e26_lhtight_ivarloose_L1EM22VHI", "HLT_e60_lhmedium_L1EM22VHI",
	"HLT_e140_lhloose_L1EM22VHI", "HLT_e140_lhloose_noringer_L1EM22VHI",
	"HLT_e300_etcut_L1EM22VHI"
      };
    }
    else if(year==2023){
      single_ele_paths = {
	"HLT_e26_lhtight_ivarloose_L1eEM26M", "HLT_e60_lhmedium_L1eEM26M",
	"HLT_e140_lhloose_L1eEM26M", "HLT_e140_lhloose_noringer_L1eEM26M",
	"HLT_e300_etcut_L1eEM26M"
      };
    }
  }

  void getMuTauTriggers(int year, const xAOD::EventInfo* eventInfo,
			const runBoolReadDecoMap& runBoolDecos,
			std::vector<std::string>& mu_tau_paths_2016,
			std::vector<std::string>& mu_tau_paths_low,
			std::vector<std::string>& mu_tau_paths_high,
			std::pair<std::string, std::string>& mu_tau_SF_path){
    if(year==2015 || runBoolDecos.at(HHBBTT::is16PeriodA)(*eventInfo)){
      mu_tau_paths_2016 = {"HLT_mu14_tau25_medium1_tracktwo"};
      // No muon SF leg yet
      mu_tau_SF_path = std::make_pair("", "tau25_medium1_tracktwo");
    }
    else if(runBoolDecos.at(HHBBTT::is16PeriodB_D3)(*eventInfo) ||
	    runBoolDecos.at(HHBBTT::is16PeriodD4_end)(*eventInfo)){
      mu_tau_paths_2016 = {"HLT_mu14_ivarloose_tau25_medium1_tracktwo"};
      // No muon SF leg yet
      mu_tau_SF_path = std::make_pair("", "tau25_medium1_tracktwo");
    }
    else if(runBoolDecos.at(HHBBTT::is17PeriodB1_B4)(*eventInfo) ||
	    runBoolDecos.at(HHBBTT::is17PeriodB5_B7)(*eventInfo) ||
	    runBoolDecos.at(HHBBTT::is17PeriodB8_end)(*eventInfo)){
      mu_tau_paths_low = {"HLT_mu14_ivarloose_tau25_medium1_tracktwo_L1MU10_TAU12IM_3J12"};
      mu_tau_paths_high = {"HLT_mu14_ivarloose_tau35_medium1_tracktwo"};
      // No muon SF leg yet
      mu_tau_SF_path = std::make_pair("", "tau25_medium1_tracktwo");
    }
    else if(runBoolDecos.at(HHBBTT::is18PeriodB_end)(*eventInfo)){
      mu_tau_paths_low = {"HLT_mu14_ivarloose_tau25_medium1_tracktwoEF_L1MU10_TAU12IM_3J12"};
      mu_tau_paths_high = {"HLT_mu14_ivarloose_tau35_medium1_tracktwoEF"};
      if(runBoolDecos.at(HHBBTT::is18PeriodK_end)(*eventInfo)){
        mu_tau_paths_low.push_back("HLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA_L1MU10_TAU12IM_3J12");
        mu_tau_paths_high.push_back("HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA");
      }
      // No muon SF leg yet
      mu_tau_SF_path = std::make_pair("", "tau25_medium1_tracktwoEF_OR_mediumRNN_tracktwoMVA");
    }
    else if(year<=2022 || year<=2023){
      mu_tau_paths_low = {"HLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1MU8F_TAU12IM_3J12"};
      mu_tau_paths_high = {"HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA_03dRAB_L1MU8F_TAU20IM"};
    }
  }

  void getEleTauTriggers(int year, const xAOD::EventInfo* eventInfo,
			 const runBoolReadDecoMap& runBoolDecos,
			 std::vector<std::string>& ele_tau_paths,
			 std::vector<std::string>& ele_tau_paths_4J12,
			 std::pair<std::string, std::string>& ele_tau_SF_path){
    if(year==2015 || runBoolDecos.at(HHBBTT::is16PeriodA)(*eventInfo)){
      ele_tau_paths = {"HLT_e17_lhmedium_nod0_tau25_medium1_tracktwo"};
      ele_tau_SF_path = std::make_pair("e17_lhmedium_nod0_L1EM15HI",
				       "tau25_medium1_tracktwo");
    }
    else if(runBoolDecos.at(HHBBTT::is16PeriodB_D3)(*eventInfo) ||
	    runBoolDecos.at(HHBBTT::is16PeriodD4_end)(*eventInfo)){
      ele_tau_paths = {"HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwo"};
      ele_tau_SF_path = std::make_pair("e17_lhmedium_nod0_AND_e17_lhvloose_nod0_L1EM15VHI",
				       "tau25_medium1_tracktwo");
    }
    else if(runBoolDecos.at(HHBBTT::is17PeriodB1_B4)(*eventInfo) ||
	    runBoolDecos.at(HHBBTT::is17PeriodB5_B7)(*eventInfo) ||
	    runBoolDecos.at(HHBBTT::is17PeriodB8_end)(*eventInfo)){
      ele_tau_paths = {"HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwo"};
      ele_tau_paths_4J12 = {"HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwo_L1EM15VHI_2TAU12IM_4J12"};
      ele_tau_SF_path = std::make_pair("e17_lhmedium_nod0_AND_e17_lhvloose_nod0_L1EM15VHI",
				       "tau25_medium1_tracktwo");
    }
    else if(runBoolDecos.at(HHBBTT::is18PeriodB_end)(*eventInfo)){
      ele_tau_paths = {"HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwoEF"};
      ele_tau_paths_4J12 = {"HLT_e17_lhmedium_nod0_ivarloose_tau25_medium1_tracktwoEF_L1EM15VHI_2TAU12IM_4J12"};
      if(runBoolDecos.at(HHBBTT::is18PeriodK_end)(*eventInfo)){
        ele_tau_paths.push_back("HLT_e17_lhmedium_nod0_ivarloose_tau25_mediumRNN_tracktwoMVA");
        ele_tau_paths_4J12.push_back("HLT_e17_lhmedium_nod0_ivarloose_tau25_mediumRNN_tracktwoMVA_L1EM15VHI_2TAU12IM_4J12");
      }
      ele_tau_SF_path = std::make_pair("e17_lhmedium_nod0_AND_e17_lhvloose_nod0_L1EM15VHI",
				       "tau25_medium1_tracktwoEF_OR_mediumRNN_tracktwoMVA");
    }
    else if(year==2022){
      ele_tau_paths = {"HLT_e24_lhmedium_ivarloose_tau20_mediumRNN_tracktwoMVA_03dRAB_L1EM22VHI"};
      ele_tau_paths_4J12 = {"HLT_e17_lhmedium_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1EM15VHI_2TAU12IM_4J12"};
    }
    else if(year==2023){
      ele_tau_paths = {"HLT_e24_lhmedium_ivarloose_tau20_mediumRNN_tracktwoMVA_03dRAB_L1eEM26M"};
      ele_tau_paths_4J12 = {"HLT_e17_lhmedium_ivarloose_tau25_mediumRNN_tracktwoMVA_03dRAB_L1EM15VHI_2TAU12IM_4J12"};
    }
  }

  void getSingleTauTriggers(int year, const xAOD::EventInfo* eventInfo,
			    const runBoolReadDecoMap& runBoolDecos,
			    std::vector<std::string>& single_tau_paths,
			    std::string& single_tau_SF_path){
    if(year==2015 || runBoolDecos.at(HHBBTT::is16PeriodA)(*eventInfo)){
      single_tau_paths = {"HLT_tau80_medium1_tracktwo_L1TAU60"};
      single_tau_SF_path = "tau80L1TAU60_medium1_tracktwo";
    }
    else if(runBoolDecos.at(HHBBTT::is16PeriodB_D3)(*eventInfo)){
      single_tau_paths = {"HLT_tau125_medium1_tracktwo"};
      single_tau_SF_path = "tau125_medium1_tracktwo";
    }
    else if(runBoolDecos.at(HHBBTT::is16PeriodD4_end)(*eventInfo) ||
	    runBoolDecos.at(HHBBTT::is17PeriodB1_B4)(*eventInfo)){
      single_tau_paths = {"HLT_tau160_medium1_tracktwo"};
      single_tau_SF_path = "tau160_medium1_tracktwo";
    }
    else if(runBoolDecos.at(HHBBTT::is17PeriodB5_B7)(*eventInfo) ||
	    runBoolDecos.at(HHBBTT::is17PeriodB8_end)(*eventInfo)){
      single_tau_paths = {"HLT_tau160_medium1_tracktwo_L1TAU100"};
      single_tau_SF_path = "tau160L1TAU100_medium1_tracktwo";
    }
    else if(year==2018){
      single_tau_paths = {"HLT_tau160_medium1_tracktwoEF_L1TAU100"};
      single_tau_SF_path = "tau160L1TAU100_medium1_tracktwoEF";
      if(runBoolDecos.at(HHBBTT::is18PeriodK_end)(*eventInfo)){
        single_tau_paths.push_back("HLT_tau160_mediumRNN_tracktwoMVA_L1TAU100");
        single_tau_SF_path = "tau160L1TAU100_medium1_tracktwoEF_OR_mediumRNN_tracktwoMVA";
      }
    }
    else if(year==2022){
      single_tau_paths = {"HLT_tau160_mediumRNN_tracktwoMVA_L1TAU100"};
    }
    else if(year==2023){
      single_tau_paths = {"HLT_tau160_mediumRNN_tracktwoMVA_L1TAU100"};
      if(runBoolDecos.at(HHBBTT::is23_first_2400bunches)(*eventInfo)){
        single_tau_paths = {"HLT_tau160_mediumRNN_tracktwoMVA_L1eTAU140"};
      }
    }
  }

  void getDiTauTriggers(int year, const xAOD::EventInfo* eventInfo,
			const runBoolReadDecoMap& runBoolDecos,
			std::vector<std::string>& ditau_paths_2016,
			std::vector<std::string>& ditau_paths_L1Topo,
			std::vector<std::string>& ditau_paths_4J12,
			std::vector<std::string>& ditau_paths_L1Topo_delayed,
			std::vector<std::string>& ditau_paths_4J12_delayed,
			std::vector<std::string>& tau35_match_paths,
			std::vector<std::string>& tau25_match_paths,
			std::pair<std::string, std::string>& di_tau_SF_path){
    if(year==2015){
      ditau_paths_2016 = {"HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1TAU20IM_2TAU12IM"};
      tau35_match_paths = {"HLT_tau35_medium1_tracktwo_L1TAU20"};
      tau25_match_paths = {"HLT_tau25_medium1_tracktwo_L1TAU12"};
      di_tau_SF_path = std::make_pair("tau35_medium1_tracktwo",
				      "tau25_medium1_tracktwo");
    }

    else if(year==2016){
      ditau_paths_2016 = {"HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo"};
      tau35_match_paths = {"HLT_tau35_medium1_tracktwo"};
      tau25_match_paths = {"HLT_tau25_medium1_tracktwo"};
      di_tau_SF_path = std::make_pair("tau35_medium1_tracktwo",
				      "tau25_medium1_tracktwo");
    }

    else if(year==2017){
      ditau_paths_4J12 = {"HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1TAU20IM_2TAU12IM_4J12"};
      tau35_match_paths = {"HLT_tau35_medium1_tracktwo"};
      tau25_match_paths = {"HLT_tau25_medium1_tracktwo_L1TAU12"};

      if(runBoolDecos.at(HHBBTT::l1topo_disabled)(*eventInfo)){
        ditau_paths_2016 = {"HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo"};
        tau25_match_paths.push_back("HLT_tau25_medium1_tracktwo");
      }

      if(runBoolDecos.at(HHBBTT::is17PeriodB1_B4)(*eventInfo)){
        // For Period B1 to B4 in 2017, should use this trigger but go to L1Topo selection
        ditau_paths_L1Topo = {"HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo"};
        tau25_match_paths.push_back("HLT_tau25_medium1_tracktwo");
      }
      else if(!runBoolDecos.at(HHBBTT::l1topo_disabled)(*eventInfo) &&
	      (runBoolDecos.at(HHBBTT::is17PeriodB5_B7)(*eventInfo) ||
	       runBoolDecos.at(HHBBTT::is17PeriodB8_end)(*eventInfo))){
        ditau_paths_L1Topo = {"HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1DR_TAU20ITAU12I_J25"};
      }

      di_tau_SF_path = std::make_pair("tau35_medium1_tracktwo",
				      "tau25_medium1_tracktwo");
    }
    
    else if(year==2018){
      ditau_paths_L1Topo = {"HLT_tau35_medium1_tracktwoEF_tau25_medium1_tracktwoEF_L1DR_TAU20ITAU12I_J25"};
      ditau_paths_4J12 = {"HLT_tau35_medium1_tracktwoEF_tau25_medium1_tracktwoEF_L1TAU20IM_2TAU12IM_4J12p0ETA23"};
      tau35_match_paths = {"HLT_tau35_medium1_tracktwoEF"};
      tau25_match_paths = {"HLT_tau25_medium1_tracktwoEF"};

      if(runBoolDecos.at(HHBBTT::is18PeriodK_end)(*eventInfo)){
        ditau_paths_L1Topo.push_back("HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_L1DR_TAU20ITAU12I_J25");
        ditau_paths_4J12.push_back("HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_L1TAU20IM_2TAU12IM_4J12p0ETA23");
        tau35_match_paths.push_back("HLT_tau35_mediumRNN_tracktwoMVA");
        tau25_match_paths.push_back("HLT_tau25_mediumRNN_tracktwoMVA");
      }

      di_tau_SF_path = std::make_pair("tau35_medium1_tracktwoEF_OR_mediumRNN_tracktwoMVA",
				      "tau25_medium1_tracktwoEF_OR_mediumRNN_tracktwoMVA");
    }

    else if(year>=2022){
      ditau_paths_L1Topo = {"HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1DR_TAU20ITAU12I_J25"};
      ditau_paths_4J12 = {"HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25"};
      // Single leg paths not available for matching for Run 3
      // LTT T&P triggers to be used instead
      tau35_match_paths = {
	"HLT_mu24_ivarmedium_tau35_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU14FCH",
	"HLT_e26_lhtight_ivarloose_tau35_mediumRNN_tracktwoMVA_probe_03dRAB_L1EM22VHI"
      };
      tau25_match_paths = {
	"HLT_mu24_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU14FCH",
	"HLT_e26_lhtight_ivarloose_tau25_mediumRNN_tracktwoMVA_probe_03dRAB_L1EM22VHI"
      };
    }

    else if(year>=2023){
      ditau_paths_L1Topo = {"HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1DR_TAU20ITAU12I_J25"};
      ditau_paths_4J12 = {"HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25"};
      tau35_match_paths = {
	"HLT_mu24_ivarmedium_tau35_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU14FCH",
	"HLT_e26_lhtight_ivarloose_tau35_mediumRNN_tracktwoMVA_probe_03dRAB_L1EM22VHI"
      };
      tau25_match_paths = {
	"HLT_mu24_ivarmedium_tau25_mediumRNN_tracktwoMVA_probe_03dRAB_L1MU14FCH",
	"HLT_e26_lhtight_ivarloose_tau25_mediumRNN_tracktwoMVA_probe_L1TAU12IM_03dRAB_L1eEM26M"
      };

      if (runBoolDecos.at(HHBBTT::is23_first_2400bunches)(*eventInfo)){
        ditau_paths_L1Topo_delayed = {"HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB30_L1DR_TAU20ITAU12I_J25"};
        ditau_paths_4J12_delayed = {"HLT_tau30_mediumRNN_tracktwoMVA_tau20_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25"};
      }
    }
  }

  void getDiBJetTriggers(int year, const xAOD::EventInfo* eventInfo,
			 const runBoolReadDecoMap& runBoolDecos,
			 std::vector<std::string>& dib_paths){
    if(year==2022){
      dib_paths = {"HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25"};
    }
    else if(year==2023){
      dib_paths = {"HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25"};
      if (runBoolDecos.at(HHBBTT::is23_from1200bunches)(*eventInfo)){
        dib_paths = {"HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25"};
      }
    }
  }
}
