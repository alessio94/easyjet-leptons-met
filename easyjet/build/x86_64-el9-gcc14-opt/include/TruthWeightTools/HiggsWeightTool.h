/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

#pragma once

// EDM include(s):
#include "AsgTools/AnaToolHandle.h"
#include "PMGAnalysisInterfaces/IPMGTruthWeightTool.h"

// Local include(s):
#include "TruthWeightTools/HiggsWeights.h"
#include "TruthWeightTools/IHiggsWeightTool.h"

namespace TruthWeightTools
{

  /// Tool for accessing of MC weights and other weigthts for QCD uncertainty propagation for Higgs analyses
  ///
  /// @author Dag Gillberg <dag.gillberg@cern.ch>
  /// @author James Robinson <james.robinson@cern.ch>
  ///
  class HiggsWeightTool : public virtual IHiggsWeightTool, public asg::AsgTool
  {
    /// Create a proper constructor for Athena
    ASG_TOOL_CLASS(HiggsWeightTool, IHiggsWeightTool)

  public:

    /// Create a constructor for standalone usage
    HiggsWeightTool(const std::string &name);
    virtual ~HiggsWeightTool() {}

    virtual StatusCode initialize();

    virtual void printSummary();

    /// @name Function(s) accessed via the truth weight tool
    /// @{

    /// Get Higgs weights
    HiggsWeights getHiggsWeights(int HTXS_Njets30, double HTXS_pTH, int HTXS_Stage1, const xAOD::EventInfo* eventInfo);


    /// Get Higgs weights
    HiggsWeights getHiggsWeights(int HTXS_Njets30, double HTXS_pTH, int HTXS_Stage1,  int HTXS_Stage1p2, int STXS_Stage1p2Fine, const xAOD::EventInfo* eventInfo);

    /// @}

  private:

    /// Get the MC weight vector
    std::vector<float> getEventWeights( const xAOD::EventInfo* eventInfo ) const;

    /// Value of MC event weight
    float getWeight(const xAOD::EventInfo* eventInfo, const std::string& wName);

    /// Value of MC event weight
    bool hasWeight(const std::string& wName);

    /// Index of MC event weight
    size_t getWeightIndex(const std::string& wName);

    /// linear interpolation
    double linInter(double x, double x1, double y1, double x2, double y2);

    // returns hardcoded list of weight names matching expected weight structure
    std::vector<std::string> loadXMLWeightNames(const std::string& filename);

    /// Weight names in metadata
    std::vector<std::string> getWeightNames();


    /// Access the HiggsWeights
    HiggsWeights getHiggsWeightsInternal(int HTXS_Njets30, double HTXS_pTH, int HTXS_Stage1, int STXS_Stage1p2, int STXS_Stage1p2Fine, const xAOD::EventInfo* eventInfo);


    // Helper for ggF weights
    void fillggFUpdatedWeights(HiggsWeights& hw, int STXS_Njets30, double STXS_pTH, int STXS_Stage1, int STXS_Stage1p2, int STXS_Stage1p2Fine, const xAOD::EventInfo* eventInfo);


    // Copied function from LHC XS WG
    void fillVBFWeights(HiggsWeights& hw, int STXS_Stage1p2, const xAOD::EventInfo* eventInfo);
    double vbf_uncert_stage_1_2(int source, int event_STXS, double Nsigma=1.0);

    // Copied function from LHC XS WG
    void fillqq2HWeights(HiggsWeights& hw, int STXS_Stage1p2, const xAOD::EventInfo* eventInfo);
    double qq2H_uncert_stage_1_2(int source, int event_STXS, double Nsigma=1.0);


    // VH-lep
    double VHlep_uncert_stage_1_2_fine(int source, int event_STXS, double Nsigma=1.0);
    void fillggVHWeights(HiggsWeights& hw, int STXS_Stage1p2Fine, const xAOD::EventInfo* eventInfo);
    void fillqqVHWeights(HiggsWeights& hw, int STXS_Stage1p2Fine, const xAOD::EventInfo* eventInfo);

    // ttH
    void fillttHWeights(HiggsWeights& hw, int STXS_Stage1p2Fine, const xAOD::EventInfo* eventInfo);
    double ttH_uncert_stage_1_2_fine(int source, int event_STXS, double Nsigma=1.0);


    /// Protect against non-finite or outside-reqired-range weights
    void updateWeights(HiggsWeights &hw);
    void updateWeight(const double &w_nom, double &w);
    void updateWeights(const double &w_nom, std::vector<double> &ws) { for (auto &w : ws) updateWeight(w_nom, w); }

    /// Setup weights
    void setupWeights(size_t Nweights);

    /// getWeight
    double getWeight(size_t idx);
    double getWeight(const std::vector<float> &ws, size_t idx);

    size_t getIndex(const std::string& wn);

    /// Flags
    bool m_init;

    enum mode { AUTO = 0, FORCE_GGF_NNLOPS = 1, FORCE_POWPY8_VBF = 2, FORCE_POWPY8_VH = 3, FORCE_POWPY8_TTH = 4 };
    mode m_mode;
    bool m_forceNNLOPS, m_forceVBF, m_forceVH, m_forceTTH;

    /// number of expected weights
    size_t m_nWeights;

    /// Current MC channel number
    uint32_t m_mcID;

    /// The truth weight tool
    asg::AnaToolHandle<PMGTools::IPMGTruthWeightTool> m_weightTool;


    /// options
    bool m_requireFinite;
    bool m_cutOff;
    bool m_is13p6TeVSample;
    double m_weightCutOff;
    std::string  m_prodMode;

    /// For statistics
    int m_Nnom, m_Nws;
    double m_sumw_nom, m_sumw2_nom, m_sumw, m_sumw2;
    double m_sumw_nomC, m_sumw2_nomC, m_sumwC, m_sumw2C;

    /// index of weights
    size_t m_nom;

    /// weight indices for PDF+alphaS uncertainites
    std::vector<size_t> m_pdfUnc, m_pdfNNPDF30;
    size_t m_aS_up, m_aS_dn;

    /// Special PDF sets
    size_t m_nnpdf30_nlo, m_nnpdf30_nnlo, m_mmht2014nlo, m_pdf4lhc_nlo, m_pdf4lhc_nnlo;
    size_t m_ct10nlo, m_ct10nlo_0118, m_ct14nlo, m_ct14nlo_0118;

    /// Special weight indices for Powheg NNLOPS
    size_t m_tinf, m_bminlo, m_nnlopsNom;
    std::vector<size_t> m_qcd, m_qcd_nnlops;

    /// For extra QCD systematics
    size_t m_muR1p0_muF1p0;
    size_t m_muR0p5_muF0p5;

  };

} // namespace TruthWeightTools


