/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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

#include <onnxruntime_cxx_api.h>

namespace HHBBYY
{
  enum BDT {
    low_mass_Run2  = 0,
    high_mass_Run2 = 1,
    VBFjets   = 2,
    HH2025_KF_low_mass_Run2 = 3,
    HH2025_KF_high_mass_Run2 = 4,
    HH2025_KF_low_mass_Run3 = 5,
    HH2025_KF_high_mass_Run3 = 6,
    HH2025_KF_low_mass_Run3_with2024 = 7,
    HH2025_KF_high_mass_Run3_with2024 = 8,
    HH2025_KF_low_mass_Run23 = 9,
    HH2025_KF_high_mass_Run23 = 10,
    HH2025_KF_low_mass_Run23_with2024 = 11,
    HH2025_KF_high_mass_Run23_with2024 = 12,
    HH2025_low_mass_Run2 = 13,
    HH2025_high_mass_Run2 = 14,
    HH2025_low_mass_Run3 = 15,
    HH2025_high_mass_Run3 = 16
  };

  enum GNN {
    ggFTarget = 0,
    VBFTarget = 1
  };

  enum Var {
    y1_ptOverMyy = 0,
    y1_eta,
    y1_phi,
    y2_ptOverMyy,
    y2_eta,
    y2_phi,
    met,
    met_phi,
    j1_pt,
    j1_eta,
    j1_phi,
    j1_pcbt,
    j2_pt,
    j2_eta,
    j2_phi,
    j2_pcbt,
    bb_pt,
    bb_eta,
    bb_phi,
    bb_m,
    jets_HT,
    topness,
    j3_pt,
    j3_eta,
    j3_phi,
    j3_pcbt,
    j4_pt,
    j4_eta,
    j4_phi,
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
    jets_HT_KF,
    topness_KF,
    bdt_sel_score_KF,
    bdt_sel_category_KF,
    bdt_sel_score_KF_with2024,
    bdt_sel_category_KF_with2024,
    bdt_sel_score_KF_corr,
    bdt_sel_category_KF_corr,
    bdt_sel_score_KF_corr_with2024,
    bdt_sel_category_KF_corr_with2024,
    bdt_sel_score_GNN,
    bdt_sel_category_GNN,
    vbfjj_dEta_KF,
    vbfjj_m_KF,
    vbfjj_dEta_GNN,
    vbfjj_m_GNN,
    sphericityT_KF,
    planarFlow_KF,
    sphericityT_GNN,
    planarFlow_GNN,
    bbyy_mStar_KF,
    bbyy_mStar_GNN,
    bb_m_KF_unconstrained,
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

  private:
    float compute_Topness(const xAOD::JetContainer *jets);
    std::vector<float> compute_EventShapes(const xAOD::Jet *Hbb_Jet1, const xAOD::Jet *Hbb_Jet2, const std::vector<TLorentzVector>& photons, const int& n_photons);
    float compute_pTBalance(const xAOD::Jet *Hbb_Jet1, const xAOD::Jet *Hbb_Jet2, const std::vector<TLorentzVector>& photons, const int& n_photons);

    VBFjetsMethod stringToVBFjetsMethod(const std::string& vbfjets_method_str);
    float getVBFjets_BDT(float ht, const TLorentzVector& ph1, const TLorentzVector& ph2,
                         const xAOD::Jet *Hbb_Jet1, const xAOD::Jet *Hbb_Jet2,
                         const xAOD::JetContainer *jets, std::array<TLorentzVector, 2>& Jets_vbf);
    void getVBFjets_mjj(const xAOD::Jet *Hbb_Jet1, const xAOD::Jet *Hbb_Jet2,
                        const xAOD::JetContainer *jets, std::array<TLorentzVector, 2>& Jets_vbf);
    void getVBFjets_pTsorting(const xAOD::Jet *Hbb_Jet1, const xAOD::Jet *Hbb_Jet2,
                              const xAOD::JetContainer *jets, std::array<TLorentzVector, 2>& Jets_vbf);
    std::vector<float> makeXGBoostDMatrixLegacyNonres(const TLorentzVector& ph1, const TLorentzVector& ph2,
                                                      ConstDataVector<xAOD::JetContainer> &categorisation_jets,
                                                      const xAOD::MissingETContainer *met, const auto &sys,
                                                      const std::map<HHBBYY::Var, float> &m_eventFloats, bool isKFvariables, bool isGNNvariables);

