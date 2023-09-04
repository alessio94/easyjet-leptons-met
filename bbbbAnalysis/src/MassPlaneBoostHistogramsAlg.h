/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

  JetBoostHistogramsAlg:
  Make some histograms!
*/



// Always protect against multiple includes!
#ifndef BBBB_MASSBOOSTHISTOGRAMALG_H
#define BBBB_MASSBOOSTHISTOGRAMALG_H

#include "MassPlaneBoostHistograms.h"

#include "HDF5Utils/IH5GroupSvc.h"

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <xAODJet/JetContainer.h>
#include <xAODEventInfo/EventInfo.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>


namespace HH4B
{

  /// \brief An algorithm for counting containers
  class MassPlaneBoostHistogramsAlg final : public AthReentrantAlgorithm
  {
    /// \brief The standard constructor
public:
    MassPlaneBoostHistogramsAlg(const std::string &name, ISvcLocator *pSvcLocator);
    ~MassPlaneBoostHistogramsAlg();

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

    // properties declared later, I'm not sure how to do this properly
    // in the header
    bhist::MassPlaneHistsConfig m_config;
    // hack because gaudi doesn't support std::set<std::string>, fill
    // this thing and then put it in config above.
    std::vector<std::string> m_mmv;

    using HP = std::unique_ptr<bhist::MassPlaneHists>;
    using JHist = std::pair<CP::SystematicSet,HP>;
    std::vector<JHist> m_jet_histograms;

  };
}

#endif
