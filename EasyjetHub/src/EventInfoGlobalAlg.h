#ifndef EventInfoGlobalAlg_H
#define EventInfoGlobalAlg_H

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <xAODEventInfo/EventInfo.h>
#include "xAODTracking/VertexContainer.h"

#include <StoreGate/WriteDecorHandleKey.h>
#include <StoreGate/ReadDecorHandleKey.h>
#include <StoreGate/WriteDecorHandleKeyArray.h>

namespace Easyjet
{

  /// \brief An algorithm for getting the data-taking years per event/sample.
  class EventInfoGlobalAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    EventInfoGlobalAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any


private:

    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{
        this, "EventInfoKey", "EventInfo", "EventInfo container to dump"};

    // Necessary additions to get the year of data taking per event.
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_runNumberKey{
        this, "runNumberDecorKey", "EventInfo.runNumber", "Run number"};
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_rdmRunNumberKey{
        this, "RandomRunNumberDecorKey", "EventInfo.RandomRunNumber", "Random run number"};

    ///Primary vertex container
    SG::ReadHandleKey<xAOD::VertexContainer>  m_vertexContainerKey
      {this,"VertexContainerName", "PrimaryVertices"};

    SG::WriteDecorHandleKey<xAOD::EventInfo> m_nVertexDecorKey;
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_PVxDecorKey;
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_PVyDecorKey;
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_PVzDecorKey;
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_yearDecorKey;

    // References:
    // https://atlas-tagservices.cern.ch/tagservices/RunBrowser/runBrowserReport/rBR_Period_Report.php
    std::vector<std::tuple<std::string, unsigned int, unsigned int>>
      m_runPeriods = {
      {"2016_periodA", 296939, 300287},
      {"2016_periodB_D3", 300345, 302872},
      {"2016_periodD4_end", 302919, 311481},
      {"2017_periodB1_B4", 325713, 326695},
      {"2017_periodB5_B7", 326834, 327490},
      {"2017_periodB5_B8", 326834, 328393},
      {"2017_periodB8_end", 327582, 341649},
      {"2018_periodB_end", 348885, 364485},
      {"2018_tau35_medium1", 350067, 364292},
      {"2018_periodK_end", 355529, 364485},
      {"2022_75bunches", 427882, 428070},
      {"2023_75bunches", 450360, 450893},
      {"2023_400bunches", 450894, 451093},
      {"2023_from1200bunches", 451587, 456749},
      {"2023_first_2400bunches", 451896, 456749},
      {"2024_75bunches", 470781, 472942},
      {"2024_400bunches_periodD", 472943, 473182},
      {"2024_400bunches_periodL", 480146, 480161},
      // References: https://twiki.cern.ch/twiki/bin/viewauth/Atlas/TauTriggerChanges2024
      {"2024_timeframeA_C", 472553, 479496},
      {"2024_timeframeD", 479507, 480828},
      {"2024_timeframeE_F", 480957, 486706},
    };

    SG::WriteDecorHandleKeyArray<xAOD::EventInfo> m_runPeriodsDecor_keys;

    SG::WriteDecorHandleKey<xAOD::EventInfo> m_L1TopoDisabledDecorKey;

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };
    Gaudi::Property<std::vector<unsigned int>> m_years
      { this, "Years", false, "which years are running" };  
    Gaudi::Property<bool> m_doRetrievePV{
      this, "doRetrievePV", false, "retrive PV information?"};

};

}

#endif