    StatusCode vbf_calculations(const TLorentzVector& ph1, const TLorentzVector& ph2,
                const int& n_photons,
				const xAOD::Jet *Hbb_Jet1, const xAOD::Jet *Hbb_Jet2, const xAOD::JetContainer *jets,
				double HT,const TLorentzVector& HH,
				const std::string& prefix_j, const std::string& prefix_jj,
				std::map<HHBBYY::Var, float> &eventFloats, const xAOD::EventInfo *event, const auto &sys);

    void performCategorisationBDT(const TLorentzVector& ph1, const TLorentzVector& ph2,
                                  const xAOD::Jet *Hbb_Jet1, const xAOD::Jet *Hbb_Jet2,
                                  const xAOD::JetContainer *jets,
                                  const xAOD::MissingETContainer *met,
                                  const auto &sys, std::map<HHBBYY::Var, float> &m_eventFloats,
                                  std::map<HHBBYY::Var, int> &m_eventInts, bool isKFvariables, bool isGNNvariables, bool isCorrelatedModel, bool is2024Model, int year);

    void loadBDT(const std::string &filePath, std::unique_ptr<MVAUtils::BDT> &bdt);

    ConstDataVector<xAOD::JetContainer> categorisation_jets(const xAOD::Jet *Hbb_Jet1, const xAOD::Jet *Hbb_Jet2, const xAOD::JetContainer *jets);

    std::vector<double> compute_angular_variables_CM(const TLorentzVector& lz_photon1,const TLorentzVector& lz_photon2,const TLorentzVector& lz_b_jet1,const TLorentzVector& lz_b_jet2);

    void fill_bb_branches(const std::vector<const xAOD::Jet*> &Hbb_jets, const std::string &prefix, const xAOD::EventInfo *event, const auto& sys);
    void fill_bbyy_branches(const std::vector<const xAOD::Jet*> &Hbb_jets, const std::vector<TLorentzVector> &Hyy_photons, const std::string &prefix, const xAOD::EventInfo *event, const auto& sys);
    void loadGNN(const std::string &filePath);

    std::vector<const xAOD::Jet*> getHbb_GNN_ggFTarget(const xAOD::JetContainer *jets, const TLorentzVector& ph1, const TLorentzVector& ph2, float& max_score, float pile_up, const auto &sys);
    std::vector<const xAOD::Jet*> getHbb_GNN_VBFTarget(const xAOD::JetContainer *jets, const TLorentzVector& ph1, const TLorentzVector& ph2, float& max_score, float pile_up, const auto &sys);

    /// \brief Setup syst-aware input container handles
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::JetContainer>
      m_bbyyJetHandle{ this, "bbyyJets", "bbyyAnalysisJets_%SYS%", "Jet container to read" };

    CP::SysReadDecorHandle<char>
    m_isBtag {this, "bTagWPDecorName", "", "Name of input decorator for b-tagging"};
    CP::SysReadDecorHandle<int> m_truthFlav{"HadronConeExclTruthLabelID", this};

    CP::SysReadDecorHandle<int>
    m_PCBT {this, "PCBTDecorName", "", "Name of pseudo-continuous b-tagging decorator"};

    CP::SysReadDecorHandle<int>
    m_nmuons{ this, "nmuons", "n_muons_%SYS%", "Number of muons from muon-in-jet correction"};

    CP::SysReadHandle<xAOD::JetContainer>
    m_KFJetHandle{this, "KFJets", "", "KF Jet container to read"};

    CP::SysReadDecorHandle<float> m_KF_MBB
      {"KF_mbb_%SYS%", this};
  

