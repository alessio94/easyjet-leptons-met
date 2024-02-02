/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Post Processing tool for EasyJet ntuple. Retrieves xSection, filterEff, kFactor & luminosity.

#ifndef GetXSectionTool_H
#define GetXSectionTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "EasyjetPlus/IPostProcessTool.h"
#include "AsgTools/AnaToolHandle.h"
#include "PMGAnalysisInterfaces/IPMGCrossSectionTool.h"

using namespace PostProc;

class GetXSectionTool final: public extends<AthAlgTool, IPostProcessTool>
{
 public:
  GetXSectionTool(const std::string&,const std::string&,const IInterface*);
  virtual ~GetXSectionTool() = default;
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  
  virtual void computeVariables
    (const std::unordered_map<std::string, varTypePointer>& inVars,
     std::unordered_map<std::string, varTypePointer>& outVars) const override;

  private:

    //PMG tool
    asg::AnaToolHandle<PMGTools::IPMGCrossSectionTool> m_pmgHandle{"PMGTools::PMGCrossSectionTool/PMGCrossSectionTool"};

    float AMIXsection(int id) const { return m_pmgHandle->getAMIXsection(id); };
    float kFactor(int id) const { return m_pmgHandle->getKfactor(id); }
    float FilterEff(int id) const { return m_pmgHandle->getFilterEff(id); }
    
    Gaudi::Property<std::vector<std::string>> m_pathsToPMGFiles {this, "pathsToPMGFiles", {}, "Path to Custom XSection txt data file"};

    std::string m_allPaths{""};

    std::unordered_map<unsigned int, float> luminosities = {
        {2015, 3.24454+33.4022},
        {2016, 3.24454+33.4022},
        {2017, 44.6306},
        {2018, 58.7916}
    };

};

#endif
