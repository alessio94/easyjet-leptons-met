///////////////////////// -*- C++ -*- /////////////////////////////
// IVertexCounter.h
// Header file for interface class IVertexCounter
//
// This declares what an external class is allowed to use from this tool type
// Should not expose details of implementation that might change
//
// Author: T.J.Khoo<khoo@cern.ch>
///////////////////////////////////////////////////////////////////
// Always protect against multiple includes!
#ifndef PLOTTING_IVERTEXCOUNTER_H
#define PLOTTING_IVERTEXCOUNTER_H

#include "AsgTools/IAsgTool.h"
#include "xAODTracking/VertexContainer.h"

// Encapsulate in namespace to avoid name clashes
namespace MSA
{

  class IVertexCounter : virtual public asg::IAsgTool
  {
    //Declare the interface that the class provides
    ASG_TOOL_INTERFACE(MFA::IVertexCounter)

  public:
    /// \brief Count vertices in a supplied container.
    ///        Method is const, because we don't allow tools to
    ///        modify themselves after initialisation.
    virtual size_t countVertices(const xAOD::VertexContainer &) const = 0;
  };
}

#endif // PLOTTING_ISVERTEXCOUNTER_H
