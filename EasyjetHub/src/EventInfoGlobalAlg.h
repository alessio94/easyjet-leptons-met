#ifndef EventInfoGlobalAlg_H
#define EventInfoGlobalAlg_H

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <xAODEventInfo/EventInfo.h>

#include <AsgDataHandles/WriteDecorHandle.h>
#include <StoreGate/ReadDecorHandle.h>


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

    // Get year of data taking
    inline unsigned int getDataTakingYear(std::vector<unsigned int> years, unsigned int rNumber ) {

        if (years.size() == 1)
            return years.at(0);
        //Get single run year per event in case of MC20a which corresponds to 2015+2016
        else if (years.size() == 2) {
            if (266904 <= rNumber && rNumber <= 284484)
                return 2015;
            else if (296939 <= rNumber && rNumber <= 311481)
                return 2016;
            else { 
              ATH_MSG_ERROR("Wrong (or unkown) combination of year and (Random)runNumber");
              return 0;
            }

        }
        else
          ATH_MSG_ERROR("Wrong (or unkown) combination of year and (Random)runNumber");

        return 0;
    }

private:

    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{
        this, "EventInfoKey", "EventInfo", "EventInfo container to dump"};

  // Necessary additions to get the year of data taking per event.
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_runNumberKey{
        this, "runNumberDecorKey", "EventInfo.runNumber", "Run number"};
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_rdmRunNumberKey{
        this, "RandomRunNumberDecorKey", "EventInfo.RandomRunNumber", "Random run number"};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_YearsDecorKey{
        this, "YearDecorKey", "EventInfo.dataTakingYear", "Data taking year"};

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };
    Gaudi::Property<std::vector<unsigned int>> m_years
      { this, "Years", false, "which years are running" };  

};

}

#endif
