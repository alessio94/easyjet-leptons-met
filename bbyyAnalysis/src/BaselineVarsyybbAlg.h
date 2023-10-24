/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HHBBYYANALYSIS_FINALVARSYYBBALG
#define HHBBYYANALYSIS_FINALVARSYYBBALG

#include <AthContainers/ConstDataVector.h>
#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <FourMomUtils/xAODP4Helpers.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
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

    SG::ReadHandleKey<ConstDataVector<xAOD::JetContainer> >
    m_smallRJets_BTag_ContainerInKey{ this, "smallRJets_BTag_ContainerInKey",
                            "",   "containerName to read" };


    SG::ReadHandleKey<ConstDataVector<xAOD::JetContainer> >
    m_smallRJets_ContainerInKey{ this, "smallRJets_ContainerInKey",
                            "",   "Jet container without WP to read" };

    SG::ReadHandleKey<ConstDataVector<xAOD::PhotonContainer> >
    m_photonContainerInKey{ this, "photonContainerInKey",
                            "",   "containerName to read" };

    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{
      this, "EventInfoKey", "EventInfo", "EventInfo container to dump"
    };

    bool m_isMC;
    std::unordered_map<std::string, SG::AuxElement::Decorator<float> > m_decos;
    std::vector<std::string> m_vars{

      // Leading/Subleading photon kinematics
      "Leading_Photon_pt", "Leading_Photon_eta", "Leading_Photon_phi", "Leading_Photon_E",
      "Subleading_Photon_pt", "Subleading_Photon_eta", "Subleading_Photon_phi", "Subleading_Photon_E",

      "myy", "pTyy", "dRyy", "Etayy", "Phiyy",

      // Leading/Subleading b-tagged jet kinematics
      "Jet_pt_B1", "Jet_eta_B1", "Jet_phi_B1", "Jet_E_B1",
      "Jet_pt_B2", "Jet_eta_B2", "Jet_phi_B2", "Jet_E_B2", 
      "mBB", "pTBB", "dRBB", "EtaBB", "PhiBB",

      // di-higgs variables
      "mBByy", "pTBByy", "dRBByy", "EtaBByy", "PhiBByy", "mBByy_star",

      // mva variables
      "Ht",

      // Flavour of Truth B-tagged Leading/Subleading jets.   
      "Jet_HadronConeExclTruthLabelID_B1","Jet_HadronConeExclTruthLabelID_B2"

    };

  };
}

#endif
