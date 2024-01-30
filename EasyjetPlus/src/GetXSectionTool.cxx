/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GetXSectionTool.h"
#include <filesystem>

GetXSectionTool::GetXSectionTool(const std::string &t, const std::string &n, const IInterface *p)
  : base_class(t, n, p){}

StatusCode GetXSectionTool::initialize(){

  // For simply copying input variables, a tool with empty outVars can be set up
  const std::unordered_map<std::string, VarType> inVars = {};
  const std::unordered_map<std::string, VarType> outVars = {
    {"AMIXsection", VarType::Float},
    {"kFactor", VarType::Float},
    {"FilterEff", VarType::Float},
    {"Luminosity", VarType::Float},
  };

  setIOVariables(inVars, outVars);

  ATH_CHECK(m_pmgHandle.retrieve());

  // Check if the input path exists. If the pmg path isn't found, the readInfosFromDir method won't break the script execution.
    if (!std::filesystem::exists(m_pathToPMGFile.value())) {
    ATH_MSG_FATAL("\n\n"
                  "****************  WRONG XSECTION FILE PATH  *************************\n"
                  "You've input a wrong date/file to parse in PMG under the path :\n"
                  << m_pathToPMGFile << 
                  "*********************************************************************"
                  "\n");
    return StatusCode::FAILURE;
    } 
  
  // Get PMG or Custom File(s) Info From Directory
  m_pmgHandle->readInfosFromDir(m_pathToPMGFile.value().c_str());

  if (AMIXsection(m_DSID)<0){ // When no DSID is found, IPMGCrossSectionTool::getAMIXsection returns -1. 
    ATH_MSG_FATAL("\n\n"      // Script execution should be stopped.
                  "****************  WRONG DATASET NUMBER (DSID)  *************************\n"
                  "Couldn't find the input dataset number to in the file :\n"
                  << m_pathToPMGFile << "\n"
                  "*********************************************************************"
                  "\n");    
        return StatusCode::FAILURE;
  }

  // Calculate Total luminosity based on input years
  for (const std::string &year : m_mcYears.value()) {

    if (luminosities.find(year) == luminosities.end()) {
        ATH_MSG_ERROR("\n\n"
                      "****************  WRONG YEAR INPUT  *************************\n"
                      "No data for year " <<year << " .\n"
                      "Please input at least one of the following years :"
                     );
        for (const auto& lumiYear : luminosities)
          ATH_MSG_INFO("                    " << lumiYear.first);
        ATH_MSG_FATAL("EXITING.\n"
                      "*********************************************************************"
                      "\n");
    } 
    
    // If we have a correct year input, calculate total luminosity. 
    else
         m_TotalLuminosity += luminosities[year];
   
  }

  return StatusCode::SUCCESS;
}

StatusCode GetXSectionTool::finalize(){
  return StatusCode::SUCCESS;
}

void GetXSectionTool::computeVariables
(const std::unordered_map<std::string, varTypePointer>& /*inVars*/,
 std::unordered_map<std::string, varTypePointer>& outVars) const{
  
  setContent<float>(outVars, "AMIXsection", AMIXsection(m_DSID)); 
  setContent<float>(outVars, "kFactor", kFactor(m_DSID)); 
  setContent<float>(outVars, "FilterEff", FilterEff(m_DSID)); 
  setContent<float>(outVars, "Luminosity", m_TotalLuminosity);

}
