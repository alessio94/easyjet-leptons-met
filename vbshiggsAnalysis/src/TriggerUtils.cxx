/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TriggerUtils.h"

namespace VBSHIGGS
{
  void getSingleEleTriggers(int year, const xAOD::EventInfo* event,
			                const runBoolReadDecoMap& runBoolDecos,
			                std::vector<std::string>& single_ele_paths){
    if(year==2015){
      single_ele_paths = {
        "HLT_e24_lhmedium_L1EM20VH", "HLT_e60_lhmedium",
        "HLT_e120_lhloose"
      };
    }
    else if(2016<=year && year<=2018){
      single_ele_paths = {
        "HLT_e26_lhtight_nod0_ivarloose", "HLT_e60_lhmedium_nod0",
        "HLT_e140_lhloose_nod0"
      };
    }
    else if(runBoolDecos.at(VBSHIGGS::is22_75bunches)(*event)){
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
    else if(runBoolDecos.at(VBSHIGGS::is23_75bunches)(*event)){
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

  void getSingleMuTriggers(int year, const xAOD::EventInfo* event,
			               const runBoolReadDecoMap& runBoolDecos,
			               std::vector<std::string>& single_mu_paths){
    if(year==2015){
      single_mu_paths = {"HLT_mu20_iloose_L1MU15", "HLT_mu50"};
    }
    else if(2016<=year && year<=2018){
      single_mu_paths = {"HLT_mu26_ivarmedium", "HLT_mu50"};
    }
    else if(2022<=year && year<=2023 &&
	        !runBoolDecos.at(VBSHIGGS::is22_75bunches)(*event) &&
	        !runBoolDecos.at(VBSHIGGS::is23_75bunches)(*event) &&
	        !runBoolDecos.at(VBSHIGGS::is23_400bunches)(*event)){
      single_mu_paths = {
        "HLT_mu24_ivarmedium_L1MU14FCH", "HLT_mu50_L1MU14FCH",
        "HLT_mu60_0eta105_msonly_L1MU14FCH", "HLT_mu60_L1MU14FCH",
        "HLT_mu80_msonly_3layersEC_L1MU14FCH"
      };
    }
  }

  void getDiEleTriggers(int year, const xAOD::EventInfo* event,
			               const runBoolReadDecoMap& runBoolDecos,
			               std::vector<std::string>& di_ele_paths){
    if(year==2015){
      di_ele_paths = {"HLT_2e12_lhloose_L12EM10VH"};
    }
    else if(year==2016){
      di_ele_paths = {"HLT_2e17_lhvloose_nod0"};
    }
    else if(runBoolDecos.at(VBSHIGGS::is17_periodB5_B8)(*event)){
      di_ele_paths = {
        "HLT_2e24_lhvloose_nod0"
      };
    }
    else if(2017<=year && year<=2018){
      di_ele_paths = {
        "HLT_2e17_lhvloose_nod0_L12EM15VHI", "HLT_2e24_lhvloose_nod0"
      };
    }
    else if(year==2022){
      di_ele_paths = {
        "HLT_e24_lhvloose_2e12_lhvloose_L1EM20VH_3EM10VH", "HLT_2e17_lhvloose_L12EM15VHI",
        "HLT_2e24_lhvloose_L12EM20VH"
      };
    }
    else if(year==2023){
      di_ele_paths = {
        "HLT_e24_lhvloose_2e12_lhvloose_L1eEM24L_3eEM12L", "HLT_2e17_lhvloose_L12eEM18M",
        "HLT_2e24_lhvloose_L12eEM24L"
      };
    }
  }

  void getDiMuTriggers(int year, const xAOD::EventInfo* event,
			               const runBoolReadDecoMap& runBoolDecos,
			               std::vector<std::string>& di_mu_paths){
    if(year==2015){
      di_mu_paths = {"HLT_mu18_mu8noL1"};
    }
    else if(2016<=year && year<=2018){
      di_mu_paths = {"HLT_mu22_mu8noL1"};
    }
    else if(2022<=year && year<=2023){
      di_mu_paths = {"HLT_mu22_mu8noL1_L1MU14FCH", "HLT_2mu14_L12MU8F"};
    }
  }

  void getAsymLep2Triggers(int year, const xAOD::EventInfo* event,
			               const runBoolReadDecoMap& runBoolDecos,
			               std::vector<std::string>& asym_lepton_paths){
    if(year==2015){
      asym_lepton_paths = {"HLT_e17_lhloose_mu14"};
    }
    else if(2016<=year && year<=2018){
      asym_lepton_paths = {"HLT_e17_lhloose_nod0_mu14"};
    }
    else if(2022<=year && year<=2023){
      asym_lepton_paths = {"HLT_e17_lhloose_mu14_L1EM15VH_MU8F"};
    }
  }

  void getAsymLep1emTriggers(int year, const xAOD::EventInfo* event,
			               const runBoolReadDecoMap& runBoolDecos,
			               std::vector<std::string>& asym_lepton_paths){
    if(year==2016){
      asym_lepton_paths = {"HLT_e26_lhmedium_nod0_L1EM22VHI_mu8noL1"};
    }
    else if(2017<=year && year<=2018){
      asym_lepton_paths = {"HLT_e26_lhmedium_nod0_mu8noL1"};
    }
    else if(2022<=year && year<=2023){
      asym_lepton_paths = {"HLT_e26_lhmedium_mu8noL1_L1EM22VHI"};
    }
  }

  void getAsymLep1meTriggers(int year, const xAOD::EventInfo* event,
			               const runBoolReadDecoMap& runBoolDecos,
			               std::vector<std::string>& asym_lepton_paths){
    if(year==2015){
      asym_lepton_paths = {"HLT_e7_lhmedium_mu24"};
    }
    else if(2016<=year && year<=2018){
      asym_lepton_paths = {"HLT_e7_lhmedium_nod0_mu24"};
    }
    else if(2022<=year && year<=2023){
      asym_lepton_paths = {"HLT_e7_lhmedium_mu24_L1MU14FCH"};
    }
  }
}