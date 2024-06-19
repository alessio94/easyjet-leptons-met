/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BBTTANALYSIS_TRIGGERUTILS
#define BBTTANALYSIS_TRIGGERUTILS

#include <AsgDataHandles/ReadDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>

#include "HHbbttEnums.h"

namespace HHBBTT
{

  typedef std::unordered_map<HHBBTT::RunBooleans, SG::ReadDecorHandle<xAOD::EventInfo, bool> > runBoolReadDecoMap;
  
  void getSingleMuTriggers(int year, const xAOD::EventInfo* eventInfo,
			   const runBoolReadDecoMap& runBoolDecos,
			   std::vector<std::string>& single_mu_paths,
			   std::string& single_mu_SF_path);
  inline void getSingleMuTriggers(int year, const xAOD::EventInfo* eventInfo,
				  const runBoolReadDecoMap& runBoolDecos,
				  std::vector<std::string>& single_mu_paths){
    std::string single_mu_SF_path;
    getSingleMuTriggers(year, eventInfo, runBoolDecos, single_mu_paths,
			single_mu_SF_path);
  }
  inline void getSingleMuTriggers(int year, const xAOD::EventInfo* eventInfo,
				  const runBoolReadDecoMap& runBoolDecos,
				  std::string& single_mu_SF_path){
    std::vector<std::string> single_mu_paths;
    getSingleMuTriggers(year, eventInfo, runBoolDecos, single_mu_paths,
			single_mu_SF_path);
  }

  void getSingleEleTriggers(int year, const xAOD::EventInfo* eventInfo,
			    const runBoolReadDecoMap& runBoolDecos,
			    std::vector<std::string>& single_ele_paths,
			    std::string& single_ele_SF_path);
  inline void getSingleEleTriggers(int year, const xAOD::EventInfo* eventInfo,
				   const runBoolReadDecoMap& runBoolDecos,
				   std::vector<std::string>& single_ele_paths){
    std::string single_ele_SF_path;
    getSingleEleTriggers(year, eventInfo, runBoolDecos, single_ele_paths,
			 single_ele_SF_path);
  }
  inline void getSingleEleTriggers(int year, const xAOD::EventInfo* eventInfo,
				   const runBoolReadDecoMap& runBoolDecos,
				   std::string& single_ele_SF_path){
    std::vector<std::string> single_ele_paths;
    getSingleEleTriggers(year, eventInfo, runBoolDecos, single_ele_paths,
			 single_ele_SF_path);
  }

  void getMuTauTriggers(int year, const xAOD::EventInfo* eventInfo,
			const runBoolReadDecoMap& runBoolDecos,
			std::vector<std::string>& mu_tau_paths_2016,
			std::vector<std::string>& mu_tau_paths_low,
			std::vector<std::string>& mu_tau_paths_high,
			std::pair<std::string, std::string>& mu_tau_SF_path);
  inline void getMuTauTriggers(int year, const xAOD::EventInfo* eventInfo,
			       const runBoolReadDecoMap& runBoolDecos,
			       std::vector<std::string>& mu_tau_paths_2016,
			       std::vector<std::string>& mu_tau_paths_low,
			       std::vector<std::string>& mu_tau_paths_high){
    std::pair<std::string, std::string> mu_tau_SF_path;
    getMuTauTriggers(year, eventInfo, runBoolDecos,
		     mu_tau_paths_2016, mu_tau_paths_low, mu_tau_paths_high,
		     mu_tau_SF_path);
  }
  inline void getMuTauTriggers(int year, const xAOD::EventInfo* eventInfo,
			       const runBoolReadDecoMap& runBoolDecos,
			       std::pair<std::string, std::string>& mu_tau_SF_path){
    std::vector<std::string> mu_tau_paths_2016, mu_tau_paths_low, mu_tau_paths_high;
    getMuTauTriggers(year, eventInfo, runBoolDecos,
		     mu_tau_paths_2016, mu_tau_paths_low, mu_tau_paths_high,
		     mu_tau_SF_path);
  }

