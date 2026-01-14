/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "SumOfWeightsTool.h"

#include "TRegexp.h"
#include "TKey.h"

SumOfWeightsTool::SumOfWeightsTool(const std::string &t, const std::string &n, const IInterface *p)
  : base_class(t, n, p){}

StatusCode SumOfWeightsTool::initialize(){

  const std::unordered_map<std::string, VarType> inVars = {};
  const std::vector<std::string> inVecVars = {};
  const std::unordered_map<std::string, VarType> outVars = {
    {"totalEvents", VarType::Long},
    {"sumOfWeights", VarType::Float}
  };

  setIOVariables(inVars, inVecVars, outVars);

  m_inFile = std::make_unique<TFile>(m_inFileName.value().c_str(), "READ");

  TRegexp re(m_inHistoName.value().c_str(), kTRUE);
  TIter next(m_inFile->GetListOfKeys());
  TKey *key = nullptr;
  while ((key=(TKey*)next())) {
    TString st = key->GetName();
    if (st.Index(re) == kNPOS) continue;
    break;
  }
  if (key==nullptr){return StatusCode::FAILURE;} 
  m_inHisto = dynamic_cast<TH1F*>(m_inFile->Get(key->GetName())); 
  m_sumOfEventWeight = m_inHisto->GetBinContent(2);
  m_sumOfEvents = m_inHisto->GetBinContent(1);

  delete m_inHisto;
  m_inFile->Close();
  
  return StatusCode::SUCCESS;
}

StatusCode SumOfWeightsTool::finalize(){
  return StatusCode::SUCCESS;
}

void SumOfWeightsTool::computeVariables
(const std::unordered_map<std::string, varTypePointer>& /*inVars*/,
 const std::unordered_map<std::string, std::vector<float>*>& /*inVecVars*/,
 std::unordered_map<std::string, varTypePointer>& outVars) const{

  setContent<float>(outVars, "sumOfWeights", m_sumOfEventWeight);
  setContent<unsigned long long>(outVars, "totalEvents", m_sumOfEvents);
  
}
