#ifndef EVENT_COUNTER_ALG_H
#define EVENT_COUNTER_ALG_H

#include "AthenaBaseComps/AthAlgorithm.h"

#include <atomic>

class EventCounterAlg: public AthAlgorithm
{
public:
  EventCounterAlg(const std::string& name, ISvcLocator* loc);

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override;

private:
  Gaudi::Property<std::string> m_outputFile {
    this, "output", "", "Name of output json file"
  };
  Gaudi::Property<std::string> m_countName {
    this, "countName", "n_events", "name for counts in json file"
  };

  std::atomic<unsigned long long> m_n_events;
};


#endif