  void getEleTauTriggers(int year, const xAOD::EventInfo* eventInfo,
			 const runBoolReadDecoMap& runBoolDecos,
			 std::vector<std::string>& ele_tau_paths,
			 std::vector<std::string>& ele_tau_paths_4J12,
			 std::pair<std::string, std::string>& ele_tau_SF_path);
  inline void getEleTauTriggers(int year, const xAOD::EventInfo* eventInfo,
				const runBoolReadDecoMap& runBoolDecos,
				std::vector<std::string>& ele_tau_paths,
				std::vector<std::string>& ele_tau_paths_4J12){
    std::pair<std::string, std::string> ele_tau_SF_path;
    getEleTauTriggers(year, eventInfo, runBoolDecos,
		      ele_tau_paths, ele_tau_paths_4J12, ele_tau_SF_path);
  }
  inline void getEleTauTriggers(int year, const xAOD::EventInfo* eventInfo,
				const runBoolReadDecoMap& runBoolDecos,
				std::pair<std::string, std::string>& ele_tau_SF_path){
    std::vector<std::string> ele_tau_paths, ele_tau_paths_4J12;
    getEleTauTriggers(year, eventInfo, runBoolDecos,
		      ele_tau_paths, ele_tau_paths_4J12, ele_tau_SF_path);
  }

  void getSingleTauTriggers(int year, const xAOD::EventInfo* eventInfo,
			    const runBoolReadDecoMap& runBoolDecos,
			    std::vector<std::string>& single_tau_paths,
			    std::string& single_tau_SF_path);
  inline void getSingleTauTriggers(int year, const xAOD::EventInfo* eventInfo,
				   const runBoolReadDecoMap& runBoolDecos,
				   std::vector<std::string>& single_tau_paths){
    std::string single_tau_SF_path;
    getSingleTauTriggers(year, eventInfo, runBoolDecos, single_tau_paths,
			 single_tau_SF_path);
  }
  inline void getSingleTauTriggers(int year, const xAOD::EventInfo* eventInfo,
				   const runBoolReadDecoMap& runBoolDecos,
				   std::string& single_tau_SF_path){
    std::vector<std::string> single_tau_paths;
    getSingleTauTriggers(year, eventInfo, runBoolDecos, single_tau_paths,
			 single_tau_SF_path);
  }

  void getDiTauTriggers(int year, const xAOD::EventInfo* eventInfo,
			const runBoolReadDecoMap& runBoolDecos,
			std::vector<std::string>& ditau_paths_2016,
			std::vector<std::string>& ditau_paths_L1Topo,
			std::vector<std::string>& ditau_paths_4J12,
			std::vector<std::string>& ditau_paths_L1Topo_delayed,
			std::vector<std::string>& ditau_paths_4J12_delayed,
			std::pair<std::string, std::string>& di_tau_SF_path);
  inline void getDiTauTriggers(int year, const xAOD::EventInfo* eventInfo,
			       const runBoolReadDecoMap& runBoolDecos,
			       std::vector<std::string>& ditau_paths_2016,
			       std::vector<std::string>& ditau_paths_L1Topo,
			       std::vector<std::string>& ditau_paths_4J12,
			       std::vector<std::string>& ditau_paths_L1Topo_delayed,
			       std::vector<std::string>& ditau_paths_4J12_delayed){
    std::pair<std::string, std::string> di_tau_SF_path;
    getDiTauTriggers(year, eventInfo, runBoolDecos,
		     ditau_paths_2016, ditau_paths_L1Topo, ditau_paths_4J12,
		     ditau_paths_L1Topo_delayed, ditau_paths_4J12_delayed,
		     di_tau_SF_path);
  }
  inline void getDiTauTriggers(int year, const xAOD::EventInfo* eventInfo,
			       const runBoolReadDecoMap& runBoolDecos,
			       std::pair<std::string, std::string>& di_tau_SF_path){
    std::vector<std::string> ditau_paths_2016, ditau_paths_L1Topo, ditau_paths_4J12,
      ditau_paths_L1Topo_delayed, ditau_paths_4J12_delayed;
    getDiTauTriggers(year, eventInfo, runBoolDecos,
		     ditau_paths_2016, ditau_paths_L1Topo, ditau_paths_4J12,
		     ditau_paths_L1Topo_delayed, ditau_paths_4J12_delayed,
		     di_tau_SF_path);
  }

  void getDiBJetTriggers(int year, const xAOD::EventInfo* eventInfo,
			 const runBoolReadDecoMap& runBoolDecos,
			 std::vector<std::string> dib_paths);

}

#endif
