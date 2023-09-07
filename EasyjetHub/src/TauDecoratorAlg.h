/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

  TauDecoratorAlg:
  An alg that copies tau information to aux decorations so can be
  output branch.
*/



// Always protect against multiple includes!
#ifndef EASYJET_TAUDECORATORALG
#define EASYJET_TAUDECORATORALG

#include <vector>
#include <utility>

#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <AthContainers/AuxElement.h>
#include <AthLinks/ElementLink.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODTau/TauJetContainer.h>

namespace bbtautau 
{
  enum Channel
  {
    LepHad = 0,
    HadHad = 1,
  };
}

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

    Gaudi::Property<std::string> m_nProngDecorName{
      this, "nProngDecorKey", "nProng", "Decoration for nProng"
    };
    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_nProngDecorKey;

    Gaudi::Property<std::string> m_IDTauDecorName{
      this, "idTauDecorKey", "isIDTau", "Decoration for ID taus"
    };
    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_IDTauDecorKey;

    Gaudi::Property<std::string> m_antiTauDecorName{
      this, "antiTauDecorKey", "isAntiTau", "Decoration for anti-taus"
    };
    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_antiTauDecorKey;


    std::string m_tauIDWP_name;
    xAOD::TauJetParameters::IsTauFlag m_tauIDWP;
    std::string m_channel_name;
    bbtautau::Channel m_channel;

    // Internal members

  };
}

#endif
