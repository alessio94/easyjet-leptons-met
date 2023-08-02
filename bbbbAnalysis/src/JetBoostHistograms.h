#ifndef JET_HISTOGRAMS_H
#define JET_HISTOGRAMS_H

#include "xAODJet/JetFwd.h"

namespace H5 {
  class Group;
}
class IJetHist;

class JetBoostHistograms {
public:
  JetBoostHistograms();
  ~JetBoostHistograms();
  void fill(const xAOD::Jet& jet, float weight);
  void write(H5::Group& output_group);
private:
  std::vector<std::unique_ptr<IJetHist>> m_hists;
};

#endif
