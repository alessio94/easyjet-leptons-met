/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EASYJET_TAUSELECTORALG
#define EASYJET_TAUSELECTORALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysListHandle.h>

#include <AthContainers/ConstDataVector.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODTau/TauJetContainer.h>

namespace Easyjet
{

  /// \brief An algorithm for counting containers
  class TauSelectorAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    TauSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);

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

    CP::SysReadHandle<xAOD::TauJetContainer>
      m_inHandle{ this, "containerInKey", "",   "Tau container to read" };

    Gaudi::Property<bool> m_keepAntiTaus
      {this, "keepAntiTaus", false, "Keep anti-taus in addition to ID taus"};

    CP::SysReadDecorHandle<char> m_antiTau{"", this};
    CP::SysReadDecorHandle<char> m_IDTau{"", this};

    Gaudi::Property<std::vector<std::string>> m_tauWPs
      { this, "tauWPs", {}, "Tau ID working points, not used to filter collection" };

    std::vector<CP::SysReadDecorHandle<char>> m_select_in;
    std::vector<CP::SysWriteDecorHandle<char>> m_select_out;

    /// \brief Setup syst-aware output container handles
    CP::SysWriteHandle<ConstDataVector<xAOD::TauJetContainer>>
      m_outHandle{ this, "containerOutKey", "",   "Tau container to write" };

    /// \brief Setup sys-aware output decorations
    CP::SysWriteDecorHandle<int>
      m_nSelPart{ this, "decorOutName", "nTaus_%SYS%",
	  "Name out output decorator for number of selected taus" };

    CP::SysWriteDecorHandle<bool > m_isSelectedTau {
        this, "decoration", "isAnalysisTau_%SYS%", "decoration for per-object if tau is selected"
    };

    Gaudi::Property<float> m_minPt            {this, "minPt", 20e3, "Minimum pT of taus"};
    Gaudi::Property<float> m_maxEta           {this, "maxEta", 2.5, "Maximum eta of taus"};
    Gaudi::Property<int>   m_minimumAmount    {this, "minimumAmount", -1, "Minimum number of taus to consider"}; // -1 means ignores this
    Gaudi::Property<bool>  m_pTsort           {this, "pTsort", true, "Sort taus by pT"};
    Gaudi::Property<int>   m_truncateAtAmount {this, "truncateAtAmount", -1, "Remove extra taus after pT sorting"}; // -1 means keep them all

    Gaudi::Property<int>   m_tauAmount       {this, "tauAmount", -1, "Number of taus to consider for isTauXX decoration"};
    std::unordered_map<std::string, CP::SysWriteDecorHandle<bool>> m_leadBranches;


  };
}

#endif
