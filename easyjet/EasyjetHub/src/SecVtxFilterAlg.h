/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

  SecVtxFilterAlg:
  An alg that applied a filter using the secondary vertex information.
*/

// Always protect against multiple includes!
#ifndef EASYJET_SECVTXFILTERALG
#define EASYJET_SECVTXFILTERALG

#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <EventBookkeeperTools/FilterReporterParams.h>
#include <xAODTracking/VertexContainer.h>

namespace Easyjet {

/// \brief An algorithm for counting containers
class SecVtxFilterAlg final : public AthHistogramAlgorithm {

public:
  SecVtxFilterAlg(const std::string &name, ISvcLocator *pSvcLocator);

  /// \brief Initialisation method, for setting up tools and other persistent
  /// configs
  StatusCode initialize() override;
  /// \brief Execute method, for actions to be taken in the event loop
  StatusCode execute() override;
  /// \brief Function finalize the algorithm
  StatusCode finalize() override;


private:
  // Members for configurable properties
  // Vertex container
  SG::ReadHandleKey<ConstDataVector<xAOD::VertexContainer>> m_vertexInKey{
      this, "vertexIn", "", "xAOD::Vertex container"};

  // Filter params
  FilterReporterParams m_filterParams{this, "foundSecVertex",
                                      "at least one secondary vertex found"};
};

} // namespace Easyjet

#endif
