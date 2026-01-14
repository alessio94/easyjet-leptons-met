/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HHBBYYANALYSIS_PHOTONVARSYYBBALG
#define HHBBYYANALYSIS_PHOTONVARSYYBBALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODEgamma/PhotonContainer.h>

namespace HHBBYY
{
  /// \brief An algorithm for counting containers
  class PhotonVarsbbyyAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
  public:
    PhotonVarsbbyyAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;

  private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::PhotonContainer>
      m_bbyyPhotonHandle{ this, "bbyyPhotons", "bbyyAnalysisPhotons_%SYS%", "Photons container to read" };

    CP::SysReadHandle<xAOD::PhotonContainer>
      m_photonHandle{ this, "photons", "AnalysisPhotons_%SYS%", "Original photon container to read" };

    CP::SysReadDecorHandle<unsigned int> m_isEMTight
      {"DFCommonPhotonsIsEMTightIsEMValue", this};

    Gaudi::Property<std::string> m_photonWPName
      { this, "photonWP", "", "Photon ID + Iso working point" };
    
    CP::SysReadDecorHandle<float> m_ph_SF{"", this};
    
    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    Gaudi::Property<bool> m_doSystematics
      { this, "doSystematics", false, "Run on all systematics" };

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    Gaudi::Property<std::vector<std::string>> m_floatVariables
      {this, "floatVariableList", {}, "Name list of float variables"};

    Gaudi::Property<std::vector<std::string>> m_intVariables
        {this, "intVariableList", {}, "Name list of integer variables"};
    
    Gaudi::Property<bool> m_do_HHbbyy_Hyy_Analysis
      { this, "do_HHbbyy_Hyy_Analysis", false, "Save additional input variables for HH_H analysis"};

    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_Fbranches;
    
    CP::SysReadDecorHandle<bool> 
    m_selected_ph { this, "selected_ph", "selected_ph_%SYS%", "Name of input decorator for selected ph"};
  };
}
#endif
