/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HH4BANALYSIS_FINALVARSBBTTALG
#define HH4BANALYSIS_FINALVARSBBTTALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <FourMomUtils/xAODP4Helpers.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>

namespace HH4B
{

  /// \brief An algorithm for counting containers
  class BaselineVarsbbttAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    BaselineVarsbbttAlg(const std::string &name, ISvcLocator *pSvcLocator);

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

    SG::ReadHandleKey<ConstDataVector<xAOD::MuonContainer> >
    m_muonContainerInKey{ this, "muonContainerInKey",
                            "",   "containerName to read" };

    SG::ReadHandleKey<ConstDataVector<xAOD::ElectronContainer> >
    m_electronContainerInKey{ this, "electronContainerInKey",
                            "",   "containerName to read" };

    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{
      this, "EventInfoKey", "EventInfo", "EventInfo container to dump"
    };

    std::string m_bTagWP;
    std::unordered_map<std::string, SG::AuxElement::Decorator<float> > m_decos;
    std::vector<std::string> m_vars{
      "Leading_Muon_pt_",     "Leading_Muon_eta_",    
      "Leading_Electron_pt_", "Leading_Electron_eta_",    
    };
  };
}

#endif
