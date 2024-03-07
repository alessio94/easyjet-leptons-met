/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!

#ifndef EASYJET_SUMOFWEIGHTSALG
#define EASYJET_SUMOFWEIGHTSALG

#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/ReadDecorHandleKey.h>

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <xAODEventInfo/EventInfo.h>


namespace Easyjet
{

  /// \brief An algorithm for counting containers
  class SumOfWeightsAlg final : public AthHistogramAlgorithm {

    public:
      SumOfWeightsAlg(const std::string &name, ISvcLocator *pSvcLocator);

      /// \brief Initialisation method, for setting up tools and other persistent
      /// configs
      StatusCode initialize() override;
      /// \brief Execute method, for actions to be taken in the event loop
      StatusCode execute() override;
      /// \brief This is the mirror of initialize() and is called after all events are processed.
      StatusCode finalize() override; ///I added this to write the cutflow histogram.

    private :
      // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
      // "someInfo"};

      SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey
      { this, "event", "EventInfo", "EventInfo to read" };

      long long int m_total_mcEvent{0};
      float m_total_mcEventWeight{0.f};
      float m_total_mcEventWeight_squared{0.f};
      std::vector<float> m_eventWeights{0.f};

      SG::ReadDecorHandleKey<xAOD::EventInfo> m_mcEventWeightsKey{
        this, "mcEventWeights", "EventInfo.mcEventWeights", "mc event weights"};

  };

}

#endif // EASYJET_SUMOFWEIGHTSALG
