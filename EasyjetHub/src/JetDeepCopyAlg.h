/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

  JetDeepCopyAlg:
  Duplicate a jet container. Might be needed to write out a view.
*/



// Always protect against multiple includes!
#ifndef EASYJET_JETDEEPCOPYALG
#define EASYJET_JETDEEPCOPYALG

#include <vector>
#include <utility>

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <xAODJet/JetContainer.h>
#include <xAODJet/JetAuxContainer.h>

namespace Easyjet
{

  /// \brief An algorithm for counting containers
  class JetDeepCopyAlg final : public AthReentrantAlgorithm
  {
    /// \brief The standard constructor
public:
    JetDeepCopyAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute(const EventContext& ctx) const override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:

    using Cont = xAOD::JetContainer;
    using ContAux = xAOD::JetAuxContainer;
    using Item = xAOD::Jet;

    // Members for configurable properties
    SG::ReadHandleKey<Cont> m_jetsInKey{
      this, "jetsIn", "", "containerName to read"
    };
    SG::WriteHandleKey<Cont> m_jetsOutKey{
      this, "jetsOut", "", "container name to write"
    };


  };
}

#endif
