/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

  JetBoostHistogramsAlg:
  Make some histograms!
*/



// Always protect against multiple includes!
#ifndef BBBB_JETBOOSTHISTOGRAMALG_H
#define BBBB_JETBOOSTHISTOGRAMALG_H

#include "HDF5Utils/IH5GroupSvc.h"

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <xAODJet/JetContainer.h>
#include <xAODEventInfo/EventInfo.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>

namespace bhist {
  class JetHists;
}

namespace HH4B
{

  /// \brief An algorithm for counting containers
  class JetBoostHistogramsAlg final : public AthReentrantAlgorithm
  {
    /// \brief The standard constructor
public:
    JetBoostHistogramsAlg(const std::string &name, ISvcLocator *pSvcLocator);
    ~JetBoostHistogramsAlg();

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute(const EventContext& ctx) const override;

    StatusCode finalize() override;

private:

    using Cont = xAOD::JetContainer;
    using Item = Cont::base_value_type;

    CP::SysListHandle m_systematicsList {this};

    // Members for configurable properties
    CP::SysReadHandle<xAOD::JetContainer>
      m_jetsIn{ this, "jetsIn", "", "Jet container to read" };
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_eventInfoKey {
      this, "eventWeight", "EventInfo.generatorWeight_NOSYS",
      "event weight key"
    };
    ServiceHandle<IH5GroupSvc> m_output_svc {
      this, "output", "", "output file service"
    };

    using JHist = std::pair<CP::SystematicSet,std::unique_ptr<bhist::JetHists>>;
    std::vector<JHist> m_jet_histograms;

  };
}

#endif
