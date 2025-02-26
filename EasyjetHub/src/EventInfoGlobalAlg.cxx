#include "EventInfoGlobalAlg.h"

namespace Easyjet
{

  EventInfoGlobalAlg::EventInfoGlobalAlg(const std::string &name,
                                       ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator) { }

  StatusCode EventInfoGlobalAlg::initialize() {

    ATH_CHECK(m_EventInfoKey.initialize());
    ATH_CHECK(m_runNumberKey.initialize());
    ATH_CHECK(m_rdmRunNumberKey.initialize());

    ATH_CHECK(m_vertexContainerKey.initialize());

    m_yearDecorKey = "EventInfo.dataTakingYear";
    ATH_CHECK(m_yearDecorKey.initialize());

    m_nVertexDecorKey = "EventInfo.nPrimaryVertices";
    ATH_CHECK(m_nVertexDecorKey.initialize());

    for(const auto& period : m_runPeriods){
      m_runPeriodsDecor_keys.emplace_back("EventInfo.is"+std::get<0>(period));
      ATH_CHECK(m_runPeriodsDecor_keys.back().initialize());
    }

    m_L1TopoDisabledDecorKey = "EventInfo.l1TopoDisabled";
    ATH_CHECK(m_L1TopoDisabledDecorKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode EventInfoGlobalAlg::execute() {

    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    ATH_CHECK(eventInfo.isValid());

    SG::ReadDecorHandle<xAOD::EventInfo, unsigned int> runNumberHandle(m_runNumberKey);
    SG::ReadDecorHandle<xAOD::EventInfo, unsigned int> rdmRunNumberHandle(m_rdmRunNumberKey);
    unsigned int runNumber = m_isMC ? rdmRunNumberHandle(*eventInfo) : runNumberHandle(*eventInfo);

    // Save Year of Data taking per event
    SG::WriteDecorHandle<xAOD::EventInfo, unsigned int> yearDecorHandle(m_yearDecorKey);

    int year = 0;
    if (m_years.size() == 1) year = m_years[0];
    // Get single run year per event in case of MC20a which corresponds
    // to 2015+2016
    else if (m_years.size() == 2) {
      if (266904 <= runNumber && runNumber <= 284484)
        year = 2015;
      else if (296939 <= runNumber && runNumber <= 311481)
        year = 2016;
      else 
         ATH_MSG_ERROR("Wrong (or unknown) combination of year and (Random)runNumber");
    } else {
      ATH_MSG_ERROR("Wrong (or unknown) combination of year and (Random)runNumber");
    }
    yearDecorHandle(*eventInfo) = year;

    for(unsigned int i=0; i<m_runPeriods.size(); i++){
      SG::WriteDecorHandle<xAOD::EventInfo, bool> handle(m_runPeriodsDecor_keys[i]);
      handle(*eventInfo) = std::get<1>(m_runPeriods[i]) <= runNumber &&
        runNumber <= std::get<2>(m_runPeriods[i]);
    }

    SG::WriteDecorHandle<xAOD::EventInfo, bool> l1TopoDisabled(m_L1TopoDisabledDecorKey);
    l1TopoDisabled(*eventInfo) =
      (runNumber == 336506) || (runNumber == 336548) || (runNumber == 336567);

    // Save number of primary vertices
    SG::ReadHandle<xAOD::VertexContainer> vertexContainer(m_vertexContainerKey);
    ATH_CHECK(vertexContainer.isValid());

    SG::WriteDecorHandle<xAOD::EventInfo, unsigned int> nVertexDecorHandle(m_nVertexDecorKey);
    nVertexDecorHandle(*eventInfo) = vertexContainer->size();

    return StatusCode::SUCCESS;
  }
}
