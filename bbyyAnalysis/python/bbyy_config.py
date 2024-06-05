from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from EasyjetHub.algs.postprocessing.trigger_matching import TriggerMatchingToolCfg


from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)

import pathlib
import os
import re


def bbyy_cfg(flags, smalljetkey, photonkey, muonkey, electronkey,
             float_variables=None, int_variables=None):
    if not float_variables:
        float_variables = []
    if not int_variables:
        int_variables = []

    cfg = ComponentAccumulator()

    PhotonWPLabel = f'{flags.Analysis.Photon.ID}_{flags.Analysis.Photon.Iso}'
    TightPhotonWP = flags.Analysis.Photon.extra_wps[0]
    TightPhotonWPLabel = f'{TightPhotonWP[0]}_{TightPhotonWP[1]}'
    cfg.addEventAlgo(
        CompFactory.Easyjet.PhotonSelectorAlg(
            "PhotonSelectorAlg",
            containerInKey=PhotonWPLabel + photonkey,
            containerOutKey="bbyyAnalysisPhotons_%SYS%",
            photon_WPs=[f'{wp[0]}_{wp[1]}' for wp in
                        flags.Analysis.Photon.extra_wps],
            isMC=flags.Input.isMC,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    MuonWPLabel = f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}'
    cfg.addEventAlgo(
        CompFactory.Easyjet.MuonSelectorAlg(
            "MuonSelectorAlg",
            containerInKey=MuonWPLabel + muonkey,
            containerOutKey="bbyyAnalysisMuons_%SYS%",
            minPt=10e3,
            muon_WP=MuonWPLabel,
            isMC=flags.Input.isMC,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    ElectronWPLabel = f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}'
    cfg.addEventAlgo(
        CompFactory.Easyjet.ElectronSelectorAlg(
            "ElectronSelectorAlg",
            containerInKey=ElectronWPLabel + electronkey,
            containerOutKey="bbyyAnalysisElectrons_%SYS%",
            minPt=10e3,
            ele_WP=ElectronWPLabel,
            isMC=flags.Input.isMC,
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    cfg.addEventAlgo(
        CompFactory.Easyjet.JetSelectorAlg(
            "JetSelectorAlg",
            containerInKey=smalljetkey,
            containerOutKey="bbyyAnalysisJets_%SYS%",
            PCBTDecorName="ftag_quantile_" + flags.Analysis.small_R_jet.btag_extra_wps[0], # noqa
            pTsort=False,
            PCBTsort=True,
            bTagWPDecorName="",
            checkOR=flags.Analysis.do_overlap_removal,
        )
    )

    selection_name = flags.Analysis.selection_name

    cfg.addEventAlgo(
        CompFactory.HHBBYY.bbyySelectorAlg(
            "bbyySelectorAlg",
            photons="bbyyAnalysisPhotons_%SYS%",
            photonWP=TightPhotonWPLabel,
            jets="bbyyAnalysisJets_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R_jet.btag_wp,
            muons="bbyyAnalysisMuons_%SYS%",
            electrons="bbyyAnalysisElectrons_%SYS%",
            cutList=flags.Analysis.CutList,
            saveCutFlow=flags.Analysis.save_bbyy_cutflow,
            photonTriggers=flags.Analysis.TriggerChains,
            trigMatchingTool=cfg.popToolsAndMerge(TriggerMatchingToolCfg(flags)),
            eventDecisionOutputDecoration=f"bbyy_pass_{selection_name}_%SYS%",
            isMC=flags.Input.isMC,
            bypass=flags.Analysis.bypass,
            enableSinglePhotonTrigger=flags.Analysis.enable_single_photon_trigger,
            weightIndex=get_weight_index(flags),
        )
    )

    cfg.addEventAlgo(
        CompFactory.HHBBYY.BaselineVarsbbyyAlg(
            "BaselineVarsbbyyAlg",
            photons="bbyyAnalysisPhotons_%SYS%",
            photonWP=TightPhotonWPLabel,
            muons="bbyyAnalysisMuons_%SYS%",
            electrons="bbyyAnalysisElectrons_%SYS%",
            jets="bbyyAnalysisJets_%SYS%",
            met="AnalysisMET_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.small_R_jet.btag_wp,
            PCBTDecorName="ftag_quantile_" + flags.Analysis.small_R_jet.btag_extra_wps[0],  # noqa
            BDT_path=flags.Analysis.BDT_path,
            isMC=flags.Input.isMC,
            floatVariableList=float_variables,
            intVariableList=int_variables
        )
    )

    if flags.Analysis.do_resonant_PNN:
        float_SH_var = [var for var in float_variables if "SH_" in var]
        cfg.addEventAlgo(
            CompFactory.SHBBYY.ResonantPNNbbyyAlg(
                "ResonantPNNbbyyAlg",
                photons="bbyyAnalysisPhotons_%SYS%",
                jets="bbyyAnalysisJets_%SYS%",
                bTagWPDecorName="ftag_select_" + flags.Analysis.small_R_jet.btag_wp,
                mX_mS_pairs=flags.Analysis.mX_mS_pairs,
                mS_values=flags.Analysis.mS_values,
                mX_values=flags.Analysis.mX_values,
                mX_1bjet=flags.Analysis.mX_1bjet,
                floatVariableList=float_SH_var
            )
        )

    return cfg


def bbyy_filter_dalitz_cfg(flags):
    cfg = ComponentAccumulator()

    cfg.addEventAlgo(
        CompFactory.HHBBYY.bbyyFilterDalitzAlg(
            "bbyyFilterDalitzAlg",
            TruthParticleSMInKey=(
                flags.Analysis.container_names.input.truthSMParticles
            ),
            TruthParticleBSMInKey=(
                flags.Analysis.container_names.input.truthBSMParticles
            ),
        ),
    )

    return cfg


def get_BaselineVarsbbyyAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    # Number of objects
    int_variable_names += ["nPhotons", "nJets", "nCentralJets", "nBJets", "nLeptons",
                           "bdtSel_category"]

    # Reconstructed Higgses
    float_variable_names += ["myy", "pTyy", "Etayy", "Phiyy", "dRyy"]
    float_variable_names += ["mbb", "pTbb", "Etabb", "Phibb", "dRbb"]

    float_variable_names += ["cos_theta_yy_cm_bbyy","phi_yy_cm_bbyy"]

    float_variable_names += ["Photon1_cos_theta_cm_gamgam","Photon1_phi_cm_gamgam"]

    # HbbCandidate jets
    for i in range(1,3):
        for var in ["pt", "phi", "eta", "E","uncorrPt","muonCorrPt"]:
            float_variable_names += [f"HbbCandidate_Jet{i}_" + var]
        for var in ["truthLabel", "pcbt","n_muons"]:
            int_variable_names += [f"HbbCandidate_Jet{i}_" + var]

    float_variable_names += ["HbbCandidate_Jet1_cos_theta_cm_bb",
                             "HbbCandidate_Jet1_phi_cm_bb"]

    # di-higgs variables
    float_variable_names += ["mbbyy", "mbbyy_star", "pTbbyy", "Etabbyy",
                             "Phibbyy", "dRbbyy", "bdtSel_score"]

    float_variable_names += ["DeltaPhi_bb_yy_cm_bbyy"]

    # VBFJets
    for i in range(1,3):
        for var in ["pt", "eta", "phi", "E", "yybb_dR", "yybb_deta"]:
            float_variable_names += [f"Jet_vbf_j{i}_" + var]
    for var in ["maxscore", "m", "deta", "yybb_dR", "yybb_deta", "yybb_pt",
                "yybb_eta", "yybb_phi", "yybb_m"]:
        float_variable_names += ["Jet_vbf_jj_" + var]

    # mva variables
    float_variable_names += ["HT", "topness", "sphericityT", "planarFlow",
                             "pTBalance"]

    return float_variable_names, int_variable_names


def get_BaselineVarsbbyyAlg_highlevelvariables(flags):
    high_level_float_variables = []
    high_level_int_variables = []

    return high_level_float_variables, high_level_int_variables


def get_BaselineVarsbbyyAlg_SH(flags):
    SH_float_variable_names = []
    SH_int_variable_names = []

    for pair in flags.Analysis.mX_mS_pairs:
        m_X = pair[0]
        m_S = pair[1]
        SH_float_variable_names += [f"SH_PNN_Score_X{m_X}_S{m_S}"]

    for m_X in flags.Analysis.mX_values:
        for m_S in flags.Analysis.mS_values:
            if m_X > 500 and m_S < 70:
                continue
            if m_X - m_S <= 125:
                continue
            SH_float_variable_names += [f"SH_PNN_Score_X{m_X}_S{m_S}"]

    for m_X in flags.Analysis.mX_1bjet:
        SH_float_variable_names += [f"SH_PNN_Score_1bjet_X{m_X}"]

    return SH_float_variable_names, SH_int_variable_names


def bbyy_branches(flags):
    branches = []

    # this will be all the variables that are calculated by the
    # BaselineVarsbbllAlg algorithm
    all_baseline_variable_names = []
    float_variable_names = []
    int_variable_names = []

    # these are the variables that will always be stored by easyjet specific to HHbbyy
    baseline_float_variables, baseline_int_variables \
        = get_BaselineVarsbbyyAlg_variables(flags)
    float_variable_names += baseline_float_variables
    int_variable_names += baseline_int_variables

    # Here are more high level variables which can be stored using the flag
    # flags.Analysis.store_high_level_variables
    if flags.Analysis.store_high_level_variables:
        high_level_float_variables, high_level_int_variables \
            = get_BaselineVarsbbyyAlg_highlevelvariables(flags)
        float_variable_names += high_level_float_variables
        int_variable_names += high_level_int_variables

    # Here are special variables to be stored for the SH resonant search
    # flags.Analysis.do_resonant_PNN
    if flags.Analysis.do_resonant_PNN:
        SH_float_variables, SH_int_variables \
            = get_BaselineVarsbbyyAlg_SH(flags)
        float_variable_names += SH_float_variables
        int_variable_names += SH_int_variables

    all_baseline_variable_names += [*float_variable_names, *int_variable_names]

    for var in all_baseline_variable_names:
        branches += [f"EventInfo.{var}_%SYS% -> bbyy_{var}"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    object_level_branches, object_level_float_variables, object_level_int_variables \
        = get_selected_objects_branches_variables(flags, "bbyy")
    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    branches += object_level_branches

    # More event info variables:
    s_name = flags.Analysis.selection_name
    branches += \
        [f"EventInfo.bbyy_pass_{s_name}_%SYS% -> bbyy_pass_{s_name}"
         + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    if (flags.Analysis.save_bbyy_cutflow):
        cutList = flags.Analysis.CutList
        for cut in cutList:
            if cut == "PASS_TRIGGER" or cut == "PASS_TRIGGER_MATCHING":
                extra = "SINGLE_OR_DIPHOTON"
                branches += [f"EventInfo.{cut}_%SYS% -> bbyy_{cut}_{extra}"
                             + flags.Analysis.systematics_suffix_separator + "%SYS%"]
            else:
                branches += [f"EventInfo.{cut}_%SYS% -> bbyy_{cut}"
                             + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    branches += ["EventInfo.eventWeight -> eventWeight"]

    photon_triggers = [
        "pass_trigger_single_photon",
        "pass_trigger_diphoton",
        "pass_matching_trigger_single_photon",
        "pass_matching_trigger_diphoton"]

    for trigger in photon_triggers:
        branches += [f"EventInfo.{trigger}_%SYS% -> {trigger}"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    return branches, float_variable_names, int_variable_names


def FullPath(rawpath):
    fpath = pathlib.Path(rawpath)
    for dirpath in [""] + os.environ["DATAPATH"].split(":"):
        fullpath = dirpath / fpath
        if fullpath.exists():
            return fullpath


def contain_dalitz(flags):
    dsid = str(flags.Input.MCChannelNumber)
    # file name hard-coded
    with open(FullPath("bbyyAnalysis/DalitzDataset.txt"), 'r') as file_in:
        dataset_list = file_in.readlines()
        for dataset in dataset_list:
            if dsid in dataset:
                return True
    return False


def get_weight_index(flags):
    dsid = str(flags.Input.MCChannelNumber)
    # file name hard-coded
    with open(FullPath("bbyyAnalysis/SpecialWeightIndices.yaml"), 'r') as file_in:
        pattern = r"DSID:\s*(\d+)\s*\n\s*weightIndex:\s*(\d+)"
        content = file_in.read()
        matches = re.findall(pattern, content)

        for match_dsid, weight_index in matches:
            if match_dsid == dsid:
                return int(weight_index)

    return 0  # in case no matching DSID is found, set the nominal weight index to 0.
