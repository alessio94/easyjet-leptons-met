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
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>

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

    CP::SysListHandle m_systematicsList {this};

    SG::ReadHandleKey<ConstDataVector<xAOD::JetContainer> >
    m_smallRContainerInKey{ this, "smallRContainerInKey",
                            "",   "containerName to read" };


    SG::ReadHandleKey<ConstDataVector<xAOD::JetContainer> >
    m_smallRContainerInKey_No_WP{ this, "smallRContainerInKey_No_WP",
                            "",   "Jet container without WP to read" };

    SG::ReadHandleKey<ConstDataVector<xAOD::PhotonContainer> >
    m_photonContainerInKey{ this, "photonContainerInKey",
                            "",   "containerName to read" };

    SG::ReadHandleKey<ConstDataVector<xAOD::MuonContainer> >
    m_muonContainerInKey{ this, "muonContainerInKey",
                            "",   "containerName to read" };

    SG::ReadHandleKey<ConstDataVector<xAOD::ElectronContainer> >
    m_electronContainerInKey{ this, "electronContainerInKey",
                            "",   "containerName to read" };

    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{
      this, "EventInfoKey", "EventInfo", "EventInfo container to dump"
    };

    std::unordered_map<std::string, SG::AuxElement::Decorator<float> > m_decos;
    std::vector<std::string> m_vars{
      // Cut flow
      "TWO_LOOSE_PHOTONS",
      "N_LOOSE_PHOTONS",
      "TWO_TIGHTID_PHOTONS",
      "TWO_ISO_PHOTONS",
      "PASS_RELPT_CUT",
      "MASSCUT",
      "myy",
      //Nlep=0 cut
      "N_LEPTONS_CUT",
      //Jets cut
      "LESS_THAN_SIX_CENTRAL_JETS",
      "EXACTLY_TWO_B_JETS",
      //Passed event through all cuts.
      "isPassed",

      // Leading/Subleading photon kinematics
      "Leading_Photon_pt", "Leading_Photon_eta", "Leading_Photon_phi", "Leading_Photon_E",
      "Subleading_Photon_pt", "Subleading_Photon_eta", "Subleading_Photon_phi", "Subleading_Photon_E",

      // Leading/Subleading jet kinematics
      "Leading_Jet_pt", "Leading_Jet_eta", "Leading_Jet_phi", "Leading_Jet_E",     
      "Subleading_Jet_pt", "Subleading_Jet_eta", "Subleading_Jet_phi", "Subleading_Jet_E",

    };

  };
}

#endif
