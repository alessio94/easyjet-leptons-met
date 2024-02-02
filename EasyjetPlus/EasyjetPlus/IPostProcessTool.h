/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Interface for PostProcessorTool for EasyJet ntuple
// This should be used by all tools, even the analysis specific ones

#ifndef IPostProcessTool_H
#define IPostProcessTool_H

#include "GaudiKernel/IAlgTool.h"

#include "EasyjetPlus/Utilities.h"

namespace PostProc{
  enum class VarList;
}

using namespace PostProc;

class IPostProcessTool : virtual public IAlgTool
{
  
public:  
  DeclareInterfaceID(IPostProcessTool, 1, 0);

  virtual void computeVariables
  (const std::unordered_map<std::string, varTypePointer>& inVars,
   const std::unordered_map<std::string, std::vector<float>*>& inVecVars,
   std::unordered_map<std::string, varTypePointer>& outVars) const = 0;

  const std::unordered_map<std::string, VarType> inputVariables() const { return m_inVars; }
  const std::vector<std::string> inputVecVariables() const { return m_inVecVars; }
  const std::unordered_map<std::string, VarType> outputVariables() const { return m_outVars; }

  void setIOVariables(const std::unordered_map<std::string, VarType>& inVars,
		      const std::vector<std::string>& inVecVars,
		      const std::unordered_map<std::string, VarType>& outVars)
  { m_inVars=inVars; m_inVecVars=inVecVars; m_outVars=outVars; }

protected:
  template <typename T>
  T getContent(const std::unordered_map<std::string, varTypePointer> vars, std::string name) const{
    return *(reinterpret_cast<T*>(vars.at(name).pointer));
  }

  template <typename T>
  void setContent(const std::unordered_map<std::string, varTypePointer> vars,
		  std::string name, const T value) const{
    *reinterpret_cast<T*>(vars.at(name).pointer) = *(&value);
  }

private:

  std::unordered_map<std::string, VarType> m_inVars;
  std::vector<std::string> m_inVecVars;
  std::unordered_map<std::string, VarType> m_outVars;

};

#endif
