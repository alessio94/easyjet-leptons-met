/*
   Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
 */

#pragma once

// EDM include(s):
#include "AsgTools/IAsgTool.h"
#include "xAODEventInfo/EventInfo.h"

// Local include(s):
#include "TruthWeightTools/HiggsWeights.h"

namespace TruthWeightTools
{
  /// Interface for xAOD Truth Weight Tool which retrieves
  /// Meta Data from a truth record to interface the event
  /// weights
  ///
  /// @author James Robinson <james.robinson@cern.ch>
  ///
  class IHiggsWeightTool: public virtual asg::IAsgTool
  {
    /// Declare the interface that the class provides
    ASG_TOOL_INTERFACE(TruthWeightTools::IHiggsWeightTool)

  public:
    /// Inherited from IAsgTool
    virtual StatusCode initialize() = 0;

    /// Write out end-of-run statistics
    virtual void printSummary() = 0;

    /// Access the HiggsWeights
    virtual HiggsWeights getHiggsWeights(int HTXS_Njets30 = -1, double HTXS_pTH = -99.0, int HTXS_cat = -1, const xAOD::EventInfo* eventInfo=NULL) = 0;

    // With the new STXS S1p1 binning
    virtual HiggsWeights getHiggsWeights(int HTXS_Njets30, double HTXS_pTH, int HTXS_cat, int HTXS_cat_1p1, int STXS_Stage1p1Fine, const xAOD::EventInfo* eventInfo) = 0;

  }; // class IHiggsWeightTool

} // namespace TruthWeightTools