    CP::SysReadHandle<xAOD::MissingETContainer>
    m_metHandle{ this, "met", "AnalysisMET_%SYS%", "MET container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
    m_eventHandle{ this, "event", "EventInfo", "EventInfo container to read" };

    CP::SysReadDecorHandle<unsigned int> m_year
      {this, "year", "dataTakingYear", ""};

    Gaudi::Property<bool> m_isMC
      { this, "isMC", false, "Is this simulation?" };

    Gaudi::Property<bool> m_doKF
      { this, "doKF", false, "Do Kinematic Fit?" };

    Gaudi::Property<std::vector<std::string>> m_floatVariables
      {this, "floatVariableList", {}, "Name list of float variables"};

    Gaudi::Property<std::vector<std::string>> m_intVariables
      {this, "intVariableList", {}, "Name list of integer variables"};

    Gaudi::Property<bool> m_do_nonresonant_BDTs
      { this, "do_nonresonant_BDTs", false, "Do nonresonant BDT computations?" };

    Gaudi::Property<std::vector<std::string>> m_bdts_path
      {this, "BDT_path", {}, "Path to BDT model"};

    Gaudi::Property<bool> m_doGNN_tagging
      { this, "doGNN_tagging", false, "Do GNN 2bjet Selection?" };

    Gaudi::Property<std::vector<std::string>> m_GNNs_path
      {this, "GNN_path", {}, "Path to GNN model"};

    Gaudi::Property<std::string> m_vbfjets_method_str
      {this, "VBFjetsMethod", "", "VBF jets selection method"};

    Gaudi::Property<bool> m_save_VBF_vars
      { this, "save_VBF_vars", false, "Save VBF variables?" };

    Gaudi::Property<bool> m_doSystematics
      { this, "doSystematics", false, "Run on all systematics" };

    Gaudi::Property<bool> m_doResonantonebtag
      { this, "doResonantonebtag", false, "Compute additional quantities for the one btag region of the resonant SHbbyy analysis." };

    Gaudi::Property<bool> m_save_HbbCand_vars
      { this, "save_HbbCand_vars", false, "Save HbbCandidate Jet quantities" };

    Gaudi::Property<bool> m_save_extra_vars
      { this, "save_extra_vars", false, "Compute quantities which may be useful, but are not needed for the barebones SHbbyy analysis." };

    Gaudi::Property<bool> m_save_nonresonant_BDTInput_variables
      {this, "save_nonresonant_BDTInput_variables", false, "Compute quantities useful for the non-resonant BDT training"};
  
    // Dynamic decoration variable lists: photons
    Gaudi::Property<std::vector<std::string>> m_intVariables_photons
      { this, "intVariableList_photons", {}, "Name list of integer variables for photons" };

    Gaudi::Property<std::vector<std::string>> m_floatVariables_photons
      { this, "floatVariableList_photons", {}, "Name list of float variables for photons" };

    /// \brief Setup sys-aware read decorations photons
    std::unordered_map<std::string, CP::SysReadDecorHandle<int>>   m_Ibranches_photons;
    std::unordered_map<std::string, CP::SysReadDecorHandle<float>> m_Fbranches_photons;

    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>> m_Ibranches;
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_Fbranches;

    // Declare the BDTs
    std::vector<std::unique_ptr<MVAUtils::BDT>> m_bdts;

    // Declare the GNNs
    std::vector<std::unique_ptr<Ort::Session>> m_sessions;
    std::vector<std::unique_ptr<Ort::Env>> m_envs;

    std::vector<std::vector<std::string>> m_input_node_names;
    std::vector<std::vector<std::string>> m_output_node_names;
    std::vector<std::vector<std::vector<int64_t>>> m_input_node_dims_vector;
    std::vector<std::vector<std::vector<int64_t>>> m_output_node_dims_vector;
    std::vector<std::vector<int64_t>> m_input_node_dims_sum;
    std::vector<std::vector<int64_t>> m_output_node_dims_sum;

    // Declare the enum of m_vbfjets_method
    VBFjetsMethod m_vbfjets_method {VBFjetsMethod::invalid};

  };
}
#endif


