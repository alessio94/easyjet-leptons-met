/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VBSVV4qANALYSIS_TRIGGERUTILS
#define VBSVV4qANALYSIS_TRIGGERUTILS

#include <AsgDataHandles/ReadDecorHandle.h>
#include "TriggerDecoratorAlg.h"
#include <xAODEventInfo/EventInfo.h>

namespace VBSVV4q
{
	
	std::vector<std::string> getSingleLargeRJetsTriggers(int year);
    
}

#endif