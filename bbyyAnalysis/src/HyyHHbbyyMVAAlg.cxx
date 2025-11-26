/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "HyyHHbbyyMVAAlg.h"
#include <fstream>
#include <AthContainers/ConstDataVector.h>
#include <AthenaKernel/Units.h>
#include "PathResolver/PathResolver.h"
#include "lwtnn/parse_json.hh"
#include "TFile.h"

namespace HHBBYY
{

  HyyHHbbyyMVAAlg::HyyHHbbyyMVAAlg(const std::string &name,
                                   ISvcLocator *pSvcLocator)
    : AthHistogramAlgorithm(name, pSvcLocator)
  {}

  StatusCode HyyHHbbyyMVAAlg::initialize()
  {
    ATH_MSG_INFO("*********************************\n");
    ATH_MSG_INFO("       HyyHHbbyyMVAAlg           \n");
    ATH_MSG_INFO("*********************************\n");

    ATH_CHECK(m_jetHandle.initialize(m_systematicsList));
    if (!m_isBtag.empty()) {
      ATH_CHECK(m_isBtag.initialize(m_systematicsList, m_jetHandle));
    }

    ATH_CHECK(m_photonHandle.initialize(m_systematicsList));
    ATH_CHECK(m_eventHandle.initialize(m_systematicsList));
    ATH_CHECK (m_metHandle.initialize(m_systematicsList));
    ATH_CHECK(m_met_sig.initialize(m_systematicsList, m_metHandle));

    ATH_CHECK(m_nCentralJets.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_nJets.initialize(m_systematicsList, m_eventHandle));
    ATH_CHECK(m_nBJets.initialize(m_systematicsList, m_eventHandle));

    // Setup decorations for BDTInput variables (int)
    for (const std::string &string_var : m_intVariables_BDTInput) {
      CP::SysReadDecorHandle<int> var { string_var + "_%SYS%", this };
      m_Ibranches_BDTInput.emplace(string_var, var);
      ATH_CHECK(m_Ibranches_BDTInput.at(string_var)
                  .initialize(m_systematicsList, m_eventHandle));
    }

    // Setup decorations for BDTInput variables (float)
    for (const std::string &string_var : m_floatVariables_BDTInput) {
      CP::SysReadDecorHandle<float> var {string_var + "_%SYS%", this };
      m_Fbranches_BDTInput.emplace(string_var, var);
      ATH_CHECK(m_Fbranches_BDTInput.at(string_var)
                  .initialize(m_systematicsList, m_eventHandle));
    }

    // Output float decorations
    for (const std::string &string_var : m_floatVariables) {
      CP::SysWriteDecorHandle<float> var { string_var + "_%SYS%", this };
      m_Fbranches.emplace(string_var, var);
      ATH_CHECK(m_Fbranches.at(string_var)
                  .initialize(m_systematicsList, m_eventHandle));
    }

    // Output int decorations
    for (const std::string &string_var : m_intVariables) {
      CP::SysWriteDecorHandle<int> var { string_var + "_%SYS%", this };
      m_Ibranches.emplace(string_var, var);
      ATH_CHECK(m_Ibranches.at(string_var)
                  .initialize(m_systematicsList, m_eventHandle));
    }

    // Init H-yy Couplings multiclass BDT
    if (m_doHyy_multiclass) {
      HyyMultiBDT_init(m_HyyMultiBDT_folder);
    }

    // Initialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK(m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode HyyHHbbyyMVAAlg::execute()
  {
    // Loop over all systs
    for (const auto &sys : m_systematicsList.systematicsVector()) {

      // In case of special Higgs sample, run only on NOSYS
      if (!m_doSystematics && sys.name() != "")
        continue;

      // Containers we read in
      const xAOD::EventInfo *event = nullptr;
      ANA_CHECK(m_eventHandle.retrieve(event, sys));

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK(m_jetHandle.retrieve(jets, sys));

      const xAOD::MissingETContainer *metCont = nullptr;
      ANA_CHECK(m_metHandle.retrieve(metCont, sys));

      const xAOD::MissingET *met = (*metCont)["Final"];
      if (!met) {
        ATH_MSG_ERROR("Could not retrieve MET");
        return StatusCode::FAILURE;
      }

      TLorentzVector ph1(0.,0.,0.,0.);
      ph1.SetPtEtaPhiE(m_Fbranches_BDTInput.at("Photon1_pt").get(*event, sys),
                       m_Fbranches_BDTInput.at("Photon1_eta").get(*event, sys),
                       m_Fbranches_BDTInput.at("Photon1_phi").get(*event, sys),
                       m_Fbranches_BDTInput.at("Photon1_E").get(*event, sys));
      TLorentzVector ph2(0.,0.,0.,0.);
      ph2.SetPtEtaPhiE(m_Fbranches_BDTInput.at("Photon2_pt").get(*event, sys),
                       m_Fbranches_BDTInput.at("Photon2_eta").get(*event, sys),
                       m_Fbranches_BDTInput.at("Photon2_phi").get(*event, sys),
                       m_Fbranches_BDTInput.at("Photon2_E").get(*event, sys));
      int n_photons =  m_Ibranches_BDTInput.at("nPhotons").get(*event, sys);

      std::vector<TLorentzVector> Hyy_photons = { ph1, ph2 };
      
      int nCentralJets = m_nCentralJets.get(*event, sys);
      int nJets        = m_nJets.get(*event, sys);
      int nBJets       = m_nBJets.get(*event, sys);

      // H-yy Couplings multiclass BDT Implementation
      if (m_doHyy_multiclass) {
        HHBBYY::HyyMultiBDTResults r =
          HyyMultiBDT_perform(event, sys, met,
                              n_photons, ph1, ph2,
                              nCentralJets, nJets, nBJets);

        m_Ibranches.at("HyyMultiBDT_multiclass_index").set(*event, r.multiclass_index, sys);
        m_Ibranches.at("HyyMultiBDT_binary_index").set(*event, r.binary_index, sys);
        m_Fbranches.at("HyyMultiBDT_binary_score").set(*event, r.binary_score, sys);

        const std::vector<std::string> names = HyyMultiBDT_get_multiclass_names();
        for (std::size_t i = 0; i < names.size(); ++i) {
            m_Fbranches.at(names[i]).set(*event, r.multi_response[i], sys);
        }
      }
      
    } // end loop over systematics

    return StatusCode::SUCCESS;
  }

  // NOLINT(modernize-use-std-numbers)
  std::vector<std::vector<float>>
  HyyHHbbyyMVAAlg::HyyMultiBDT_get_boundaries_multiclass()
  {
    // high threshold means very pure
    // (0) <<-- less pure | -->> more pure (1)
    return {
      {0.5390599866365131},                               // GG2H_0J_PTH_0_10
      {},                                                 // GG2H_0J_PTH_GT10
      {0.4343733552631579},                               // GG2H_1J_PTH_0_60  // NOLINT(modernize-use-std-numbers)
      {0.39341917660361847},                              // GG2H_1J_PTH_60_120
      {0.4069370836759869},                               // GG2H_1J_PTH_120_200
      {0.4342382085376101,  0.6557343291836714},          // GG2H_GE2J_MJJ_0_350_PTH_0_60  // NOLINT(modernize-use-std-numbers)
      {0.47669769736842105},                              // GG2H_GE2J_MJJ_0_350_PTH_60_120
      {0.43264573396381584},                              // GG2H_GE2J_MJJ_0_350_PTH_120_200
      {0.3389567359095229,  0.5089861528541939},          // GG2H_GE2J_MJJ_350_700_PTH_0_200
      {0.3338366090473377,  0.5253786871659128},          // GG2H_GE2J_MJJ_700_1000_PTH_0_200
      {0.3508147210384671,  0.49530318037836085},         // GG2H_GE2J_MJJ_GT1000_PTH_0_200
      {0.44375187088815793},                              // GG2H_PTH_200_300
      {0.36541900773287594, 0.5900783270997434},          // GG2H_PTH_300_450
      {0.3855770829037619,  0.5199264422215908},          // GG2H_PTH_450_650
      {},                                                 // GG2H_PTH_GT650
      {0.5634193801202272, 0.7814422454407337},           // QQ2HQQ_0J  // NOLINT(modernize-use-std-numbers)
      {0.4326597320289005, 0.6491818407456339},           // QQ2HQQ_1J
      {0.4669492103360201, 0.6934343304219999},           // QQ2HQQ_GE2J_MJJ_0_60  // NOLINT(modernize-use-std-numbers)
      {0.2961276033308943, 0.5364035198556749},           // QQ2HQQ_GE2J_MJJ_60_120
      {0.38908549970702133, 0.6637830841014256},          // QQ2HQQ_GE2J_MJJ_120_350
      {0.34979975924209555, 0.636725186309061},           // QQ2HQQ_GE2J_MJJ_350_700_PTH_0_200
      {0.5324046078330593},                               // QQ2HQQ_GE2J_MJJ_700_1000_PTH_0_200
      {0.476258195415296},                                // QQ2HQQ_GE2J_MJJ_GT1000_PTH_0_200
      {0.37367643416555274, 0.6316457307514391},          // QQ2HQQ_GE2J_MJJ_350_700_PTH_GT200
      {0.5409615234375},                                  // QQ2HQQ_GE2J_MJJ_700_1000_PTH_GT200
      {0.4950179893092106},                               // QQ2HQQ_GE2J_MJJ_GT1000_PTH_GT200
      {0.5228712273848684},                               // QQ2HLNU_PTV_0_75
      {0.44227105263157895},                              // QQ2HLNU_PTV_75_150
      {0.1504621710526317},                               // QQ2HLNU_PTV_150_250
      {0.06807878289473687},                              // QQ2HLNU_PTV_GT250
      {0.861470538651316},                                // QQ2HLL_PTV_0_75
      {0.059182894736842115},                             // QQ2HLL_PTV_75_150
      {0.24136918174342106},                              // QQ2HLL_PTV_150_250
      {},                                                 // QQ2HLL_PTV_GT250
      {0.3319628949617084,  0.6013439724570825},          // QQ2HNUNU_PTV_0_75
      {0.54043133354187,    0.7930990196629575},          // QQ2HNUNU_PTV_75_150
      {0.27316551303612563, 0.5415385652767984},          // QQ2HNUNU_PTV_150_250
      {0.18128626644736845},                              // QQ2HNUNU_PTV_GT250
      {0.38235748350973175, 0.6472071170665714},          // TTH_PTH_0_60
      {0.42209490388569076},                              // TTH_PTH_60_120
      {0.40546262078536194},                              // TTH_PTH_120_200
      {0.28798660567434214},                              // TTH_PTH_200_300
      {0.09902368421052643},                              // TTH_PTH_GT300
      {0.5233851562500003},                               // THJB
      {0.3789814453124999},                               // THW
    };
  }

  std::vector<float>
  HyyHHbbyyMVAAlg::HyyMultiBDT_get_weight_multiclass()
  {
    return {
      5.09811663,  8.74027068, 11.07907091, 10.87102167,  1.09818254, 36.9685032,
      14.5968635,  1.92228768,  2.47051653,  2.26252078,  0.65508076,  2.229232,
      1.0823288,   1.75996132,  0.23804092,  9.67751662,  4.75482342,  2.30044847,
      2.79506629,  9.91884314,  1.10576635,  0.63304991,  0.2797203,   1.16530436,
      2.03488271,  0.25374325,  0.69646131,  0.48343818,  0.30569068,  0.5135227,
      2.90510798,  0.18207231,  1.09179819,  0.46709051,  5.47657648,  2.85129545,
      0.77661606,  0.40344227,  0.40326453,  0.18133248,  0.17405083,  0.11171737,
      0.20540754,  0.24098085,  0.62998928
    };
  }

  std::vector<std::string>
  HyyHHbbyyMVAAlg::HyyMultiBDT_get_multiclass_names()
  {
    return {
      /* 0  */ "GG2H_0J_PTH_0_10", "GG2H_0J_PTH_GT10",
      /* 2  */ "GG2H_1J_PTH_0_60", "GG2H_1J_PTH_60_120", "GG2H_1J_PTH_120_200",
      /* 5  */ "GG2H_GE2J_MJJ_0_350_PTH_0_60", "GG2H_GE2J_MJJ_0_350_PTH_60_120", "GG2H_GE2J_MJJ_0_350_PTH_120_200",
      /* 8  */ "GG2H_GE2J_MJJ_350_700_PTH_0_200", "GG2H_GE2J_MJJ_700_1000_PTH_0_200", "GG2H_GE2J_MJJ_GT1000_PTH_0_200",
      /* 11 */ "GG2H_PTH_200_300", "GG2H_PTH_300_450", "GG2H_PTH_450_650", "GG2H_PTH_GT650",
      /* 15 */ "QQ2HQQ_0J", "QQ2HQQ_1J",
      /* 17 */ "QQ2HQQ_GE2J_MJJ_0_60", "QQ2HQQ_GE2J_MJJ_60_120", "QQ2HQQ_GE2J_MJJ_120_350",
      /* 20 */ "QQ2HQQ_GE2J_MJJ_350_700_PTH_0_200", "QQ2HQQ_GE2J_MJJ_700_1000_PTH_0_200", "QQ2HQQ_GE2J_MJJ_GT1000_PTH_0_200",
      /* 23 */ "QQ2HQQ_GE2J_MJJ_350_700_PTH_GT200", "QQ2HQQ_GE2J_MJJ_700_1000_PTH_GT200", "QQ2HQQ_GE2J_MJJ_GT1000_PTH_GT200",
      /* 26 */ "QQ2HLNU_PTV_0_75", "QQ2HLNU_PTV_75_150", "QQ2HLNU_PTV_150_250", "QQ2HLNU_PTV_GT250",
      /* 30 */ "QQ2HLL_PTV_0_75", "QQ2HLL_PTV_75_150", "QQ2HLL_PTV_150_250", "QQ2HLL_PTV_GT250",
      /* 34 */ "QQ2HNUNU_PTV_0_75", "QQ2HNUNU_PTV_75_150", "QQ2HNUNU_PTV_150_250", "QQ2HNUNU_PTV_GT250",
      /* 38 */ "TTH_PTH_0_60", "TTH_PTH_60_120", "TTH_PTH_120_200", "TTH_PTH_200_300", "TTH_PTH_GT300",
      /* 43 */ "THJB", "THW"
    };
  }

  void HyyHHbbyyMVAAlg::HyyMultiBDT_init(const std::string &folder)
  {
    std::string resolvedFolder = PathResolverFindCalibDirectory(folder);

    const std::string filename_multiclass = resolvedFolder + "/model1_HyymulticlassMVA.root";
    TFile *file_multiclass = TFile::Open(filename_multiclass.c_str());
    TTree *tree_multiclass = nullptr;

    file_multiclass->GetObject("lgbm", tree_multiclass);
    HyyMultiBDT_multiclass = std::make_unique<MVAUtils::BDT>(tree_multiclass);
    file_multiclass->Close();

    const std::vector<std::string> names = HyyMultiBDT_get_multiclass_names();

    for (const std::string &suffix : names) {
      const std::string filename_binary = resolvedFolder + "/model2_" + suffix + ".root";
      TFile *file_binary = TFile::Open(filename_binary.c_str());
      TTree *tree_binary = nullptr;
      file_binary->GetObject("lgbm", tree_binary);
      std::unique_ptr<MVAUtils::BDT> binary_model =
        std::make_unique<MVAUtils::BDT>(tree_binary);
      HyyMultiBDT_binaries.push_back(std::move(binary_model));
      file_binary->Close();
    }

    int offset = 0;
    HyyMultiBDT_boundaries   = HyyMultiBDT_get_boundaries_multiclass();
    HyyMultiBDT_num_classes  = HyyMultiBDT_boundaries.size();

    for (const auto &b : HyyMultiBDT_boundaries) {
      HyyMultiBDT_offset_cat.push_back(offset);
      offset += b.size() + 1;
    }

    HyyMultiBDT_weight_multiclass = HyyMultiBDT_get_weight_multiclass();
  }

  template<typename T>
  T HyyHHbbyyMVAAlg::var_or_nan(T value, T default_value)
  {
    return (value == default_value)
      ? std::numeric_limits<T>::quiet_NaN()
      : value;
  }

  HyyMultiBDTResults
  HyyHHbbyyMVAAlg::HyyMultiBDT_perform(const xAOD::EventInfo *event,
                                       const auto &sys,
                                       const xAOD::MissingET *met,
                                       const int n_photons,
                                       const TLorentzVector &ph1,
                                       const TLorentzVector &ph2,
                                       int nCentralJets,
                                       int nJets,
                                       int nBJets)
  {
    const std::vector<std::vector<HyyBDTAllVariables>> vars_ids = {
      #include "HyyBDTInputVariables.def"
    };

    std::vector<float> multiBDT_Input =
      get_HyyBDTInput(vars_ids[0], event, sys, met,
                      n_photons, ph1, ph2,
                      nCentralJets, nJets, nBJets);

    const std::vector<float> raw_multi_response =
      HyyMultiBDT_multiclass->GetMultiResponse(multiBDT_Input, HyyMultiBDT_num_classes);

    std::vector<float> multi_response(raw_multi_response.size());
    for (std::size_t i = 0; i != multi_response.size(); ++i) {
      multi_response[i] = raw_multi_response[i] * HyyMultiBDT_weight_multiclass[i];
    }

    const int multiclass_index =
      std::distance(std::begin(multi_response),
                    std::max_element(std::begin(multi_response),
                                     std::end(multi_response)));

    int   binary_bin   = 0;
    float binary_score = 0;

    if (HyyMultiBDT_boundaries[multiclass_index].size() != 0) {

      std::vector<float> BinaryBDT_Input =
        get_HyyBDTInput(vars_ids[multiclass_index + 1], event, sys, met,
                        n_photons, ph1, ph2,
                        nCentralJets, nJets, nBJets);

      binary_score =
        HyyMultiBDT_binaries[multiclass_index]->GetClassification(BinaryBDT_Input);

      const auto boundaries = HyyMultiBDT_boundaries[multiclass_index];

      // remember that pure bins (higher value) have lower index.
      // Extremes (0, 1) and not inside boundaries
      binary_bin =
        std::distance(std::upper_bound(std::begin(boundaries),
                                       std::end(boundaries),
                                       binary_score),
                      std::end(boundaries));
    }

    return HHBBYY::HyyMultiBDTResults(
      { multi_response,
        multiclass_index,
        binary_bin + HyyMultiBDT_offset_cat[multiclass_index],
        binary_score }
    );
  }

  std::vector<float>
  HyyHHbbyyMVAAlg::get_HyyBDTInput(const std::vector<HyyBDTAllVariables> &var_ids,
                                   const xAOD::EventInfo *event,
                                   const auto &sys,
                                   const xAOD::MissingET *met,
                                   const int n_photons,
                                   const TLorentzVector &ph1,
                                   const TLorentzVector &ph2,
                                   const int &nCentralJets,
                                   const int &nJets,
                                   const int &nBJets)
  {
    TLorentzVector H_yy(0., 0., 0., 0.);

    float pT_yy     (-99.);
    float yAbs_yy   (-99.);
    float ph0_eta   (-99.);
    float ph1_eta   (-99.);
    float Dy_y_y    (-99.);
    float pTt_yy    (-99.);
    float phiStar_yy(-99.);

    if (n_photons == 1) {

      ph0_eta = m_Fbranches_BDTInput.at("Photon1_eta").get(*event, sys);

    } else if (n_photons > 1) {

      H_yy   = ph1 + ph2;
      pT_yy  = H_yy.Pt();
      yAbs_yy = std::fabs(H_yy.Rapidity());

      ph0_eta = m_Fbranches_BDTInput.at("Photon1_eta").get(*event, sys);
      ph1_eta = m_Fbranches_BDTInput.at("Photon2_eta").get(*event, sys);

      Dy_y_y = std::fabs(ph1.Rapidity() - ph2.Rapidity());

      pTt_yy = std::fabs(ph1.Px() * ph2.Py() - ph2.Px() * ph1.Py())
               / (ph1 - ph2).Pt() * 2.0;

      phiStar_yy =
        tan((TMath::Pi() - std::fabs(ph1.DeltaPhi(ph2))) / 2.0) *
        std::sqrt(1.0 - std::pow(std::tanh((ph1.Eta() - ph2.Eta()) / 2.0), 2.0));
    }

    std::vector<float> result;
    result.reserve(var_ids.size());

    for (const auto var_id : var_ids) {
      switch (var_id) {

        case HyyBDTAllVariables::pT_yy:
          result.push_back(pT_yy);
          break;

        case HyyBDTAllVariables::yAbs_yy:
          result.push_back(yAbs_yy);
          break;

        case HyyBDTAllVariables::ph0_eta:
          result.push_back(ph0_eta);
          break;

        case HyyBDTAllVariables::ph1_eta:
          result.push_back(ph1_eta);
          break;

        case HyyBDTAllVariables::Dy_y_y:
          result.push_back(Dy_y_y);
          break;

        case HyyBDTAllVariables::pTt_yy:
          result.push_back(pTt_yy);
          break;

        case HyyBDTAllVariables::phiStar_yy:
          result.push_back(phiStar_yy);
          break;

        case HyyBDTAllVariables::N_j_central:
          result.push_back(nCentralJets);
          break;

        case HyyBDTAllVariables::N_j:
          result.push_back(nJets);
          break;

        case HyyBDTAllVariables::N_j_btag:
          result.push_back(std::min(nBJets, 3));
          break;

        case HyyBDTAllVariables::m_jj:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("m_jj").get(*event, sys), -99));
          break;

        case HyyBDTAllVariables::pT_yyjj:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("pT_yyjj").get(*event, sys), -99));
          break;

