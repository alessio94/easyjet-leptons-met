///////////////////////// -*- C++ -*- /////////////////////////////
// Author: Teng Jian Khoo (teng.jian.khoo@cern.ch)
//

#include "NTrkVertexCounter.h"

// EDM includes
#include "xAODTracking/VertexContainer.h"

namespace HH4B {

    ///////////////////////////////////////////////////////////////////
    // Construct
    ///////////////////////////////////////////////////////////////////

    NTrkVertexCounter ::
    NTrkVertexCounter(const std::string& name) :
      AsgTool(name),
      m_minNtrks(2)
    {
      // Property name, member variable, documentation string
      declareProperty("NTrk_Min", m_minNtrks, "Minimum NTrks for vertex counting");
    }

    ///////////////////////////////////////////////////////////////////
    // Initialise
    ///////////////////////////////////////////////////////////////////

    StatusCode NTrkVertexCounter ::
    initialize()
    {
      ATH_MSG_DEBUG("Initialising " << name());
      ATH_MSG_INFO("Will count vertices with at least " << m_minNtrks << " tracks.");
      return StatusCode::SUCCESS;
    }

    ///////////////////////////////////////////////////////////////////
    // Execute
    ///////////////////////////////////////////////////////////////////

    bool NTrkVertexCounter ::
    accept(const xAOD::Vertex& vtx) const {
      // Use enums, not magic nummers
      // VxType::VertexType is defined in TrackingPrimitives.h
      // When writing selection functions, exit as early as possible.
      if( vtx.vertexType() != xAOD::VxType::PriVtx
          && vtx.vertexType() != xAOD::VxType::PileUp ) {return false;}
      return vtx.nTrackParticles() >= m_minNtrks;
    }

    // Implementation of public method
    // Done very explicitly for transparency, but one could use STL algorithms
    size_t NTrkVertexCounter ::
    countVertices(const xAOD::VertexContainer& vertices) const
    {
      size_t NVtx = 0;
      for(const xAOD::Vertex* vtx : vertices) {
        NVtx += accept(*vtx);
      }
      return NVtx;
    }

}
