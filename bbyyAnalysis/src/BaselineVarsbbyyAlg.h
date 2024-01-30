/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
#include <xAODMissingET/MissingETContainer.h>



namespace HHBBYY
{

  /// \brief An algorithm for counting containers
  class BaselineVarsbbyyAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
  public:
    BaselineVarsbbyyAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

    float compute_Topness(const xAOD::JetContainer *jets);
    float* compute_EventShapes(const xAOD::JetContainer *jets, const xAOD::PhotonContainer *photons);

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

    Gaudi::Property<std::string> m_photonWPName
      { this, "photonWP", "","Photon ID + Iso working point" };
    CP::SysReadDecorHandle<float> m_ph_SF{"", this};

    CP::SysReadHandle<xAOD::MissingETContainer>
    m_metHandle{ this, "met", "",   "MET container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    std::unordered_map<std::string, CP::SysWriteDecorHandle<float> > m_Fbranches;
    std::vector<std::string> m_Fvarnames{
      // Leading/Subleading photon kinematics
      "Photon1_pt", "Photon1_eta", "Photon1_phi", "Photon1_E",
      "Photon2_pt", "Photon2_eta", "Photon2_phi", "Photon2_E",

      "myy", "pTyy", "dRyy", "Etayy", "Phiyy", 

      // Leading/Subleading b-tagged jet kinematics
      "Jet_b1_pt", "Jet_b1_eta", "Jet_b1_phi", "Jet_b1_E",
      "Jet_b2_pt", "Jet_b2_eta", "Jet_b2_phi", "Jet_b2_E",

      "mbb", "pTbb", "dRbb", "Etabb", "Phibb", 

      // Inclusive jet kinematics
      "Jet1_pt", "Jet1_eta", "Jet1_phi", "Jet1_E",
      "Jet2_pt", "Jet2_eta", "Jet2_phi", "Jet2_E",

      // di-higgs variables
      "mbbyy", "pTbbyy", "dRbbyy", "Etabbyy", "Phibbyy", "mbbyy_star", "Photon1_ptOvermyy", "Photon2_ptOvermyy",

      // mva variables
        "HT", "topness", "sphericityT", "planarFlow", "missEt", "metphi",
    };

    std::vector<std::string> m_Fvarnames_MC{
      "Photon1_effSF", "Photon2_effSF",
    };

    std::unordered_map<std::string, CP::SysWriteDecorHandle<int> > m_Ibranches;
    std::vector<std::string> m_Ivarnames{
      "nPhotons", "nBJets", "nJets", "nCentralJets",
      "Jet_b1_truthLabel", "Jet_b2_truthLabel",
      "Jet1_truthLabel", "Jet2_truthLabel",
      "Jet1_PassWP", "Jet2_PassWP",
    };

  };
}
#endif