        case HyyBDTAllVariables::N_j_central30:
          result.push_back(
            m_Ibranches_BDTInput.at("nCentralJets_30").get(*event, sys));
          break;

        case HyyBDTAllVariables::N_j_30:
          result.push_back(
            m_Ibranches_BDTInput.at("nJets_30").get(*event, sys));
          break;

        case HyyBDTAllVariables::N_j_btag30:
          result.push_back(
            std::min(m_Ibranches_BDTInput.at("nBJets_30").get(*event, sys), 3));
          break;

        case HyyBDTAllVariables::m_jj_30:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("m_jj_30").get(*event, sys), -99));
          break;

        case HyyBDTAllVariables::pT_yyjj_30:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("pT_yyjj_30").get(*event, sys), -99));
          break;

        case HyyBDTAllVariables::pT_jj:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("pT_jj").get(*event, sys), -99));
          break;

        case HyyBDTAllVariables::pT_j1_30:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("pT_j1_30").get(*event, sys), -99));
          break;

        case HyyBDTAllVariables::Dy_j_j:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("Dy_j_j").get(*event, sys), -99));
          break;

        case HyyBDTAllVariables::Dphi_j_j:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("Dphi_j_j").get(*event, sys), -99));
          break;

        case HyyBDTAllVariables::Deta_j_j:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("Deta_j_j").get(*event, sys), -99));
          break;

        case HyyBDTAllVariables::pT_yyj:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("pT_yyj").get(*event, sys), -99));
          break;

        case HyyBDTAllVariables::m_yyj:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("m_yyj").get(*event, sys), -99));
          break;

        case HyyBDTAllVariables::m_yyjj:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("m_yyjj").get(*event, sys), -99));
          break;

        case HyyBDTAllVariables::Dphi_yy_jj:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("Dphi_yy_jj").get(*event, sys), -99));
          break;

        case HyyBDTAllVariables::DRmin_y_j:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("DRmin_y_j").get(*event, sys), -99));
          break;

        case HyyBDTAllVariables::Dy_yy_jj:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("Dy_yy_jj").get(*event, sys), -99));
          break;

        case HyyBDTAllVariables::m_alljet:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("m_alljets").get(*event, sys), -99));
          break;

        case HyyBDTAllVariables::cosTS_yyjj:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("cosTS_yyjj").get(*event, sys), -99));
          break;

        case HyyBDTAllVariables::N_lep:
          result.push_back(
            m_Ibranches_BDTInput.at("nLeptons").get(*event, sys));
          break;

        case HyyBDTAllVariables::N_e:
          result.push_back(
            m_Ibranches_BDTInput.at("nElectrons").get(*event, sys));
          break;

        case HyyBDTAllVariables::N_mu:
          result.push_back(
            m_Ibranches_BDTInput.at("nMuons").get(*event, sys));
          break;

        case HyyBDTAllVariables::m_ee:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("m_ee_os").get(*event, sys), -99));
          break;

        case HyyBDTAllVariables::m_mumu:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("m_mumu_os").get(*event, sys), -99));
          break;

        case HyyBDTAllVariables::met_TST:
          result.push_back(
            var_or_nan<float>(met->met(), -99));
          break;

        case HyyBDTAllVariables::sumet_TST:
          result.push_back(
            var_or_nan<float>(met->sumet(), -99));
          break;

        case HyyBDTAllVariables::MET_sig:
          result.push_back(
            var_or_nan<float>(m_met_sig.get(*met, sys), -99));
          break;

        case HyyBDTAllVariables::phi_TST:
          result.push_back(
            var_or_nan<float>(met->phi(), -99));
          break;

        case HyyBDTAllVariables::pT_ll:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("pT_ll_os_max").get(*event, sys), -99));
          break;

        case HyyBDTAllVariables::pTlepMET:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("pTlepMET").get(*event, sys), -99));
          break;

        case HyyBDTAllVariables::mTlepMET:
          result.push_back(
            m_Fbranches_BDTInput.at("mTlepMET").get(*event, sys));  // $ 0 to follow same bug in training
          break;

        case HyyBDTAllVariables::flag_eTmiss:
          result.push_back(
            m_Fbranches_BDTInput.at("metTST_HVTST_flag").get(*event, sys));
          break;

        case HyyBDTAllVariables::HT_30:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("HT_30").get(*event, sys), 0.));
          break;

        case HyyBDTAllVariables::Zepp:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("Zepp").get(*event, sys), -99));
          break;

        case HyyBDTAllVariables::mu:
          // This affects only data, since mu and actualMu are the same on MC
          result.push_back(event->actualInteractionsPerCrossing());
          break;

        case HyyBDTAllVariables::score_recotop:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("score_recotop1").get(*event, sys), -1.));
          break;

        case HyyBDTAllVariables::recotop_m:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("recotop1_m").get(*event, sys), 0.));
          break;

        case HyyBDTAllVariables::recotop_phi:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("recotop1_phi").get(*event, sys), 0.));
          break;

        case HyyBDTAllVariables::recotop_eta:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("recotop1_eta").get(*event, sys), 0.));
          break;

        case HyyBDTAllVariables::recotop_pT:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("recotop1_pT").get(*event, sys), 0.));
          break;

        case HyyBDTAllVariables::score_hybrtop:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("score_recotop2").get(*event, sys), -1.));
          break;

        case HyyBDTAllVariables::hybrtop_m:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("hybrtop_m").get(*event, sys), 0.));
          break;

        case HyyBDTAllVariables::hybrtop_phi:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("hybrtop_phi").get(*event, sys), 0.));
          break;

        case HyyBDTAllVariables::hybrtop_eta:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("hybrtop_eta").get(*event, sys), 0.));
          break;

        case HyyBDTAllVariables::hybrtop_pT:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("hybrtop_pT").get(*event, sys), 0.));
          break;

        case HyyBDTAllVariables::fwdjet_eta:
          result.push_back(
            std::fabs(var_or_nan<float>(m_Fbranches_BDTInput.at("fwdJet_eta")
                                          .get(*event, sys),
                                        -99.)));
          break;

        case HyyBDTAllVariables::m_fwdJyy:
          result.push_back(
            var_or_nan<float>(m_Fbranches_BDTInput.at("m_fwdJet_yy")
                                .get(*event, sys),
                              -99));
          break;

        case HyyBDTAllVariables::deltaR_Wb_t2:
          result.push_back(
            m_Fbranches_BDTInput.at("dR_Wb_t2").get(*event, sys));
          break;
      }
    }

    return result;
  }

} // namespace HHBBYY

