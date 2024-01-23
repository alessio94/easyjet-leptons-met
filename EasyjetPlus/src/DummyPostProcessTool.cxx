/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "DummyPostProcessTool.h"

DummyPostProcessTool::DummyPostProcessTool(const std::string &t, const std::string &n, const IInterface *p)
  : base_class(t, n, p){}

StatusCode DummyPostProcessTool::initialize(){

  // For simply copying input variables, a tool with empty outVars can be set up
  const std::unordered_map<std::string, VarType> inVars = {
    {"runNumber", VarType::Int}
  };
  const std::unordered_map<std::string, VarType> outVars = {
    {"test", VarType::Float}
  };

  setIOVariables(inVars, outVars);

  return StatusCode::SUCCESS;
}

StatusCode DummyPostProcessTool::finalize(){
  return StatusCode::SUCCESS;
}

void DummyPostProcessTool::computeVariables
(const std::unordered_map<std::string, varTypePointer>& inVars,
 std::unordered_map<std::string, varTypePointer>& outVars) const{

  unsigned int runNumber = getContent<unsigned int>(inVars, "runNumber");

  setContent<float>(outVars, "test", runNumber + 100.);
  
}
