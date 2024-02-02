/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GetXSectionTool.h"
#include <filesystem>

GetXSectionTool::GetXSectionTool(const std::string &t, const std::string &n, const IInterface *p)
  : base_class(t, n, p){}

StatusCode GetXSectionTool::initialize(){

  // For simply copying input variables, a tool with empty outVars can be set up
  const std::unordered_map<std::string, VarType> inVars = {
    {"dataTakingYear", VarType::Int},
    {"mcChannelNumber", VarType::Int},
  };
  const std::unordered_map<std::string, VarType> outVars = {
    {"AMIXsection", VarType::Float},
    {"kFactor", VarType::Float},
    {"FilterEff", VarType::Float},
    {"Luminosity", VarType::Float},
  };

  setIOVariables(inVars, outVars);

  ATH_CHECK(m_pmgHandle.retrieve());

  // Check if the input path exists. If the pmg path isn't found, the readInfosFromDir method won't break the script execution.
  for (const std::string& path : m_pathsToPMGFiles.value()) {

    if (!std::filesystem::exists(path)) {
    ATH_MSG_FATAL("\n\n"
                  "****************  WRONG XSECTION FILE PATH  *************************\n"
                  "You've input a wrong date/file to parse under the path :\n"
                  << path << 
                  "*********************************************************************"
                  "\n");
    return StatusCode::FAILURE;
    } 

    m_allPaths += path + "\n";
  }
  
  // Get PMG or Custom File(s) Info From Directory
  m_pmgHandle->readInfosFromFiles(m_pathsToPMGFiles.value());

  return StatusCode::SUCCESS;
}

StatusCode GetXSectionTool::finalize(){
  return StatusCode::SUCCESS;
}

void GetXSectionTool::computeVariables
(const std::unordered_map<std::string, varTypePointer>& inVars,
 std::unordered_map<std::string, varTypePointer>& outVars) const{
  
  unsigned int dataTakingYear = getContent<unsigned int>(inVars, "dataTakingYear");
  unsigned int mcChannelNumber = getContent<unsigned int>(inVars, "mcChannelNumber");
  float luminosity{0.f};

  if (AMIXsection(mcChannelNumber)<0){ // When no DSID (mcChannelNumber) is found, IPMGCrossSectionTool::getAMIXsection returns -1. 
      throw std::runtime_error("\n\n"
                              "      // Script execution should be stopped.\n"
                              "      ****************  WRONG DATASET NUMBER (DSID)  ***********************\n"
                              "      Couldn't find the mcChannelNumber number in the following files :\n      " 
                              + m_allPaths + "\n"
                              "      Please make sure that the DSID-mcChannelNumber value exists in your input files\n"
                              "      *******************************************************************************\n");
  }


  if (luminosities.find(dataTakingYear) == luminosities.end()) {
    ATH_MSG_ERROR("\n\n"
                  "****************  WRONG YEAR INPUT  *************************\n"
                  "No data for year " <<dataTakingYear << " .\n"
                  "Please check the compatibility of the sample's data-taking"
                  "year value and the available luminosities :"
                  );
    for (const auto& lumiYear : luminosities)
      ATH_MSG_INFO("                    " << lumiYear.first);
    throw std::runtime_error("EXITING.\n"
                         "*********************************************************************\n");

  } 
  else //Get luminosity based on input years
      luminosity = luminosities.at(dataTakingYear);


  setContent<float>(outVars, "AMIXsection", AMIXsection(mcChannelNumber)); 
  setContent<float>(outVars, "kFactor", kFactor(mcChannelNumber)); 
  setContent<float>(outVars, "FilterEff", FilterEff(mcChannelNumber)); 
  setContent<float>(outVars, "Luminosity", luminosity);

}
