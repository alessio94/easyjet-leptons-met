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
			      std::vector<std::string>& single_ele_paths,
            std::string& single_ele_SF_path);

    void getSingleMuTriggers(int year, const xAOD::EventInfo* event,
			     const runBoolReadDecoMap& runBoolDecos,
			     std::vector<std::string>& single_mu_paths,
           std::string& single_mu_SF_path);
}

#endif
