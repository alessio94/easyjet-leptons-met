#include "EventCounterAlg.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <filesystem>

EventCounterAlg::EventCounterAlg(const std::string &name,
                                 ISvcLocator* loc):
  AthAlgorithm(name, loc),
  m_n_events(0)
{
}

StatusCode EventCounterAlg::initialize() {
  if (m_outputFile.value().empty()) {
    ATH_MSG_ERROR("Event counter file name not given");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode EventCounterAlg::execute() {
  m_n_events++;
  return StatusCode::SUCCESS;
}

StatusCode EventCounterAlg::finalize() {
  namespace fs = std::filesystem;
  using list_t = std::vector<std::pair<std::string, unsigned long long>>;

  fs::path output_file = m_outputFile.value();
  nlohmann::json metadata;
  if (fs::exists(output_file)) {
    std::ifstream stream(output_file.string());
    metadata = nlohmann::json::parse(stream);
  } else {
    metadata = list_t();
  }
  std::string key = m_countName.value();
  for (auto& [oldkey, count]: metadata.get<list_t>()) {
    if (oldkey == key) {
      ATH_MSG_ERROR(
        "metadata key " << key << " already exists in " << output_file);
      return StatusCode::FAILURE;
    }
  }
  // this conversion from atomic seems needed for nlohmann
  unsigned long long n_events = m_n_events;
  metadata.push_back({key, n_events});
  std::ofstream metastream(m_outputFile.value());
  // dump with 2 indents per level of nesting
  metastream << metadata.dump(2);
  return StatusCode::SUCCESS;
}
