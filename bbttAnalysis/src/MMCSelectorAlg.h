/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BBTTANALYSIS_MMCSELECTORALG
#define BBTTANALYSIS_MMCSELECTORALG

#include "AnaAlgorithm/AnaAlgorithm.h"

#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysFilterReporterParams.h>

#include <xAODEventInfo/EventInfo.h>

#include "HHbbttEnums.h"

namespace HHBBTT
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
      { this, "MMC_min", 60000, "Minimal MMC value" };

    Gaudi::Property<bool> m_bypass
      { this, "bypass", false, "Run selector algorithm in pass-through mode" };

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

    CP::SysReadDecorHandle<bool> m_pass_LepHad {this, "passLepHad", "pass_baseline_LepHad_%SYS%", "events pass lep-had"};
    CP::SysReadDecorHandle<bool> m_pass_HadHad {this, "passHadHad", "pass_baseline_HadHad_%SYS%", "events pass had-had"};

    CP::SysReadDecorHandle<float> 
      m_mmc_m { this, "mmc_m", "mmc_m_%SYS%", "MMC mass key"};

    CP::SysFilterReporterParams m_filterParams {this, "HHbbtautau selection"};

    Gaudi::Property<std::vector<std::string>> m_channel_names
      { this, "channel", {}, "Which channel to run" };

    std::vector<HHBBTT::Channel> m_channels;

  };
}

#endif
