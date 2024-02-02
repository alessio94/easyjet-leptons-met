/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "EasyjetPlus/PostProcessor.h"

PostProcessor::PostProcessor(const std::string &name,
			     ISvcLocator *pSvcLocator)
  : AthAlgorithm(name, pSvcLocator)
{}

StatusCode PostProcessor::initialize(){

  if(!m_tools.empty()){
    ATH_CHECK(m_tools.retrieve());
  }

  for(const auto& tool : m_tools){
    ATH_CHECK(addIOVars(tool->inputVariables(), tool->inputVecVariables(),
			tool->outputVariables()));
  }
  
  m_inTree = std::make_unique<TChain>(m_inTreeName.value().c_str());
  m_inTree->Add(m_inFileName.value().c_str());

  // Set address of input branches
  setBranchIn();
  
  m_outTree = std::make_unique<TTree>(m_outTreeName.value().c_str(),
				      m_outTreeName.value().c_str());

  // Set address of output branches
  setBranchOut(m_inVars);
  setBranchOut(m_outVars);

  return StatusCode::SUCCESS;
}

StatusCode PostProcessor::execute(){

  Long64_t nentries = m_inTree->GetEntries();
  if(m_maxEvents>0) nentries = std::min(nentries, Long64_t(m_maxEvents));

  ATH_MSG_INFO(nentries<<" events to process");

  for (int i=0;i<nentries;i++){
    if(i%10000==0)
      ATH_MSG_INFO("Processing event #"<<i);

    m_inTree->GetEntry(i);
    
    // Computing variables
    for(const auto& tool : m_tools){
      tool->computeVariables(m_inVars, m_inVecVars, m_outVars);
    }
			    
    // Filling output tree
    m_outTree->Fill();
  }  

  ATH_MSG_INFO("Successfully processed "<<nentries<<" events");
  return StatusCode::SUCCESS;
}

StatusCode PostProcessor::finalize(){
  m_outFile = std::unique_ptr<TFile>(TFile::Open(m_outFileName.value().c_str(),"RECREATE"));
  m_outTree->Write();
  m_outFile->Close();
  return StatusCode::SUCCESS;
}

StatusCode PostProcessor::addIOVars
(const std::unordered_map<std::string, VarType>& inVars,
 const std::vector<std::string>& inVecVars,
 const std::unordered_map<std::string, VarType>& outVars){

  for(const auto& [name, type]: inVars){
    switch(type) {
    case VarType::Int:
      m_inVars[name] = varTypePointer{type, new unsigned int};
      break;
    case VarType::Long:
      m_inVars[name] = varTypePointer{type, new unsigned long long};
      break;
    case VarType::Float:
      m_inVars[name] = varTypePointer{type, new float};
      break;
    }
  }

  for(const auto& name: inVecVars){
    m_inVecVars[name] = new std::vector<float>;
  }

  for(const auto& [name, type]: outVars){
    switch(type) {
    case VarType::Int:
      m_outVars[name] = varTypePointer{type, new unsigned int};
      break;
    case VarType::Long:
      m_outVars[name] = varTypePointer{type, new unsigned long long};
      break;
    case VarType::Float:
      m_outVars[name] = varTypePointer{type, new float};
      break;
    default:
      ATH_MSG_FATAL("Only flat types are allowed to be written out in postprocessing");
      return StatusCode::FAILURE;
    }
  }

  return StatusCode::SUCCESS;
}

void PostProcessor::setBranchIn(){

  for(const auto& [name, type_pointer]: m_inVars){
    m_inTree->SetBranchAddress(name.c_str(), type_pointer.pointer);
  }

  for(const auto& [name, pointer]: m_inVecVars){
    // You would think so and yet...
    //m_inTree->SetBranchAddress(name.c_str(), pointer);
    m_inTree->SetBranchAddress(name.c_str(), &(m_inVecVars.at(name)));
  }

}


void PostProcessor::setBranchOut
(const std::unordered_map<std::string, varTypePointer>& outVar){

  for(const auto& [name, type_pointer]: outVar){

    std::string suffix = "";
    switch(type_pointer.type) {
    case VarType::Int:
      suffix = "/i";
      break;
    case VarType::Long:
      suffix = "/l";
      break;
    case VarType::Float:
      suffix = "/F";
      break;
    default:
      continue;
    }

    m_outTree->Branch(name.c_str(), type_pointer.pointer, (name+suffix).c_str());
  }

}
