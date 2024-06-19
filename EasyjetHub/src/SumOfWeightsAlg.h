/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!

#ifndef EASYJET_SUMOFWEIGHTSALG
#define EASYJET_SUMOFWEIGHTSALG

#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/ReadDecorHandleKey.h>

#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>

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
      double m_total_mcEventWeight{0.0};
      double m_total_mcEventWeight_squared{0.0};

      SG::ReadDecorHandleKey<xAOD::EventInfo> m_mcEventWeightsKey{
        this, "mcEventWeights", "EventInfo.mcEventWeights", "mc event weights"};

      // for systematics 
      std::unordered_map<CP::SystematicSet,TH1*> m_hist;
      
      Gaudi::Property<std::string> m_histPattern 
      {this, "histPattern", "SumOfWeights_%SYS%", "the pattern for histogram names"};
      
      Gaudi::Property<std::string> m_histTitle 
      {this, "histTitle", "sum of weights for sys_%SYS%", "title for the created histograms"};
      
      CP::SysReadDecorHandle<float>
      m_generatorWeight{ this, "generatorWeight", "generatorWeight_%SYS%", "MC event weights" };

      CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle{ this, "event_for_sys", "EventInfo",   "EventInfo container to read" };

      CP::SysListHandle m_systematicsList {this};


    Gaudi::Property<int> m_weightIndex
      { this, "weightIndex", -1, "Special weight Index based on MCChannelNumber"};        

  };

}

#endif // EASYJET_SUMOFWEIGHTSALG
