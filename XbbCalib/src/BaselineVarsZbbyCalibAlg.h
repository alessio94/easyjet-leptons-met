/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef ZBBYCALIB_FINALVARSXBBCALIBALG
#define ZBBYCALIB_FINALVARSXBBCALIBALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/PhotonContainer.h>

namespace XBBCALIB
{

  // forward declare, define in the Cxx file
  class FourVectorOutBlock;

  /// \brief An algorithm for counting containers
  class BaselineVarsZbbyCalibAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    BaselineVarsZbbyCalibAlg(const std::string &name, ISvcLocator *pSvcLocator);
    ~BaselineVarsZbbyCalibAlg();

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

    CP::SysReadHandle<xAOD::JetContainer>
    m_lrjetHandle{ this, "lrjets", "ZcandLRJets_%SYS%",   "Large-R jet container to read" };

    CP::SysReadHandle<xAOD::PhotonContainer>
    m_photonHandle{ this, "photons", "XbbCalibPhotons_%SYS%", "Photon container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo",   "EventInfo container to read" };

    CP::SysWriteDecorHandle<int> m_nLRJetsHandle {
      "lrjets_n_%SYS%", this
    };
    CP::SysWriteDecorHandle<int> m_nPhotonsHandle {
      "photons_n_%SYS%", this
    };

    // copy variables from the jet to the eventInfo
    template <typename T>
    using SRDH_t = CP::SysReadDecorHandle<T>;
    template <typename T>
    using SWDH_t = CP::SysWriteDecorHandle<T>;
    template <typename T>
    using rw_pair_t = std::pair<SRDH_t<T>, SWDH_t<T>>;

    Gaudi::Property<std::string> m_copied_variable_prefix{
      this, "copiedVariablePrefix", "", "prefix for copied variables"
    };

    // floats
    Gaudi::Property<std::vector<std::string>> m_floats_to_copy{
      this, "floatsToCopy", {}, "floats to copy to eventinfo"};
    std::vector<std::unique_ptr<rw_pair_t<float>>> m_float_copy_pairs;

    // ints
    Gaudi::Property<std::vector<std::string>> m_ints_to_copy{
      this, "intsToCopy", {}, "intss to copy to eventinfo"};
    std::vector<std::unique_ptr<rw_pair_t<int>>> m_int_copy_pairs;

    /// \brief Setup sys-aware output decorations
    std::unique_ptr<FourVectorOutBlock> m_photon_4vec;
    std::unique_ptr<FourVectorOutBlock> m_z_candidate_4vec;
  };
}

#endif
