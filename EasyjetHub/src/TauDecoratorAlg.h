/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

  TauDecoratorAlg:
  An alg that copies tau information to aux decorations so can be
  output branch.
*/



// Always protect against multiple includes!
#ifndef EASYJET_TAUDECORATORALG
#define EASYJET_TAUDECORATORALG

#include <vector>
#include <utility>
#include <string>
#include <unordered_map>

#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AthenaBaseComps/AthReentrantAlgorithm.h>

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
    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    // Taus
    SG::ReadHandleKey<xAOD::TauJetContainer> m_tausInKey{
      this, "tausIn", "", "containerName to read"
    };

    Gaudi::Property<std::string> m_nProngDecorName{
      this, "nProngDecorKey", "nProng", "Decoration for nProng"
    };
    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_nProngDecorKey;

    Gaudi::Property<std::string> m_decayModeDecorName{
      this, "decayMode", "decayMode", "Decoration for decayMode"
    };
    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_decayModeDecorKey;

    Gaudi::Property<std::string> m_truthTypeDecorName{
      this, "truthTypeDecorKey", "truthType", "Decoration for truthType"
    };
    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_truthTypeDecorKey;

  };

}

#endif
