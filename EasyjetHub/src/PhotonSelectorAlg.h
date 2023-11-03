/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EASYJET_PHOTONSELECTORALG
#define EASYJET_PHOTONSELECTORALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysListHandle.h>

#include <AthContainers/ConstDataVector.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODEgamma/PhotonContainer.h>

namespace Easyjet
{

  /// \brief An algorithm for counting containers
  class PhotonSelectorAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    PhotonSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

    CP::SysReadHandle<xAOD::PhotonContainer>
    m_inHandle{ this, "containerInKey", "",   "Photon container to read" };

    // \brief Setup syst-aware input decorations
    CP::SysReadDecorHandle<char> m_isLoose {"DFCommonPhotonsIsEMLoose", this};

    // Photin cleaning
    CP::SysReadDecorHandle<char> m_isClean {"DFCommonPhotonsCleaning", this};

    /// \brief Setup syst-aware output container handles
    CP::SysWriteHandle<ConstDataVector<xAOD::PhotonContainer>>
    m_outHandle{ this, "containerOutKey", "",   "Photon container to write" };

    /// \brief Setup sys-aware output decorations
    CP::SysWriteDecorHandle<int> m_nSelPart {this, "decorOutName", "Photons_%SYS%", 
        "Name out output decorator for number of selected photons"};

    Gaudi::Property<float> m_minPt            {this, "minPt", 25e3, "Minimum pT of photons"};
    Gaudi::Property<float> m_minEtaVeto       {this, "minEtaVeto", 1.37, "Minimum eta veto of EMCal"};
    Gaudi::Property<float> m_maxEtaVeto       {this, "maxEtaVeto", 1.52, "Maximum eta veto of EMCal"};
    Gaudi::Property<float> m_maxEta           {this, "maxEta", 2.37, "Maximum eta of photons"};
    Gaudi::Property<int>   m_minimumAmount    {this, "minimumAmount", -1, "Minimum number of photons to consider"}; // -1 means ignores this
    Gaudi::Property<bool>  m_pTsort           {this, "pTsort", true, "Sort photons by pT"};
    Gaudi::Property<int>   m_truncateAtAmount {this, "truncateAtAmount", -1, "Remove extra photons after pT sorting"}; // -1 means keep them all
  };
}

#endif
