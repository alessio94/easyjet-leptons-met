/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef SHBBYYANALYSIS_FINALVARSYYBBALG
#define SHBBYYANALYSIS_FINALVARSYYBBALG

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/PhotonContainer.h>
#include "lwtnn/LightweightGraph.hh"
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>


namespace SHBBYY
{

  /// \brief An algorithm for counting containers
  class ResonantPNNbbyyAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
  public:
    ResonantPNNbbyyAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

  private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::JetContainer>
    m_jetHandle{ this, "jets", "",   "Jet container to read" };

    CP::SysReadDecorHandle<char> 
    m_isBtag {this, "bTagWPDecorName", "", "Name of input dectorator for b-tagging"};

    CP::SysReadHandle<xAOD::PhotonContainer>
    m_photonHandle{ this, "photons", "",   "Photons container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

    std::unordered_map<std::string, CP::SysWriteDecorHandle<float> > m_Fbranches;
    const std::string m_PNN_ScoreLabel = "SH_PNN_Score" ;
    const std::string m_PNN_1bjet_ScoreLabel = "SH_PNN_Score_1bjet";
    std::vector<std::string> m_Fvarnames{};
    std::unique_ptr<lwt::LightweightGraph> m_model_PNN_cv1;
    std::unique_ptr<lwt::LightweightGraph> m_model_PNN_cv2;
    std::unique_ptr<lwt::LightweightGraph> m_model_PNN_cv3;
    std::unique_ptr<lwt::LightweightGraph> m_model_PNN_1bjet_cv1;
    std::unique_ptr<lwt::LightweightGraph> m_model_PNN_1bjet_cv2;
    std::unique_ptr<lwt::LightweightGraph> m_model_PNN_1bjet_cv3;
    std::unordered_map<std::string, CP::SysWriteDecorHandle<int> > m_Ibranches;
    std::vector<std::pair<double, double>>m_mX_mS_all_pairs{};
    Gaudi::Property <std::vector<std::pair<double, double>>> m_mX_mS_pairs {this, "mX_mS_pairs", {}, "Low Mass mX and mS pairs"};
    Gaudi::Property <std::vector<double>> m_mX_values {this, "mX_values", {}, "High Mass mX values"};
    Gaudi::Property <std::vector<double>> m_mS_values {this, "mS_values", {}, "High Mass mS values"};
    Gaudi::Property <std::vector<double>> m_mX_1bjet {this, "mX_1bjet", {}, "mX values for 1-bjet"};      
  };
}
#endif
