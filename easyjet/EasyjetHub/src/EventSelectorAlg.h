/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EASYJET_EVENTSELECTORALG
#define EASYJET_EVENTSELECTORALG

#include "AnaAlgorithm/AnaAlgorithm.h"

#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysFilterReporterParams.h>

#include <xAODEventInfo/EventInfo.h>

namespace Easyjet
{

  /// \brief An algorithm for selecting events based on a pre-computed EventInfo decoration
  class EventSelectorAlg final : public EL::AnaAlgorithm
  {
    /// \brief The standard constructor
  public:
    EventSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;

  private:

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::EventInfo> m_eventHandle
      {this, "event", "EventInfo", "EventInfo container to read" };

    CP::SysReadDecorHandle<bool> m_filter_deco
      {this, "filterDecoration", "", "EventInfo boolean decoration used to filer events"};

    /// \brief Setup sys-aware output decorations
    CP::SysFilterReporterParams m_filterParams {this, "Event selection"};

  };
    
}


#endif
