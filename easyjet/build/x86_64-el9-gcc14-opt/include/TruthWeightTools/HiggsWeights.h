/*
   Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
 */

#pragma once

// // EDM include(s):
// #include "AsgTools/AsgMessaging.h"

// STL include(s):
#include <vector>

namespace TruthWeightTools
{
  /// Simple class for Higgs weights
  ///
  /// @author Dag Gillberg <dag.gillberg@cern.ch>
  ///
  class HiggsWeights
  {
  public:

    /// Nominal event weight
    double nominal, weight0;

    /// 30 PDF4LHC uncertainty variations + alphaS up/down
    std::vector<double> pdf4lhc_unc, nnpdf30_unc;
    double alphaS_up, alphaS_dn;

    /// Weights to reweigh central to a different PDF set
    /// If the PDF set is not present in file, this weight will be zero
    double nnpdf30_nlo, nnpdf30_nnlo, mmht2014nlo, pdf4lhc_nlo, pdf4lhc_nnlo;
    double ct10nlo, ct10nlo_0118, ct14nlo, ct14nlo_0118;

    /// QCD scale variations (muR,muF)
    std::vector<double> qcd;

    /// Special weights for Powheg ggF NNLOPS
    /// 1. QCD scale variations 3x(NNLO), 9xPowheg(muR,muF) - 26 variations
    std::vector<double> ggF_qcd_nnlops;

    /// information of the current event kinematiocs
    double pTH;
    int Njets30, STXS;

    // ggF systematics
    std::vector<double> ggF_scheme;

    // VBF systematics
    std::vector<double> qq2Hqq_VBF_scheme;

    // qqH systematics
    std::vector<double> qq2Hqq_scheme;

    // VH-lep systematics
    std::vector<double> qq2Hll_scheme;
    std::vector<double> gg2Hll_scheme;

    // ttH systematics
    std::vector<double> ttH_scheme;

    /// methods to print weights to the screen
    char *uncStr(double var, double nom);

    void print();
  };

}