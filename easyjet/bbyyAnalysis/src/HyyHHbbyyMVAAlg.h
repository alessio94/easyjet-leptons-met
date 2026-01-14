/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Always protect against multiple includes!
#ifndef HHBBYYANALYSIS_MVAALG
#define HHBBYYANALYSIS_MVAALG

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODEgamma/PhotonContainer.h>
#include <xAODEgamma/PhotonContainer.h>
#include <xAODMissingET/MissingETContainer.h>

#include "MVAUtils/BDT.h"

namespace HHBBYY
{

  struct HyyMultiBDTResults
  {
    std::vector<float> multi_response;
    int                multiclass_index;
    int                binary_index;
    float              binary_score;
  };

  enum HyyBDTAllVariables {
    pT_yy, yAbs_yy, ph0_eta, ph1_eta,
    Dy_y_y, pTt_yy, phiStar_yy,
    N_j_central,  N_j, N_j_btag, m_jj, pT_yyjj,
    N_j_central30,  N_j_30, N_j_btag30, m_jj_30, pT_yyjj_30,
    pT_jj, pT_j1_30, Dy_j_j, Dphi_j_j, Deta_j_j, pT_yyj, m_yyj, m_yyjj,
    Dphi_yy_jj, DRmin_y_j, Dy_yy_jj, m_alljet, cosTS_yyjj,
    N_lep, N_e, N_mu, m_ee, m_mumu,
    met_TST, sumet_TST, MET_sig, phi_TST, pT_ll, pTlepMET, mTlepMET,
    flag_eTmiss,
    HT_30, Zepp,
    mu,
    score_recotop, recotop_m, recotop_phi, recotop_eta, recotop_pT,
    score_hybrtop, hybrtop_m, hybrtop_phi, hybrtop_eta, hybrtop_pT,
    fwdjet_eta, m_fwdJyy, deltaR_Wb_t2
  };

  /// \brief An algorithm for counting containers
  class HyyHHbbyyMVAAlg final : public AthHistogramAlgorithm
  {
    /// \brief The standard constructor
  public:
    HyyHHbbyyMVAAlg(const std::string &name, ISvcLocator *pSvcLocator);

    /// \brief Initialisation method, for setting up tools and other persistent 
    /// configs
    StatusCode initialize() override;

    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute() override;


  private:
    // ToolHandle<whatever> handle {this, "pythonName", "defaultValue", "someInfo"};

    // Systematics list
    CP::SysListHandle m_systematicsList {this};

    // Sys-aware input container handles
    CP::SysReadHandle<xAOD::JetContainer>
      m_jetHandle { this, "jets", "bbyyAnalysisJets_%SYS%", "Jet container to read" };

    CP::SysReadHandle<xAOD::PhotonContainer>
      m_photonHandle { this, "photons", "bbyyAnalysisPhotons_%SYS%", "Photon container to read" };

    CP::SysReadHandle<xAOD::EventInfo>
      m_eventHandle { this, "event", "EventInfo", "EventInfo container to read" };

    CP::SysReadHandle<xAOD::MissingETContainer>
      m_metHandle { this, "met", "AnalysisMET_%SYS%", "MET container to read" };

    // B-tagging decorator
    CP::SysReadDecorHandle<char>
      m_isBtag { this, "bTagWPDecorName", "", "Name of input decorator for b-tagging" };


    CP::SysReadDecorHandle<int> m_nCentralJets
      { this, "nCentralJets", "nCentralJets_%SYS%", "Number of Central Jets decoration" };

    CP::SysReadDecorHandle<int> m_nJets
      { this, "nJets", "nJets_%SYS%", "Number of Jets decoration" };

    CP::SysReadDecorHandle<int> m_nBJets
      { this, "nBJets", "nBJets_%SYS%", "Number of B Jets decoration" };

    // MET significance
    CP::SysReadDecorHandle<float>
      m_met_sig { this, "METSignificance", "significance_%SYS%", "Met Significance" };

    // Declare the BDTs
    std::vector<std::unique_ptr<MVAUtils::BDT>> m_bdts;

    // HyyMultiBDT helper methods
    std::vector<std::vector<float>> HyyMultiBDT_get_boundaries_multiclass();
    std::vector<float>              HyyMultiBDT_get_weight_multiclass();
    std::vector<std::string>        HyyMultiBDT_get_multiclass_names();

    void HyyMultiBDT_init(const std::string &folder);

    HyyMultiBDTResults HyyMultiBDT_perform(
      const xAOD::EventInfo *event,
      const auto            &sys,
      const xAOD::MissingET *met,
      const int              n_photons,
      const TLorentzVector  &ph1,
      const TLorentzVector  &ph2,
      int                    nCentralJets,
      int                    nJets,
      int                    nBJets
    );

    template<typename T>
    T var_or_nan(T value, T default_value);

    std::vector<float> get_HyyBDTInput(
      const std::vector<HyyBDTAllVariables> &var_ids,
      const xAOD::EventInfo                 *event,
      const auto                            &sys,
      const xAOD::MissingET                 *met,
      const int                              n_photons,
      const TLorentzVector                  &ph1,
      const TLorentzVector                  &ph2,
      const int                             &nCentralJets,
      const int                             &nJets,
      const int                             &nBJets
    );

    // Configuration: Hyy multiclass
    Gaudi::Property<bool> m_doHyy_multiclass
      { this, "doHyy_multiclass", false, "Do H-yy Couplings multiclass BDT?" };

    Gaudi::Property<std::string> m_HyyMultiBDT_folder
      { this, "HyyMultiBDT_folder", "", "Path to H-yy Couplings multiclass BDT" };

    // Dynamic decoration variable lists: photon-jets
    Gaudi::Property<std::vector<std::string>> m_intVariables_BDTInput
      { this, "intVariableList_BDTInput", {}, "Name list of integer variables for BDTInput" };

    Gaudi::Property<std::vector<std::string>> m_floatVariables_BDTInput
      { this, "floatVariableList_BDTInput", {}, "Name list of float variables for BDTInput" };

    /// \brief Setup sys-aware read decorations photonjets
    std::unordered_map<std::string, CP::SysReadDecorHandle<int>>   m_Ibranches_BDTInput;
    std::unordered_map<std::string, CP::SysReadDecorHandle<float>> m_Fbranches_BDTInput;

    // Declare HyyMultiBDT
    std::unique_ptr<MVAUtils::BDT>              HyyMultiBDT_multiclass;
    std::vector<std::unique_ptr<MVAUtils::BDT>> HyyMultiBDT_binaries;

    unsigned int                                HyyMultiBDT_num_classes = 0;
    std::vector<int>                            HyyMultiBDT_offset_cat;
    std::vector<std::vector<float>>             HyyMultiBDT_boundaries;
    std::vector<float>                          HyyMultiBDT_weight_multiclass;

    // Systematics control
    Gaudi::Property<bool> m_doSystematics
      { this, "doSystematics", false, "Run on all systematics" };

    // // Generic int/float variable lists for output decorations
    Gaudi::Property<std::vector<std::string>> m_intVariables
      { this, "intVariableList", {}, "Name list of integer variables" };

    Gaudi::Property<std::vector<std::string>> m_floatVariables
      { this, "floatVariableList", {}, "Name list of integer variables" };
    

    /// \brief Setup sys-aware output decorations
    std::unordered_map<std::string, CP::SysWriteDecorHandle<int>>   m_Ibranches;
    std::unordered_map<std::string, CP::SysWriteDecorHandle<float>> m_Fbranches;
  };

} // namespace HHBBYY

#endif // HHBBYYANALYSIS_MVAALG

