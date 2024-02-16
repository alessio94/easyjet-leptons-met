/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

  BTaggingDecoratorAlg:
  A convenience alg that copies b-tagging information from the BTagging
  object to its associated jet, to simplify output branch handling with
  selections on the jets.
*/



// Always protect against multiple includes!
#ifndef EASYJET_BTAGGINGDECORATORALG
#define EASYJET_BTAGGINGDECORATORALG

#include <vector>
#include <utility>

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <AthContainers/AuxElement.h>
#include <AthLinks/ElementLink.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODBTagging/BTaggingContainer.h>

namespace Easyjet
{

  /// \brief An algorithm for counting containers
  class BTaggingDecoratorAlg final : public AthReentrantAlgorithm
  {
    /// \brief The standard constructor
public:
    BTaggingDecoratorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute(const EventContext& ctx) const override;
    /// We use default finalize() -- this is for cleanup, and we don't do any
    float evaluate_Db(const xAOD::Jet& jet) const;
    int evaluate_pcbtExp(float Db) const;

private:

    // Members for configurable properties
    SG::ReadHandleKey<xAOD::JetContainer> m_jetsInKey{
      this, "jetsIn", "", "containerName to read"
    };

    Gaudi::Property<std::string> m_btagLinkName{
      this, "btagLinkName", "btaggingLink", "Name of the b-tag link decoration"
    };

    Gaudi::Property<std::vector<std::string> > m_floatVars{
      this, "floatVars", {}, "Float variables to be decorated onto jets"
    };

    Gaudi::Property<bool > m_doExp{
      this, "expVars", false, "Flag to add experimental decorations to jets"
    };

    // Internal members
    // Hold accessor as data member to avoid multiple SG lookups
    SG::AuxElement::ConstAccessor<ElementLink<xAOD::BTaggingContainer> > m_btagLinkAcc;
    // Hold Accessor -> Decorator pairs for the copy operation
    std::vector< std::pair<
        SG::AuxElement::ConstAccessor<float>,
        SG::AuxElement::Decorator<float>
    > > m_floatHandlers;
  };
}

#endif
