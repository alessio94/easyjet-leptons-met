/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/// @author Derrick Allen
/// @author Jason Oliver - systematics, scale factors, handle aliasing.
// Always protect against multiple includes!

#ifndef TTCALIB_FINALVARSXBBCALIBALG
#define TTCALIB_FINALVARSXBBCALIBALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODMissingET/MissingETContainer.h>

namespace XBBCALIB
{

    using GaudiBool_t        = Gaudi::Property<bool>;
    using GaudiFloat_t       = Gaudi::Property<float>;
    using GaudiString_t      = Gaudi::Property<std::string>;
    using GaudiVectorofString_t = Gaudi::Property<std::vector<std::string>>;

    using listHandle_t       = CP::SysListHandle;

    using EventInfoHandle_t  = CP::SysReadHandle<xAOD::EventInfo>;
    using JetHandle_t        = CP::SysReadHandle<xAOD::JetContainer>;
    using ElectronHandle_t   = CP::SysReadHandle<xAOD::ElectronContainer>;
    using MuonHandle_t       = CP::SysReadHandle<xAOD::MuonContainer>;
    using METHandle_t        = CP::SysReadHandle<xAOD::MissingETContainer>;

    using CharHandle_t       = CP::SysReadDecorHandle<char>;
    using FloatHandle_t      = CP::SysReadDecorHandle<float>;
    using IntHandle_t        = CP::SysReadDecorHandle<int>;

    using FloatWriteHandle_t = CP::SysWriteDecorHandle<float>;
    using IntWriteHandle_t   = CP::SysWriteDecorHandle<int>;

  /// \brief An algorithm for counting containers
  class BaselineVarsttCalibAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    BaselineVarsttCalibAlg(const std::string &name, ISvcLocator *pSvcLocator);

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
    listHandle_t m_systematicsList {this};

    // Handles for ttCalib object containers
    JetHandle_t       m_ttcalib_jetHandle     { this, "ttcalib_jets"     , "ttCalibJets_%SYS%"     ,   "Jet container to read" };
    JetHandle_t       m_ttcalib_lrjetHandle   { this, "ttcalib_lrjets"   , "ttCalibLRJets_%SYS%"   ,   "Large-R jet container to read" };
    ElectronHandle_t  m_ttcalib_electronHandle{ this, "ttcalib_electrons", "ttCalibElectrons_%SYS%",   "Electron container to read" };
    MuonHandle_t      m_ttcalib_muonHandle    { this, "ttcalib_muons"    , "ttCalibMuons_%SYS%"    ,   "Muon container to read" };
    EventInfoHandle_t m_eventHandle   { this, "event"    , "EventInfo"              ,   "EventInfo container to read" };
    METHandle_t       m_metHandle     { this, "met"      , "AnalysisMET_%SYS%"      ,   "MET container to read" };

    GaudiBool_t m_isMC { this, "isMC", false, "Is this simulation?" };

    // Handles for electron and muon scale factor extraction tools
    ElectronHandle_t m_electronHandle { this, "electrons", "AnalysisElectrons_%SYS%", "Electron container to read" };
    MuonHandle_t     m_muonHandle     { this, "muons",     "AnalysisMuons_%SYS%",     "Muon container to read" };

    // Electron and muon ID working points
    GaudiString_t m_electronWPName { this, "eleWP" , "", "Electron ID + Iso working point" };
    GaudiString_t m_muonWPName  { this, "muonWP", "", "Muon ID + Iso working point" };

    FloatHandle_t m_ele_SF {"", this};
    FloatHandle_t m_mu_SF  {"", this};

    // electron and muon truth information
    IntHandle_t m_ele_truthOrigin{"truthOrigin", this};
    IntHandle_t m_ele_truthType{"truthType", this};
    IntHandle_t m_ele_firstEgMotherTruthType{"firstEgMotherTruthType", this};
    IntHandle_t m_ele_firstEgMotherTruthOrigin{"firstEgMotherTruthOrigin", this};
    IntHandle_t m_ele_firstEgMotherPdgId{"firstEgMotherPdgId", this};
    CharHandle_t m_eleECIDS { this, "ElectronsECIDS", "DFCommonElectronsECIDS", "Charge ID Selector" };

    // muon truth decorations
    IntHandle_t m_mu_truthOrigin{"truthOrigin", this};
    IntHandle_t m_mu_truthType{"truthType", this};



    GaudiVectorofString_t m_GN2X_WPs{ this, "GN2X_WPs", {}, "GN2X_hbb_WPs from the ttCalib config" };
    std::vector<IntHandle_t> m_GN2X_WP_Handles;

    GaudiFloat_t m_minMet {this, "minMet", 20000, "Minimum MET cut"};

    FloatHandle_t m_GN2Xv01_phbb = {this, "phbb", "GN2Xv01_phbb", "GN2Xv01_phbb"};
    FloatHandle_t m_GN2Xv01_phcc = {this, "phcc", "GN2Xv01_phcc", "GN2Xv01_phcc"};
    FloatHandle_t m_GN2Xv01_pqcd = {this, "pqcd", "GN2Xv01_pqcd", "GN2Xv01_pqcd"};
    FloatHandle_t m_GN2Xv01_ptop = {this, "ptop", "GN2Xv01_ptop", "GN2Xv01_ptop"};

    GaudiVectorofString_t m_floatVariables {this, "floatVariableList", {}, "Name list of floating variables"};
    GaudiVectorofString_t m_intVariables   {this, "intVariableList"  , {}, "Name list of integer variables"};

    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, FloatWriteHandle_t> m_floatBranches;

    std::unordered_map<std::string, IntWriteHandle_t> m_intBranches;

  };
}

#endif
