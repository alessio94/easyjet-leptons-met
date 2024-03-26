/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TotalWeightsTool.h"

TotalWeightsTool::TotalWeightsTool(const std::string &t, const std::string &n, const IInterface *p)
  : base_class(t, n, p){}

StatusCode TotalWeightsTool::initialize(){

  std::unordered_map<std::string, VarType> inVars;

  m_SF_names = {
    "ftag_effSF_"+m_bTagWP+"_NOSYS",
    "jvt_effSF_NOSYS",
    "PileupWeight_NOSYS"
  };

  const std::vector<std::pair<std::string, int>> objects = {
    {"Lepton", m_nLepton},
    {"Photon", m_nPhoton},
    {"Tau", m_nTau}
  };

  for(const auto& obj : objects){
    std::string name = obj.first;
    int n = obj.second;
    if(n==0) continue;
    for(int i=0; i<n; i++){
      std::string index = (n==0) ? "" : std::to_string(i+1);
      std::string SFname = m_analysis + "_" + name + index + "_effSF_NOSYS";
      m_SF_names.push_back(SFname);
    }
  } 

  inVars[m_MCWeightName] = VarType::Float;
  for(const auto& name : m_SF_names){
    inVars[name] = VarType::Float;
  }

  const std::vector<std::string> inVecVars = {};
  const std::unordered_map<std::string, VarType> outVars = {
    {m_totalWeightName, VarType::Float}
  };

  setIOVariables(inVars, inVecVars, outVars);

  return StatusCode::SUCCESS;
}

StatusCode TotalWeightsTool::finalize(){
  return StatusCode::SUCCESS;
}

void TotalWeightsTool::computeVariables
(const std::unordered_map<std::string, varTypePointer>& inVars,
 const std::unordered_map<std::string, std::vector<float>*>& /*inVecVars*/,
 std::unordered_map<std::string, varTypePointer>& outVars) const{

  float weight = getContent<float>(inVars, m_MCWeightName);

  // Need to run SumOfWeightsTool first to get this in outVars
  weight /= getContent<float>(outVars, "sumOfWeights");
  
  for(const auto& name : m_SF_names){
    float SF = getContent<float>(inVars, name);
    if(SF >= 0) weight *= SF;
  }
  
  // Need to run GetXSectionTool first to get those in outVars
  weight *= getContent<float>(outVars, "AMIXsection")*1e3; // XS is stored in pb
  weight *= getContent<float>(outVars, "kFactor");
  weight *= getContent<float>(outVars, "FilterEff");
  weight *= getContent<float>(outVars, "Luminosity"); // Lumi is stored in fb-1
  
  setContent<float>(outVars, m_totalWeightName, weight);
  
}

