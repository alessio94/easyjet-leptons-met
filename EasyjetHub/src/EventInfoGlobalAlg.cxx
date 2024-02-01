#include "EventInfoGlobalAlg.h"

namespace Easyjet
{

EventInfoGlobalAlg::EventInfoGlobalAlg(const std::string &name,
                                       ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm(name, pSvcLocator) { }

StatusCode EventInfoGlobalAlg::initialize() {

    ATH_CHECK(m_EventInfoKey.initialize());
    ATH_CHECK(m_YearsDecorKey.initialize());
    ATH_CHECK(m_runNumberKey.initialize());
    ATH_CHECK(m_rdmRunNumberKey.initialize());

    return StatusCode::SUCCESS;
}

StatusCode EventInfoGlobalAlg::execute() {

    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    ATH_CHECK(eventInfo.isValid());
  // Save Year of Data taking per event
    SG::WriteDecorHandle<xAOD::EventInfo, unsigned int> m_YearsDecorHandle(m_YearsDecorKey);
    SG::ReadDecorHandle<xAOD::EventInfo, unsigned int> m_runNumberHandle(m_runNumberKey);
    SG::ReadDecorHandle<xAOD::EventInfo, unsigned int> m_rdmRunNumberHandle(m_rdmRunNumberKey);

    unsigned int rdmNumber = m_isMC ? m_rdmRunNumberHandle(*eventInfo) : m_runNumberHandle(*eventInfo);

    if (getDataTakingYear(m_years,rdmNumber)==0)
      return StatusCode::FAILURE;

    m_YearsDecorHandle(*eventInfo) = getDataTakingYear(m_years,rdmNumber);

    return StatusCode::SUCCESS;
}
}