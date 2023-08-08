#ifndef JET_HISTOGRAMS_H
#define JET_HISTOGRAMS_H

#include "xAODJet/JetFwd.h"

namespace H5 {
  class Group;
}

namespace bhist {

  class IJetHist;

  class JetHists {
  public:
    JetHists();
    ~JetHists();
    void fill(const xAOD::Jet& jet, float weight);
    void write(H5::Group& output_group) const;
  private:
    std::vector<std::unique_ptr<IJetHist>> m_hists;
  };

}

#endif
