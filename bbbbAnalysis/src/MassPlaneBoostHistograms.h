#ifndef MASS_PLANE_HISTOGRAMS_H
#define MASS_PLANE_HISTOGRAMS_H

#include "xAODJet/JetFwd.h"

#include <string>
#include <set>
#include <vector>
#include <memory>

namespace H5 {
  class Group;
}

namespace bhist {

  class MassHist;
  using Jets = std::vector<const xAOD::Jet*>;

  struct MassPlaneHistsConfig
  {
    std::string truth_matching_prefix = "";
    std::set<std::string> matching_methods;
  };

  class MassPlaneHists
  {
  public:
    MassPlaneHists(const MassPlaneHistsConfig&);
    ~MassPlaneHists();
    // pass by value because we immediately mutate the container
    void fill(Jets jets, float weight);
    void write(H5::Group& output_group) const;
  private:
    std::vector<std::unique_ptr<MassHist>> m_mpl;
  };

}

#endif
