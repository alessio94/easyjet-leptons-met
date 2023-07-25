/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

  TauDecoratorAlg:
  An alg that copies tau information to aux decorations so can be
  output branch.
*/



// Always protect against multiple includes!
#ifndef EASYJET_TAUDECORATORALG
#define EASYJET_TAUDECORATORALG

#include <vector>
#include <utility>

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <AthContainers/AuxElement.h>
#include <AthLinks/ElementLink.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODTau/TauJetContainer.h>

namespace Easyjet
{

  /// \brief An algorithm for counting containers
  class TauDecoratorAlg final : public AthReentrantAlgorithm
  {
    /// \brief The standard constructor
public:
    TauDecoratorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute(const EventContext& ctx) const override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:

    // Members for configurable properties
    SG::ReadHandleKey<xAOD::TauJetContainer> m_tausInKey{
      this, "tausIn", "", "containerName to read"
    };

    // Internal members

  };
}

#endif
