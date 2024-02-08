/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Example of PostProcessorTool for EasyJet ntuple, to be adapted for each analysis

#ifndef TotalWeightsTool_H
#define TotalWeightsTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "EasyjetPlus/IPostProcessTool.h"

using namespace PostProc;

class TotalWeightsTool final: public extends<AthAlgTool, IPostProcessTool>
{
 public:
  TotalWeightsTool(const std::string&,const std::string&,const IInterface*);
  virtual ~TotalWeightsTool() = default;
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  
  virtual void computeVariables
    (const std::unordered_map<std::string, varTypePointer>& inVars,
     const std::unordered_map<std::string, std::vector<float>*>& inVecVars,
     std::unordered_map<std::string, varTypePointer>& outVars) const override;

 private:
  Gaudi::Property<std::string> m_analysis {this, "analysis", "", "Analysis prefix"};
  Gaudi::Property<int> m_nLepton {this, "nLeptons", 0, "Number of leptons"};
  Gaudi::Property<int> m_nPhoton {this, "nPhotons", 0, "Number of photons"};
  Gaudi::Property<int> m_nTau {this, "nTaus", 0, "Number of taus"};
  Gaudi::Property<std::string> m_bTagWP {this, "bTagWP", "", "btagging working point"};
  Gaudi::Property<std::string> m_totalWeightName {this, "totalWeightName", "weight", "Name of total weight branch"};

  std::vector<std::string> m_SF_names;
  
};

#endif
