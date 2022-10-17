/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HH4BANALYSIS_JETSELECTORALG
#define HH4BANALYSIS_JETSELECTORALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <xAODBase/IParticleContainer.h>
#include <xAODEventInfo/EventInfo.h>

namespace HH4B
{

  /// \brief An algorithm for counting containers
  class JetSelectorAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    JetSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    SG::ReadHandleKey<xAOD::IParticleContainer> m_containerInKey{
        this, "containerInKey", "", "containerName to read"};
    SG::WriteHandleKey<ConstDataVector<xAOD::IParticleContainer>>
        m_containerOutKey{this, "containerOutKey", "",
                          "containerName to write"};
    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{
        this, "EventInfoKey", "EventInfo", "EventInfo container to dump"};

    std::string m_bTagWP;
    float m_minPt;
    float m_maxEta;
    int m_howManyToKeep;
    bool m_pTsort;
    std::unordered_map<std::string,
                       SG::AuxElement::Decorator<std::vector<float>>>
        m_fourVecDecos;
  };
}

#endif
