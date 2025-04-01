/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Kehang Bai

#include "SecVtxFilterAlg.h"
#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>
#include <AthContainers/ConstDataVector.h>
#include <EventBookkeeperTools/FilterReporter.h>

namespace Easyjet {

SecVtxFilterAlg ::SecVtxFilterAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
    : AthHistogramAlgorithm(name, pSvcLocator) {}

StatusCode SecVtxFilterAlg ::initialize() {
  // Initializa input containers
  ATH_CHECK(m_vertexInKey.initialize());

  // Initialize filter parms
  ATH_CHECK(m_filterParams.initialize());

  return StatusCode::SUCCESS;
}

StatusCode SecVtxFilterAlg ::execute() {
  // Filter
  FilterReporter filter(m_filterParams, false);

  // Input handles
  SG::ReadHandle<ConstDataVector<xAOD::VertexContainer>> vertexIn(
      m_vertexInKey);
  ATH_CHECK(vertexIn.isValid());

  // Vertex container
  ConstDataVector<xAOD::VertexContainer> vertices = *vertexIn;

  // If there are more than 1 vertex found, pass the event
  if (vertices.size() != 0) {
    filter.setPassed(true);
  }

  return StatusCode::SUCCESS;
}

} // namespace Easyjet
