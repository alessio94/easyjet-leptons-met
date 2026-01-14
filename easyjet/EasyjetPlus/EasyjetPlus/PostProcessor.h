/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Base class of PostProcessor for EasyJet ntuple

#ifndef EASYJETPLUS_POSTPROCESSOR
#define EASYJETPLUS_POSTPROCESSOR

#include "AthenaBaseComps/AthAlgorithm.h"

#include "EasyjetPlus/IPostProcessTool.h"
#include "EasyjetPlus/Utilities.h"

#include "TChain.h"
#include "TTree.h"
#include "TFile.h"

namespace PostProc{
  enum class VarList;
}

using namespace PostProc;

class PostProcessor : public AthAlgorithm
{
  
 public:
  PostProcessor(const std::string &name, ISvcLocator *pSvcLocator);

  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

 private:

  StatusCode addIOVars(const std::unordered_map<std::string, VarType>& inVars,
		       const std::vector<std::string>& inVecVars,
		       const std::unordered_map<std::string, VarType>& outVars);

  void setBranchIn();
  void setBranchOut(const std::unordered_map<std::string, varTypePointer>& outVar);

  Gaudi::Property<std::string> m_inFileName {this, "inFile", "", "EasyJet input file"};
  Gaudi::Property<std::string> m_inTreeName {this, "inTree", "AnalysisMiniTree", "Input tree"};
  Gaudi::Property<int> m_maxEvents {this, "maxEvents", -1, "Number of events to process"};

  Gaudi::Property<std::string> m_outFileName {this, "outFile", "", "Output file"};
  Gaudi::Property<std::string> m_outTreeName {this, "outTree", "AnalysisMiniTree", "Output tree"};

  Gaudi::Property<bool> m_copyInputs {this, "copyInputs", true, "Copy input variables of tools"};

  ToolHandleArray<IPostProcessTool> m_tools {this, "postProcessTools", {}, "List of postprocess tools"};

  std::unique_ptr<TChain> m_inTree;
  std::unique_ptr<TTree> m_outTree;
  std::unique_ptr<TFile> m_outFile;

  std::unordered_map<std::string, varTypePointer> m_inVars;
  std::unordered_map<std::string, std::vector<float>*> m_inVecVars;
  std::unordered_map<std::string, varTypePointer> m_outVars;
  
};

#endif
