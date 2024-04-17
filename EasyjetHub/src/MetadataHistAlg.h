/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!

#ifndef EASYJET_METADATAHISTALG
#define EASYJET_METADATAHISTALG

#include "AthenaBaseComps/AthHistogramAlgorithm.h"

namespace Easyjet {

  class MetadataHistAlg : public AthHistogramAlgorithm {

    public:
      MetadataHistAlg(const std::string& name, ISvcLocator* pSvcLocator);

      /// \brief Initialisation method, for setting up tools and other persistent
      /// configs
      StatusCode initialize() override; 
      /// \brief Execute method, for actions to be taken in the event loop
      StatusCode execute() override;
      /// \brief This is the mirror of initialize() and is called after all events are processed.
      StatusCode finalize() override; ///I added this to write the cutflow histogram.

    private:
      Gaudi::Property<std::string> m_dataType {
        this, "dataType", "", "sample dataType"
      };
      Gaudi::Property<std::string> m_mcCampaign {
        this, "mcCampaign", "", "mc campaign"
      };
      Gaudi::Property<std::string> m_mcChannelNumber {
        this, "mcChannelNumber", "", "mc channel number"
      };
  };
} 

#endif // EASYJET_METADATAHISTALG 
