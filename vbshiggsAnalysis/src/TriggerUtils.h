/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VBSHIGGSANALYSIS_TRIGGERUTILS
#define VBSHIGGSANALYSIS_TRIGGERUTILS

#include <AsgDataHandles/ReadDecorHandle.h>
#include "TriggerDecoratorAlg.h"
#include <xAODEventInfo/EventInfo.h>

namespace VBSHIGGS
{
    typedef std::unordered_map<VBSHIGGS::RunBooleans, SG::ReadDecorHandle<xAOD::EventInfo, bool> > runBoolReadDecoMap;

    void getSingleEleTriggers(int year, const xAOD::EventInfo* event,
			      const runBoolReadDecoMap& runBoolDecos,
			      std::vector<std::string>& single_ele_paths);

    void getSingleMuTriggers(int year, const xAOD::EventInfo* event,
			     const runBoolReadDecoMap& runBoolDecos,
			     std::vector<std::string>& single_mu_paths);

    void getDiEleTriggers(int year, const xAOD::EventInfo* event,
			  const runBoolReadDecoMap& runBoolDecos,
			  std::vector<std::string>& di_ele_paths);

    void getDiMuTriggers(int year, std::vector<std::string>& di_mu_paths);

    void getAsymLep2Triggers(int year,
			     std::vector<std::string>& asym_lepton_paths);

    void getAsymLep1emTriggers(int year,
			       std::vector<std::string>& asym_lepton_paths);

    void getAsymLep1meTriggers(int year,
			       std::vector<std::string>& asym_lepton_paths);
}

#endif
