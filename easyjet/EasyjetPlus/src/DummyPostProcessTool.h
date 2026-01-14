/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Example of PostProcessorTool for EasyJet ntuple, to be adapted for each analysis

#ifndef DummyPostProcessTool_H
#define DummyPostProcessTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "EasyjetPlus/IPostProcessTool.h"

using namespace PostProc;

class DummyPostProcessTool final: public extends<AthAlgTool, IPostProcessTool>
{
 public:
  DummyPostProcessTool(const std::string&,const std::string&,const IInterface*);
  virtual ~DummyPostProcessTool() = default;
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  
  virtual void computeVariables
    (const std::unordered_map<std::string, varTypePointer>& inVars,
     const std::unordered_map<std::string, std::vector<float>*>& inVecVars,
     std::unordered_map<std::string, varTypePointer>& outVars) const override;
  
};

#endif
