/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HHBBYYANALYSIS_FINALVARSYYBBALG
#define HHBBYYANALYSIS_FINALVARSYYBBALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>

#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>

#include <AthContainers/ConstDataVector.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODEgamma/PhotonContainer.h>
#include <xAODMissingET/MissingETContainer.h>

#include "MVAUtils/BDT.h"



namespace HHBBYY
{
  enum BDT {
    low_mass  = 0,
    high_mass = 1,
    VBFjets   = 2
  };
  
  enum Var {
    y1_ptOverMyy = 0,
    y1_eta,
    y1y1_deltaPhi,
    y2_ptOverMyy,
    y2_eta,
    y1y2_deltaPhi,
    met,
    y1met_deltaPhi,
    j1_pt,
    j1_eta,
    y1j1_deltaPhi,
    j1_pcbt,
    j2_pt,
    j2_eta,
    y1j2_deltaPhi,
    j2_pcbt,
    bb_pt,
    bb_eta,
    y1bb_deltaPhi,
    bb_m,
    jets_HT,
    topness,
    j3_pt,
    j3_eta,
    y1j3_deltaPhi,
    j3_pcbt,
    j4_pt,
    j4_eta,
    y1j4_deltaPhi,
    j4_pcbt,
    vbfjj_dEta,
    vbfjj_m,
    bbyy_mStar,
    yy_dR,
    bb_dR,
    sphericityT,
    planarFlow,
    bbyy_ptOverSumPt,
    NVars,
    bdt_sel_score,
    bdt_sel_category,
    size_enum,
  };

  enum VBFVars {
    HT = 0,
    vbf_jj_m,
    vbf_jj_deta,
    dR_yybb_vbfj1,
    dR_yybb_vbfj2,
    deta_yybb_vbfj1,
    deta_yybb_vbfj2,
    dR_yybb_jj,
    deta_yybb_jj,
    pT_yybbjj,
    eta_yybbjj,
    m_yybbjj,
    vbf_j1_pt,
    vbf_j1_eta,
    vbf_j2_pt,
    vbf_j2_eta,
    nVars
  };

  enum VBFjetsMethod {
    BDT,
    mjj,
    pTsorting,
    invalid
  };

  /// \brief An algorithm for counting containers
  class BaselineVarsbbyyAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
  public:
    BaselineVarsbbyyAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

    float compute_Topness(const xAOD::JetContainer *jets);
    float* compute_EventShapes(std::unique_ptr<ConstDataVector<xAOD::JetContainer>> &bjets, const xAOD::PhotonContainer *photons);
    float compute_pTBalance(std::unique_ptr<ConstDataVector<xAOD::JetContainer>> &bjets, const xAOD::PhotonContainer *photons);
    
    VBFjetsMethod stringToVBFjetsMethod(const std::string& vbfjets_method_str);
    float getVBFjets_BDT(float ht, const xAOD::Photon *ph1, const xAOD::Photon *ph2,
                      const xAOD::Jet *Hbb_Jet1, const xAOD::Jet *Hbb_Jet2,
                      const xAOD::JetContainer *jets, TLorentzVector Jets_vbf[2]);
    void getVBFjets_mjj(const xAOD::Jet *Hbb_Jet1, const xAOD::Jet *Hbb_Jet2,
                        const xAOD::JetContainer *jets, TLorentzVector Jets_vbf[2]);
    void getVBFjets_pTsorting(const xAOD::Jet *Hbb_Jet1, const xAOD::Jet *Hbb_Jet2,
                              const xAOD::JetContainer *jets, TLorentzVector Jets_vbf[2]);
    std::vector<float> makeXGBoostDMatrixLegacyNonres(const xAOD::Photon *ph1, const xAOD::Photon *ph2, 
                                                      ConstDataVector<xAOD::JetContainer> &categorisation_jets,
                                                      const xAOD::MissingETContainer *met, const auto &sys,
                                                      const std::map<HHBBYY::Var, float> &m_eventFloats);

    void performCategorisationBDT(const xAOD::Photon *ph1, const xAOD::Photon *ph2,
                                  const xAOD::Jet *Hbb_Jet1, const xAOD::Jet *Hbb_Jet2, 
                                  const xAOD::JetContainer *jets,
                                  const xAOD::MissingETContainer *met,
                                  const auto &sys, std::map<HHBBYY::Var, float> &m_eventFloats,
                                  std::map<HHBBYY::Var, int> &m_eventInts);

    void loadBDT(const std::string &filePath, std::unique_ptr<MVAUtils::BDT> &bdt);

    ConstDataVector<xAOD::JetContainer> categorisation_jets(const xAOD::Jet *Hbb_Jet1, const xAOD::Jet *Hbb_Jet2, const xAOD::JetContainer *jets);

    std::vector<double> compute_angular_variables_CM(const TLorentzVector& lz_photon1,const TLorentzVector& lz_photon2,const TLorentzVector& lz_b_jet1,const TLorentzVector& lz_b_jet2);

  private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue",
    // "someInfo"};

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::JetContainer>
    m_jetHandle{ this, "jets", "bbyyAnalysisJets_%SYS%", "Jet container to read" };

    CP::SysReadDecorHandle<char> 
    m_isBtag {this, "bTagWPDecorName", "", "Name of input decorator for b-tagging"};

    CP::SysReadDecorHandle<int> 
    m_PCBT {this, "PCBTDecorName", "", "Name of pseudo-continuous b-tagging decorator"};

    CP::SysReadHandle<xAOD::PhotonContainer>
    m_photonHandle{ this, "photons", "bbyyAnalysisPhotons_%SYS%", "Photons container to read" };

    CP::SysReadHandle<xAOD::ElectronContainer>
    m_electronHandle{ this, "electrons", "bbyyAnalysisElectrons_%SYS%", "Electron container to read" };

    CP::SysReadHandle<xAOD::MuonContainer>
    m_muonHandle{ this, "muons", "bbyyAnalysisMuons_%SYS%", "Muon container to read" };

    CP::SysReadHandle<xAOD::JetContainer>
    m_KFJetHandle{this, "KFJets", "", "KF Jet container to read"};

    Gaudi::Property<std::string> m_photonWPName
      { this, "photonWP", "", "Photon ID + Iso working point" };
    CP::SysReadDecorHandle<float> m_ph_SF{"", this};

    CP::SysReadHandle<xAOD::MissingETContainer>
    m_metHandle{ this, "met", "AnalysisMET_%SYS%", "MET container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    Gaudi::Property<bool> m_doKF
      { this, "doKF", false, "Do Kinematic Fit?" };
    
    Gaudi::Property<std::vector<std::string>> m_floatVariables
      {this, "floatVariableList", {}, "Name list of float variables"};
    
    Gaudi::Property<std::vector<std::string>> m_intVariables
      {this, "intVariableList", {}, "Name list of integer variables"};

    Gaudi::Property<std::vector<std::string>> m_bdts_path 
      {this, "BDT_path", {}, "Path to BDT model"};

    Gaudi::Property<std::string> m_vbfjets_method_str
      {this, "VBFjetsMethod", "", "VBF jets selection method"};

    CP::SysReadDecorHandle<bool> 
    m_selected_ph { this, "selected_ph", "selected_ph_%SYS%", "Name of input decorator for selected ph"};

    
    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_Fbranches;
    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;

    // Declare the BDTs
    std::vector<std::unique_ptr<MVAUtils::BDT>> m_bdts;

    // Declare the enum of m_vbfjets_method
    VBFjetsMethod m_vbfjets_method;
  };
}
#endif
