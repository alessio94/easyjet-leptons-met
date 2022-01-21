///////////////////////// -*- C++ -*- /////////////////////////////
// HistSpec.h
// Header file for HistSpec objects specifying histogram parameters
//
// Author: T.J.Khoo<khoo@cern.ch>
///////////////////////////////////////////////////////////////////
// Always protect against multiple includes!
#ifndef PLOTTING_HISTSPEC_H
#define PLOTTING_HISTSPEC_H

#include <cstddef>
#include <string>

namespace MSA
{
  /// Declare structs for holding information that is naturally structured
  struct HistSpec1D
  {
    std::string histname;
    std::string histdescr;

    // These connect the C++ variables to the python configuration
    size_t nbinsx;
    float xmin;
    float xmax;

    // Needs a custom constructor to pass parameters to the properties
    // Args are in the same order as for the Property constructor
    // * Pointer to owning component
    // * Histogram name
    // * Nbinsx, xMin, xMax`
    // * Histogram description
    // Templated because declareProperty is a method of the owning class
    // but is not in a common base class of AthAlgorithm and AthAlgTool.
    template <class OWNER>
    HistSpec1D(OWNER *parent, const std::string &_histname,
               unsigned int _nbinsx, float _xmin, float _xmax,
               const std::string &_histdescr) : histname{_histname},
                                                histdescr{_histdescr}
    {
      // This invokes the actual command used inside Gaudi::Property and
      // Tool/DataHandle constructors to establish the C++/python connection
      parent->declareProperty(histname + "_nBinsx", nbinsx = _nbinsx, "Number of x bins for " + histname);
      parent->declareProperty(histname + "_xMin", xmin = _xmin, "Minimum x value for " + histname);
      parent->declareProperty(histname + "_xMax", xmax = _xmax, "Maximum x value for " + histname);
    }
  };

}

#endif
