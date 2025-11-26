/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef ZllyCaLIB_FINALVARSXBBCALIBALG
#define ZllyCaLIB_FINALVARSXBBCALIBALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

#include <xAODEgamma/ElectronContainer.h>
#include <xAODEgamma/PhotonContainer.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMissingET/MissingETContainer.h>
#include <xAODMuon/MuonContainer.h>

namespace XBBCALIB
{

  class FourVectorOutBlock;
  /// \brief An algorithm for counting containers
  class BaselineVarsZllyCalibAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
public:
    BaselineVarsZllyCalibAlg(const std::string &name,
                             ISvcLocator *pSvcLocator);
    ~BaselineVarsZllyCalibAlg();
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
    CP::SysListHandle m_systematicsList{this};

    CP::SysReadHandle<xAOD::JetContainer> m_lrjetHandle{
        this, "lrjets", "XbbCalibLRJets_%SYS%",
        "Large-R jet container to read"};

    CP::SysReadHandle<xAOD::MuonContainer> m_XbbCalib_muonHandle{
        this, "XbbCalib_muons", "XbbCalibMuons_%SYS%",
        "Muon container to read"};

    CP::SysReadHandle<xAOD::ElectronContainer> m_XbbCalib_electronHandle{
        this, "XbbCalib_electrons", "XbbCalibElectrons_%SYS%",
        "Electron container to read"};

    // Handles for electron and muon scale factor extraction tools
    CP::SysReadHandle<xAOD::ElectronContainer> m_electronHandle{
        this, "electrons", "AnalysisElectrons_%SYS%",
        "Original electron container to read"};

    CP::SysReadHandle<xAOD::MuonContainer> m_muonHandle{
        this, "muons", "AnalysisMuons_%SYS%",
        "Original muon container to read"};

    CP::SysReadHandle<xAOD::MissingETContainer> m_metHandle{
        this, "met", "AnalysisMET_%SYS%", "MET container to read"};

    CP::SysReadHandle<xAOD::PhotonContainer> m_XbbCalib_photonHandle{
        this, "XbbCalib_photons", "XbbCalibPhotons_%SYS%",
        "Photon container to read"};

    CP::SysReadHandle<xAOD::PhotonContainer> m_photonHandle{
        this, "photons", "AnalysisPhotons_%SYS%",
        "Original photon container to read"};

    CP::SysReadHandle<xAOD::EventInfo> m_eventHandle{
        this, "event", "EventInfo", "EventInfo container to read"};

    CP::SysWriteDecorHandle<int> m_nPhotonsHandle{"photons_n_%SYS%", this};
    CP::SysWriteDecorHandle<int> m_nElectronsHandle{"electrons_n_%SYS%", this};
    CP::SysWriteDecorHandle<int> m_nMuonsHandle{"muons_n_%SYS%", this};

    Gaudi::Property<bool> m_isMC{this, "isMC", false, "Is this simulation?"};

    Gaudi::Property<std::vector<std::string>> m_floatVariables{
        this, "floatVariableList", {}, "Name list of floating variables"};

    Gaudi::Property<std::vector<std::string>> m_intVariables{
        this, "intVariableList", {}, "Name list of integer variables"};

    /// \brief Setup sys-aware output decorations
    // std::unordered_map<std::string, CP::SysWriteDecorHandle<float>>
    // m_Fbranches;
    std::unordered_map<std::string,
                       std::unique_ptr<CP::SysWriteDecorHandle<float>>>
        m_Fbranches;
    std::unordered_map<std::string,
                       std::unique_ptr<CP::SysWriteDecorHandle<int>>>
        m_Ibranches;

    // std::unordered_map<std::string, CP::SysWriteDecorHandle<int>>
    // m_Ibranches; copy variables from the jet to the eventInfo
    template <typename T> using SRDH_t = CP::SysReadDecorHandle<T>;
    template <typename T> using SWDH_t = CP::SysWriteDecorHandle<T>;
    template <typename T> using rw_pair_t = std::pair<SRDH_t<T>, SWDH_t<T>>;

    Gaudi::Property<std::string> m_copied_variable_prefix{
        this, "copiedVariablePrefix", "", "prefix for copied variables"};

    // floats
    Gaudi::Property<std::vector<std::string>> m_floats_to_copy{
        this, "floatsToCopy", {}, "floats to copy to eventinfo"};
    std::vector<std::unique_ptr<rw_pair_t<float>>> m_float_copy_pairs;

    // ints
    Gaudi::Property<std::vector<std::string>> m_ints_to_copy{
        this, "intsToCopy", {}, "ints to copy to eventinfo"};
    std::vector<std::unique_ptr<rw_pair_t<int>>> m_int_copy_pairs;

    /// \brief Setup sys-aware output decorations
    std::unique_ptr<FourVectorOutBlock> m_photon_4vec;
    std::unique_ptr<FourVectorOutBlock> m_z_candidate_4vec;
    std::unique_ptr<FourVectorOutBlock> m_e0_candidate_4vec;
    std::unique_ptr<FourVectorOutBlock> m_e1_candidate_4vec;
    std::unique_ptr<FourVectorOutBlock> m_mu0_candidate_4vec;
    std::unique_ptr<FourVectorOutBlock> m_mu1_candidate_4vec;

    // Setup lepton SFs

    Gaudi::Property<std::string> m_eleWPName{
        this, "eleWP", "", "Electron ID + Iso working point"};
    CP::SysReadDecorHandle<float> m_ele_SF{"", this};
    Gaudi::Property<std::string> m_muWPName{this, "muonWP", "",
                                            "Muon ID + Iso working point"};
    CP::SysReadDecorHandle<float> m_mu_SF{"", this};
    CP::SysWriteDecorHandle<float> m_Electron1SFHandle{"Electron1_effSF_%SYS%",
                                                       this};
    CP::SysWriteDecorHandle<float> m_Electron2SFHandle{"Electron2_effSF_%SYS%",
                                                       this};
    CP::SysWriteDecorHandle<float> m_Muon1SFHandle{"Muon1_effSF_%SYS%", this};
    CP::SysWriteDecorHandle<float> m_Muon2SFHandle{"Muon2_effSF_%SYS%", this};

    // Setup photon SFs
    Gaudi::Property<std::string> m_photonWPName{
        this, "photonWP", "", "Photon ID + Iso working point"};

    CP::SysReadDecorHandle<float> m_ph_SF{"", this};
    CP::SysWriteDecorHandle<float> m_PhotonSFHandle{"photon_effSF_%SYS%",
                                                    this};
  };
} // namespace XBBCALIB

#endif
