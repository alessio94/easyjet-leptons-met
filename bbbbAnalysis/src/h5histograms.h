#ifndef HISTOGRAMS_H
#define HISTOGRAMS_H

namespace H5 {
  class Group;
}

#include <string>

namespace h5h {
  template <typename T>
  void write_hist_to_group(
    H5::Group& group,
    const T& hist,
    const std::string& name);
}

#include "h5histograms.icc"

#endif
