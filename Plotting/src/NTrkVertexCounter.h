///////////////////////// -*- C++ -*- /////////////////////////////
// SayHelloTool.h
// Header file for class SayHelloTool
// Author: T.J.Khoo<khoo@cern.ch>
///////////////////////////////////////////////////////////////////
#ifndef PLOTTING_SAYHELLOTOOL_H
#define PLOTTING_SAYHELLOTOOL_H

// STL includes
#include <string>

// FrameWork includes
#include "AsgTools/AsgTool.h"

// Local includes
#include "Plotting/IVertexCounter.h"

// EDM includes
#include "xAODTracking/VertexContainer.h"

// Forward declaration

namespace MSA
{

  class NTrkVertexCounter final
      : virtual public IVertexCounter,
        public asg::AsgTool
  {
    ASG_TOOL_CLASS(NTrkVertexCounter, IVertexCounter)

    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////
  public:
    /// Constructor with parameters:
    NTrkVertexCounter(const std::string &name);
    ~NTrkVertexCounter(){};

    // Athena algtool's Hooks
    StatusCode initialize();

    ///////////////////////////////////////////////////////////////////
    // Const methods:
    ///////////////////////////////////////////////////////////////////

    /// \brief Implements the interface specified in IVertexCounter
    size_t countVertices(const xAOD::VertexContainer &) const override;

    ///////////////////////////////////////////////////////////////////
    // Private methods:
    ///////////////////////////////////////////////////////////////////
  private:
    bool accept(const xAOD::Vertex &) const;

    /// Default constructor:
    // NTrkVertexCounter();

    ///////////////////////////////////////////////////////////////////
    // Private data:
    ///////////////////////////////////////////////////////////////////

    // Member variables used for configuration of the tool
    // ~Always use size_t for counting -- it is unsigned and is large enough
    // to accommodate the size of any container
    size_t m_minNtrks;
  };

} //> end namespace MSA
#endif //> !PLOTTING_NTRKVERTEXCOUNTER_H
