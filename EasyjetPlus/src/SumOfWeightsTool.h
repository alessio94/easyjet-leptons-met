/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// PostProcessorTool to compute sum of MC weights

#ifndef SumOfWeightsTool_H
#define SumOfWeightsTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "EasyjetPlus/IPostProcessTool.h"

#include "TFile.h"
#include "TH1F.h"

using namespace PostProc;

class SumOfWeightsTool final: public extends<AthAlgTool, IPostProcessTool>
{
 public:
  SumOfWeightsTool(const std::string&,const std::string&,const IInterface*);
  virtual ~SumOfWeightsTool() = default;
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  virtual void computeVariables
    (const std::unordered_map<std::string, varTypePointer>& inVars,
     std::unordered_map<std::string, varTypePointer>& outVars) const override;

 private:
     Gaudi::Property<std::string> m_inFileName {this, "inFile", "", "EasyJet input file"};
     Gaudi::Property<std::string> m_inHistoName {this, "inHisto", "CutBookkeeper_*_NOSYS", "CutBookkeeper histo"};
     std::unique_ptr<TFile> m_inFile;
     TH1F* m_inHisto; 
     float m_sumOfEventWeight = -1.;
     
};

#endif
