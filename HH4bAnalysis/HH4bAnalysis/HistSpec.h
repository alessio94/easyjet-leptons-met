///////////////////////// -*- C++ -*- /////////////////////////////
// HistSpec.h
// Header file for HistSpec objects specifying histogram parameters
//
// Author: T.J.Khoo<khoo@cern.ch>
///////////////////////////////////////////////////////////////////
// Always protect against multiple includes!
#ifndef HH4BANALYSIS_HISTSPEC_H
#define HH4BANALYSIS_HISTSPEC_H

#include <cstddef>
#include <string>
#include "Gaudi/Property.h"

namespace HH4B
{
  /// Declare structs for holding information that is naturally structured
  struct HistSpec1D
  {
    std::string histname;
    std::string histdescr;

    // These connect the C++ variables to the python configuration
    unsigned int nbinsx;
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
      parent->declareProperty(histname + "_nBinsx", nbinsx = _nbinsx, "Number of x bins for " + histname);
      parent->declareProperty(histname + "_xMin", xmin = _xmin, "Minimum x value for " + histname);
      parent->declareProperty(histname + "_xMax", xmax = _xmax, "Maximum x value for " + histname);
    }

  }; // end HistSpec1D

  /// Declare structs for holding information that is naturally structured
  struct HistSpec2D
  {
    std::string histname;
    std::string histdescr;

    // These connect the C++ variables to the python configuration
    unsigned int nbinsx;
    float xmin;
    float xmax;
    unsigned int nbinsy;
    float ymin;
    float ymax;

    // Args are in the same order as for the Property constructor
    // * Pointer to owning component
    // * Histogram name
    // * Nbinsx, xMin, xMax, Nbinsy, yMin, yMax
    // * Histogram description
    template <class OWNER>
    HistSpec2D(OWNER *parent, const std::string &_histname,
               unsigned int _nbinsx, float _xmin, float _xmax,
               unsigned int _nbinsy, float _ymin, float _ymax,
               const std::string &_histdescr) : histname{_histname},
                                                histdescr{_histdescr}
    {
      parent->declareProperty(histname + "_nBinsx", nbinsx = _nbinsx, "Number of x bins for " + histname);
      parent->declareProperty(histname + "_xMin", xmin = _xmin, "Minimum x value for " + histname);
      parent->declareProperty(histname + "_xMax", xmax = _xmax, "Maximum x value for " + histname);
      //
      parent->declareProperty(histname + "_nBinsy", nbinsy = _nbinsy, "Number of y bins for " + histname);
      parent->declareProperty(histname + "_yMin", ymin = _ymin, "Minimum y value for " + histname);
      parent->declareProperty(histname + "_yMax", ymax = _ymax, "Maximum y value for " + histname);
    }

  }; // end HistSpec2D

  /// Define a special HistSpec to handle jet kinematics --
  /// this will set up multiple histograms:
  ///   - 1D hists: one each for pt, eta, phi, m
  ///   - 2D hists: pt vs eta, eta vs phi (convention: y vs x)
  /// The x-ranges for pt & m are the only ones likely to vary
  /// Share nbins, can be made more flexible if we feel the need
  struct HistSpecJetKine
  {
    std::string histname;
    std::string histdescr;

    // Fixed by detector acceptance
    static constexpr float min_eta = -5;
    static constexpr float max_eta = 5;
    static constexpr float min_phi = -3.2; // Round number just larger than pi
    static constexpr float max_phi = 3.2;

    HistSpec1D ptHist, etaHist, phiHist, mHist;
    HistSpec2D ptVsEtaHist, etaPhiMap;

    // Slightly different arg ordering to the basic histspecs:
    // * Pointer to owning component
    // * Histogram name
    // * Histogram description
    // * Binning specifications (these have defaults)
    //   - nbins is used for everything except phi
    //   - for phi, define nbins_phi with a value that subdivides 6.4 well
    //   - min_pt, max_pt; min_m, max_m are customisable,
    //     whereas we fix the eta,phi ranges by detector acceptance
    template <class OWNER>
    HistSpecJetKine(OWNER *parent, const std::string &_histname,
                    const std::string &_histdescr,
                    unsigned int nbins = 100, unsigned int nbins_phi = 128,
                    float min_pt = 0., float max_pt = 500.,
                    float min_m = 0., float max_m = 200.) : histname{_histname},
                                                            histdescr{_histdescr},
                                                            // Initialisers for the individual hists -- 1D
                                                            ptHist{parent, _histname + "_pt", nbins, min_pt, max_pt, _histdescr + " -- pT hist"},
                                                            etaHist{parent, _histname + "_eta", nbins, min_eta, max_eta, _histdescr + " -- eta hist"},
                                                            phiHist{parent, _histname + "_phi", nbins_phi, min_phi, max_phi, _histdescr + " -- phi hist"},
                                                            mHist{parent, _histname + "_m", nbins, min_m, max_m, _histdescr + " -- m hist"},
                                                            // Initialisers for the individual hists -- 2D
                                                            ptVsEtaHist{parent, _histname + "_pt_vs_eta",
                                                                        nbins, min_eta, max_eta, nbins, min_pt, max_pt, _histdescr + " -- pT vs eta hist"},
                                                            etaPhiMap{parent, _histname + "_phi_vs_eta",
                                                                      nbins, min_eta, max_eta, nbins_phi, min_phi, max_phi, _histdescr + " -- phi vs eta hist"}
    {
    }

  }; // end HistSpecJetKine

}

#endif
