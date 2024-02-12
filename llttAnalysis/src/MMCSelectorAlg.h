/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LLTTANALYSIS_MMCSELECTORALG
#define LLTTANALYSIS_MMCSELECTORALG

#include "AnaAlgorithm/AnaAlgorithm.h"

#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysFilterReporterParams.h>

#include <xAODEventInfo/EventInfo.h>

namespace HLLTT
{

  /// \brief An algorithm for counting containers
  class MMCSelectorAlg final : public EL::AnaAlgorithm
  {
    /// \brief The standard constructor
  public:
    MMCSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// \brief Finalisation method, for cleanup, final print out etc
    StatusCode finalize() override;


  private:

   Gaudi::Property<float> m_mmc_min
      { this, "MMC_min", 0, "Minimal MMC value" };

    Gaudi::Property<bool> m_bypass
      { this, "bypass", false, "Run selector algorithm in pass-through mode" };

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

    CP::SysReadDecorHandle<float> 
      m_mmc_m { this, "mmc_m", "mmc_m_%SYS%", "MMC mass key"};

    CP::SysFilterReporterParams m_filterParams {this, "Hlltt selection"};

    /// \brief Internal variables

    bool MMC_MASS;

  };
}

#endif
