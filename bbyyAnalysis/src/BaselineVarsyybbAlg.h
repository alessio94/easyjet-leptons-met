/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HHBBYYANALYSIS_FINALVARSYYBBALG
#define HHBBYYANALYSIS_FINALVARSYYBBALG

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <AthContainers/ConstDataVector.h>

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <FourMomUtils/xAODP4Helpers.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/PhotonContainer.h>

namespace HHBBYY
{

  /// \brief An algorithm for counting containers
  class BaselineVarsyybbAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
  public:
    BaselineVarsyybbAlg(const std::string &name, ISvcLocator *pSvcLocator);

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
    m_bjetHandle{ this, "bjets", "",   "BJet container to read" };

    CP::SysReadHandle<xAOD::JetContainer>
    m_jetHandle{ this, "jets", "",   "Jet container to read" };

    CP::SysReadHandle<xAOD::PhotonContainer>
    m_photonHandle{ this, "photons", "",   "Photons container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

    bool m_isMC;

    std::unordered_map<std::string, CP::SysWriteDecorHandle<float> > m_Fbranches;
    std::vector<std::string> m_Fvarnames{
      // Leading/Subleading photon kinematics
      "Leading_Photon_pt", "Leading_Photon_eta", "Leading_Photon_phi", "Leading_Photon_E",
      "Subleading_Photon_pt", "Subleading_Photon_eta", "Subleading_Photon_phi", "Subleading_Photon_E",

      "myy", "pTyy", "dRyy", "Etayy", "Phiyy", 

      // Leading/Subleading b-tagged jet kinematics
      "Jet_pt_b1", "Jet_eta_b1", "Jet_phi_b1", "Jet_E_b1", 
      "Jet_pt_b2", "Jet_eta_b2", "Jet_phi_b2", "Jet_E_b2", 

      "mbb", "pTbb", "dRbb", "Etabb", "Phibb", 

      // di-higgs variables
      "mbbyy", "pTbbyy", "dRbbyy", "Etabbyy", "Phibbyy", "mbbyy_star",

      // mva variables
      "HT",
    };

    std::unordered_map<std::string, CP::SysWriteDecorHandle<int> > m_Ibranches;
    std::vector<std::string> m_Ivarnames{
      "nPhotons", "nBJets", "nJets",
      "Jet_truthLabel_b1", "Jet_truthLabel_b2",
    };

  };
}
#endif
