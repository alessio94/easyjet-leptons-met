/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HH4BANALYSIS_FINALVARSYYBBALG
#define HH4BANALYSIS_FINALVARSYYBBALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <FourMomUtils/xAODP4Helpers.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/PhotonContainer.h>

namespace HH4B
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
    m_smallRContainerInKey{ this, "smallRContainerInKey",
                            "",   "containerName to read" };

    SG::ReadHandleKey<ConstDataVector<xAOD::PhotonContainer> >
    m_photonContainerInKey{ this, "photonContainerInKey",
                            "",   "containerName to read" };

    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{
      this, "EventInfoKey", "EventInfo", "EventInfo container to dump"
    };

    std::string m_bTagWP;
    std::unordered_map<std::string, SG::AuxElement::Decorator<float> > m_decos;
    std::vector<std::string> m_vars{
      "DeltaR12_",              "Leading_Photon_pt_",
      "Leading_Photon_eta_",    "Leading_Photon_phi_",
      "Leading_Photon_E_",      "Subleading_Photon_pt_",
      "Subleading_Photon_eta_", "Subleading_Photon_phi_",
      "Subleading_Photon_E_",   "myy_",
    };
  };
}

#endif
