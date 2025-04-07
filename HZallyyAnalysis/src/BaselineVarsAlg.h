/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HZALLYYANALYSIS_BASELINEVARSALG
#define HZALLYYANALYSIS_BASELINEVARSALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <xAODEventInfo/EventInfo.h>
namespace HZALLYY
{
  
  /// \brief An algorithm for counting containers
  class BaselineVarsAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
  public:
    BaselineVarsAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any
    
  private:
    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};
             
    CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };
    
    Gaudi::Property<std::vector<std::string>> m_floatVariables
      {this, "floatVariableList", {}, "Name list of floating variables"};
    
    Gaudi::Property<std::vector<std::string>> m_intVariables
      {this, "intVariableList", {}, "Name list of integer variables"};
    
    std::unordered_map<std::string, CP::SysReadDecorHandle<float>> m_FDecors;
    std::unordered_map<std::string, CP::SysReadDecorHandle<int>> m_IDecors;
    
    std::vector<std::string> m_fvars;
    std::vector<std::string> m_ivars
      {
	"nLeptons",
	"nPhotons"
      };
    
    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_Fbranches;
    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;
    
  };
}
#endif
