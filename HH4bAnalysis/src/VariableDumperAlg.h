///////////////////////// -*- C++ -*- /////////////////////////////
// VariableDumperAlg.h
//
// This is an algorithm that will dump variables into a tree.
//
// Author: Victor Ruelas<victor.hugo.ruelas.rivera@cern.ch>
///////////////////////////////////////////////////////////////////

// Always protect against multiple includes!
#ifndef HH4BANALYSIS_VARIABLEDUMPERALG
#define HH4BANALYSIS_VARIABLEDUMPERALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <xAODJet/JetContainer.h>
#include <xAODEventInfo/EventInfo.h>

namespace HH4B
{

  /// \brief An algorithm for plotting dilepton masses
  class VariableDumperAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
  public:
    VariableDumperAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

  private:
    StatusCode bookTTree();

    // Call in execute to fill the EventInfo and jet variables
    StatusCode fillVariableTTree(const xAOD::EventInfo &, const xAOD::JetContainer &);

    // Member variables for configuration
    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{this, "EventInfoKey", "", "EventInfo container to dump"};
    SG::ReadHandleKey<xAOD::JetContainer> m_JetsKey{this, "JetsKey", "", "Jets container to dump"};

    // output variables for the current event
    unsigned int m_runNumber = 0;
    unsigned long long m_eventNumber = 0;
    // Jet 4-momentum variables
    std::vector<float> m_jetEta;
    std::vector<float> m_jetPhi;
    std::vector<float> m_jetPt;
    std::vector<float> m_jetE;
  };
}

#endif
