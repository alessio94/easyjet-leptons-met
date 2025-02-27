from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod

from EasyjetHub.algs.cpalgs_config import get_sys_weight_name
from EasyjetHub.algs.postprocessing.trigger_matching import TriggerMatchingToolCfg
from EasyjetHub.algs.postprocessing.SelectorAlgConfig import (
    PhotonSelectorAlgCfg, MuonSelectorAlgCfg, ElectronSelectorAlgCfg,
    JetSelectorAlgCfg)
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_photon_branches_variables,
    get_selected_jet_branches_variables,
)
import AthenaCommon.SystemOfUnits as Units
from itertools import chain


def bbyy_cfg(flags, smalljetkey, photonkey, muonkey, electronkey, largeRjetkey,
             float_variables=None, int_variables=None):
    keys = ["baseline", "photons", "leptons"]
    if not float_variables:
        float_variables = {key: [] for key in keys}
    if not int_variables:
        int_variables = {key: [] for key in keys}

    cfg = ComponentAccumulator()
    cfg.merge(PhotonSelectorAlgCfg(flags,
                                   containerInKey=photonkey,
                                   containerOutKey="bbyyAnalysisPhotons_%SYS%",
                                   minPt=22. * Units.GeV))

    cfg.merge(MuonSelectorAlgCfg(flags,
                                 containerInKey=muonkey,
                                 containerOutKey="bbyyAnalysisMuons_%SYS%",
                                 minPt=10 * Units.GeV))

    cfg.merge(ElectronSelectorAlgCfg(flags,
                                     containerInKey=electronkey,
                                     containerOutKey="bbyyAnalysisElectrons_%SYS%",
                                     minPt=10. * Units.GeV))

    cfg.merge(JetSelectorAlgCfg(
        flags,
        containerInKey=smalljetkey,
        containerOutKey="bbyyAnalysisJets_%SYS%",
        PCBTDecorName="ftag_quantile_" + flags.Analysis.Small_R_jet.btag_extra_wps[0], # noqa
        minPt=float(flags.Analysis.Small_R_jet.min_pT),
        pTsort=False,
        PCBTsort=True,
        bTagWPDecorName="",
        selectBjet=False))

    if flags.Analysis.do_Boosted:
        cfg.merge(JetSelectorAlgCfg(
            flags,
            name="LargeRJetSelectorAlg",
            containerInKey=largeRjetkey,
            containerOutKey="bbyyAnalysisLargeRJets_%SYS%",
            minPt=250. * Units.GeV,
            maxEta=2.0,
            pTsort=True,
            PCBTsort=False,
            bTagWPDecorName="",
            selectBjet=False))

    selection_name = flags.Analysis.selection_name

    # First extra wp is used to select photons
    SelectedPhotonWP = flags.Analysis.Photon.extra_wps[0]
    SelectedPhotonLabel = f'{SelectedPhotonWP[0]}_{SelectedPhotonWP[1]}'
    cfg.addEventAlgo(
        CompFactory.HHBBYY.bbyySelectorAlg(
            "bbyySelectorAlg",
            photonWP=SelectedPhotonLabel,
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
            cutList=flags.Analysis.CutList,
            saveCutFlow=flags.Analysis.save_cutflow,
            photonTriggers=flags.Analysis.TriggerChains,
            trigMatchingTool=cfg.popToolsAndMerge(TriggerMatchingToolCfg(flags)),
            eventDecisionOutputDecoration=f"bbyy_pass_{selection_name}_%SYS%",
            isMC=flags.Input.isMC,
            bypass=flags.Analysis.bypass,
            enableSinglePhotonTrigger=flags.Analysis.enable_single_photon_trigger,
            specialSysWeight=get_sys_weight_name(flags),
            saveTriggerInfo=flags.Analysis.save_pass_trigger_info,
        )
    )

    if flags.Analysis.do_KinematicFit:
        cfg.addEventAlgo(
            CompFactory.HHBBYY.MbbKinFitDecoratorAlg(
                "MbbKinFitDecoratorAlg",
                KinFitTool=CompFactory.KinematicFitTool(
                    JetMinPt=25. * Units.GeV,
                    bTagWPDecorName=(
                        "ftag_select_" + flags.Analysis.Small_R_jet.btag_wp),
                    isRun3=flags.GeoModel.Run is LHCPeriod.Run3),
                doSystematics=flags.Analysis.do_CP_systematics,
            )
        )

    cfg.addEventAlgo(
        CompFactory.HHBBYY.LeptonVarsbbyyAlg(
            "LeptonVarsbbyyAlg",
            intVariableList=int_variables['leptons'],
            doSystematics=flags.Analysis.do_CP_systematics,
        )
    )

    cfg.addEventAlgo(
        CompFactory.HHBBYY.PhotonVarsbbyyAlg(
            "PhotonVarsbbyyAlg",
            photons=photonkey,
            photonWP=SelectedPhotonLabel,
            saveDummyPhotonSF=flags.GeoModel.Run is LHCPeriod.Run2,
            floatVariableList=float_variables['photons'],
            intVariableList=int_variables['photons'],
            doSystematics=flags.Analysis.do_CP_systematics,
            isMC=flags.Input.isMC,
        )
    )

    cfg.addEventAlgo(
        CompFactory.HHBBYY.BaselineVarsbbyyAlg(
            "BaselineVarsbbyyAlg",
            KFJets="bbyyAnalysisKFJets_%SYS%" if flags.Analysis.do_KinematicFit else "",
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
            PCBTDecorName="ftag_quantile_" + flags.Analysis.Small_R_jet.btag_extra_wps[0],  # noqa
            do_nonresonant_BDTs=flags.Analysis.do_nonresonant_BDTs,
            BDT_path=flags.Analysis.BDT_path,
            doGNN_tagging=flags.Analysis.do_GNN2bjetSelection,
            GNN_path=(flags.Analysis.GNN_path_run2 if flags.Analysis.Run == 2 else
                      flags.Analysis.GNN_path_run3),
            VBFjetsMethod=flags.Analysis.VBFjetsMethod,
            save_VBF_vars=flags.Analysis.save_VBF_vars,
            isMC=flags.Input.isMC,
            doKF=flags.Analysis.do_KinematicFit,
            floatVariableList=float_variables['baseline'],
            intVariableList=int_variables['baseline'],
            doSystematics=flags.Analysis.do_CP_systematics,
            doResonantonebtag=flags.Analysis.do_resonant_onebtag,
            save_HbbCand_vars=flags.Analysis.save_HbbCand_vars,
            save_extra_vars=flags.Analysis.save_extra_vars,
            save_nonresonant_BDTInput_variables=(
                flags.Analysis.save_nonresonant_BDTInput_variables)
        )
    )

    if flags.Analysis.do_Boosted:
        cfg.addEventAlgo(
            CompFactory.HHBBYY.BoostedVarsbbyyAlg(
                "BoostedVarsbbyyAlg",
                isMC=flags.Input.isMC,
                floatVariableList=float_variables,
                intVariableList=int_variables,
                doSystematics=flags.Analysis.do_CP_systematics,
            )
        )

    if flags.Analysis.do_resonant_PNN:
        float_SH_var = [var for var in float_variables if "SH_" in var]
        cfg.addEventAlgo(
            CompFactory.SHBBYY.ResonantPNNbbyyAlg(
                "ResonantPNNbbyyAlg",
                bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
                mX_mS_pairs=flags.Analysis.mX_mS_pairs,
                mS_values=flags.Analysis.mS_values,
                mX_values=flags.Analysis.mX_values,
                mX_1bjet=flags.Analysis.mX_1bjet,
                floatVariableList=float_SH_var,
            )
        )

    return cfg


def get_PhotonVarsbbyyAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    # Photon variables
    int_variable_names += ["nPhotons"]
    float_variable_names += ["myy", "pTyy", "Etayy", "Phiyy", "dRyy"]

    return float_variable_names, int_variable_names


def get_LeptonVarsbbyyAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    # Lepton variables
    int_variable_names += ["nLeptons"]

    return float_variable_names, int_variable_names


def get_BaselineVarsbbyyAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    # Number of objects
    int_variable_names += ["nJets", "nCentralJets", "nBJets"]
    if flags.Analysis.do_nonresonant_BDTs:
        int_variable_names += ["bdtSel_category"]

    # Reconstructed Higgses
    float_variable_names += ["mbb", "pTbb", "Etabb", "Phibb", "dRbb"]

    if (flags.Analysis.save_extra_vars):
        float_variable_names += ["cos_theta_yy_cm_bbyy", "phi_yy_cm_bbyy"]

        float_variable_names += ["Photon1_cos_theta_cm_gamgam", "Photon1_phi_cm_gamgam"]

    # HbbCandidate jets
    if (flags.Analysis.save_HbbCand_vars):
        HbbCandidate_float_vars = ["pt", "phi", "eta", "E"]
        if (flags.Analysis.save_extra_vars):
            HbbCandidate_float_vars += ["uncorrPt", "muonCorrPt"]
        for i in range(1, 3):
            for var in HbbCandidate_float_vars:
                float_variable_names += [f"HbbCandidate_Jet{i}_" + var]
            for var in ["truthLabel", "pcbt", "n_muons"]:
                int_variable_names += [f"HbbCandidate_Jet{i}_" + var]

    if (flags.Analysis.save_extra_vars):
        float_variable_names += ["HbbCandidate_Jet1_cos_theta_cm_bb",
                                 "HbbCandidate_Jet1_phi_cm_bb"]

    # di-higgs variables
    float_variable_names += ["mbbyy", "mbbyy_star", "pTbbyy", "Etabbyy",
                             "Phibbyy", "dRbbyy"]
    if flags.Analysis.do_nonresonant_BDTs:
        float_variable_names += ["bdtSel_score"]

    if (flags.Analysis.save_extra_vars):
        float_variable_names += ["DeltaPhi_bb_yy_cm_bbyy"]

    # three body mass used for
    # SHbbyy, onebtag region
    if flags.Analysis.do_resonant_onebtag:
        float_variable_names += ["mbyy"]

    # Kinematic Fit variables
    if flags.Analysis.do_KinematicFit:
        float_variable_names += ["KF_pTbb", "KF_Etabb", "KF_Phibb",
                                 "KF_mbbyy", "KF_dRbb", "KF_mbbyy_star", "KF_pTbbyy",
                                 "KF_Etabbyy", "KF_Phibbyy", "KF_dRHH"]
        # KF mva variables
        float_variable_names += ["KF_HT", "KF_topness", "KF_sphericityT",
                                 "KF_planarFlow", "KF_pTBalance"]

        float_variable_names += ["KF_bdtSel_score", "KF_corr_bdtSel_score",
                                 "KF_with2024_bdtSel_score",
                                 "KF_corr_with2024_bdtSel_score"]
        int_variable_names += ["KF_bdtSel_category", "KF_corr_bdtSel_category",
                               "KF_with2024_bdtSel_category",
                               "KF_corr_with2024_bdtSel_category"]

        for i in range(1, 5):
            for var in ["pt", "phi", "eta", "E"]:
                float_variable_names += [f"KF_Jet{i}_" + var]

        if flags.Analysis.save_HbbCand_vars:
            for i in range(1, 3):
                for var in ["pt", "phi", "eta", "E"]:
                    float_variable_names += [f"KF_HbbCandidate_Jet{i}_" + var]

    # VBFJets
    if flags.Analysis.save_VBF_vars:
        for i in range(1, 3):
            for var in ["pt", "eta", "phi", "E"]:
                float_variable_names += [f"Jet_vbf_j{i}_" + var]
                if (flags.Analysis.do_KinematicFit):
                    float_variable_names += [f"KF_Jet_vbf_j{i}_" + var]
        for var in ["m", "deta"]:
            float_variable_names += ["Jet_vbf_jj_" + var]
            if (flags.Analysis.do_KinematicFit):
                float_variable_names += ["KF_Jet_vbf_jj_" + var]
        if ("BDT" in flags.Analysis.VBFjetsMethod):
            float_variable_names += ["Jet_vbf_jj_maxscore"]
            if (flags.Analysis.do_KinematicFit):
                float_variable_names += ["KF_Jet_vbf_jj_maxscore"]

    # GNN HbbCandidate jets
    if (flags.Analysis.do_GNN2bjetSelection):
        for c in ["ggFTarget", "VBFTarget"]:
            GNN_float_list = list(flags.Analysis.Small_R_jet.variables_allJets)
            GNN_float_list.extend(["uncorrPt", "muonCorrPt"])
            GNN_int_list = list(flags.Analysis.Small_R_jet.variables_int_allJets)
            GNN_int_list.extend(["n_muons"])

            for i in range(1, 3):
                for v in GNN_float_list:
                    float_variable_names += ["GNN_" + c + f"_HbbCandidate_Jet{i}_" + v]
                for v in GNN_int_list:
                    int_variable_names += ["GNN_" + c + f"_HbbCandidate_Jet{i}_" + v]

            float_variable_names += ["GNN_" + c + "_mbb", "GNN_" + c + "_pTbb",
                                     "GNN_" + c + "_Etabb", "GNN_" + c + "_Phibb",
                                     "GNN_" + c + "_dRbb"]
            float_variable_names += ["GNN_" + c + "_cos_theta_yy_cm_bbyy",
                                     "GNN_" + c + "_phi_yy_cm_bbyy"]
            float_variable_names += ["GNN_" + c + "_HbbCandidate_Jet1_cos_theta_cm_bb",
                                     "GNN_" + c + "_HbbCandidate_Jet1_phi_cm_bb"]
            float_variable_names += ["GNN_" + c + "_DeltaPhi_bb_yy_cm_bbyy"]
            float_variable_names += ["GNN_" + c + "_Photon1_cos_theta_cm_gamgam",
                                     "GNN_" + c + "_Photon1_phi_cm_gamgam"]
            float_variable_names += ["GNN_" + c + "_mbbyy", "GNN_" + c + "_mbbyy_star",
                                     "GNN_" + c + "_pTbbyy", "GNN_" + c + "_Etabbyy",
                                     "GNN_" + c + "_Phibbyy", "GNN_" + c + "_dRbbyy",
                                     "GNN_" + c + "_bdtSel_score",
                                     "GNN_" + c + "_maxscore",
                                     "GNN_" + c + "_Jet_vbf_jj_maxscore"]
            float_variable_names += ["GNN_" + c + "_sphericityT",
                                     "GNN_" + c + "_planarFlow",
                                     "GNN_" + c + "_pTBalance"]
            float_variable_names += ["GNN_" + c + "_Jet_vbf_jj_m",
                                     "GNN_" + c + "_Jet_vbf_jj_deta"]
            int_variable_names += ["GNN_" + c + "_bdtSel_category"]

    # save extra variables which may be useful, but are
    # not necessary for barebones SHbbyy analysis
    if (flags.Analysis.save_nonresonant_BDTInput_variables):
        # mva variables
        float_variable_names += ["HT", "topness", "sphericityT", "planarFlow",
                                 "pTBalance"]

    return float_variable_names, int_variable_names


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


def get_BoostedVarsbbyyAlg(flags):
    boosted_float_variables = []
    boosted_int_variables = []

    boosted_int_variables += ["nLargeRJets"]

    return boosted_float_variables, boosted_int_variables


def bbyy_branches(flags):
    branches = []

    # this will be all the variables that are calculated by the
    # BaselineVarsbbllAlg algorithm
    all_baseline_variable_names = []
    keys = ["baseline", "photons", "leptons", "jets"]
    float_variable_names = {key: [] for key in keys}
    int_variable_names = {key: [] for key in keys}

    # these are the variables that will always be stored by easyjet specific to HHbbyy
    baseline_float_variables, baseline_int_variables \
        = get_BaselineVarsbbyyAlg_variables(flags)
    photon_float_variables, photon_int_variables \
        = get_PhotonVarsbbyyAlg_variables(flags)
    lepton_float_variables, lepton_int_variables \
        = get_LeptonVarsbbyyAlg_variables(flags)

    float_variable_names['baseline'] += baseline_float_variables
    int_variable_names['baseline'] += baseline_int_variables
    float_variable_names['photons'] += photon_float_variables
    int_variable_names['photons'] += photon_int_variables
    float_variable_names['leptons'] += lepton_float_variables
    int_variable_names['leptons'] += lepton_int_variables

    if flags.Analysis.do_KinematicFit:
        # do not append KF_mbb variables to float_variable_names['baseline']
        # as they are stored by the KF algorithm not BaselineVarsbbyyAlg
        all_baseline_variable_names += ["KF_mbb"]

    # Here are special variables to be stored for the SH resonant search
    # flags.Analysis.do_resonant_PNN
    if flags.Analysis.do_resonant_PNN:
        SH_float_variables, SH_int_variables \
            = get_BaselineVarsbbyyAlg_SH(flags)
        float_variable_names['baseline'] += SH_float_variables
        int_variable_names['baseline'] += SH_int_variables

    # Here are some variables which can be stored for the boosted case
    if flags.Analysis.do_Boosted:
        boosted_float_variables, boosted_int_variables \
            = get_BoostedVarsbbyyAlg(flags)
        float_variable_names['baseline'] += boosted_float_variables
        int_variable_names['baseline'] += boosted_int_variables

    all_baseline_variable_names += [*chain.from_iterable(float_variable_names.values()),
                                    *chain.from_iterable(int_variable_names.values())]

    if not flags.Analysis.do_CP_systematics:
        sys_suffix = "NOSYS"
    else:
        sys_suffix = "%SYS%"

    for var in all_baseline_variable_names:
        branches += [f"EventInfo.{var}_{sys_suffix} -> bbyy_{var}"
                     + flags.Analysis.systematics_suffix_separator + sys_suffix]

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    object_level_branches = {key: [] for key in keys}
    object_level_float_variables = {key: [] for key in keys}
    object_level_int_variables = {key: [] for key in keys}

    (object_level_branches['photons'],
     object_level_float_variables['photons'],
     object_level_int_variables['photons']) = \
        get_selected_photon_branches_variables(flags, "bbyy")

    float_variable_names['photons'] += object_level_float_variables['photons']
    int_variable_names['photons'] += object_level_int_variables['photons']

    (object_level_branches['jets'],
     object_level_float_variables['jets'],
     object_level_int_variables['jets']) = \
        get_selected_jet_branches_variables(flags, "bbyy")

    # TODO this will have to be changed in "jets"
    # when moving Jet-related variables in separate algo
    float_variable_names['baseline'] += object_level_float_variables["jets"]
    int_variable_names['baseline'] += object_level_int_variables["jets"]

    branches += object_level_branches["photons"]
    branches += object_level_branches["jets"]

    # More event info variables:
    s_name = flags.Analysis.selection_name
    branches += \
        [f"EventInfo.bbyy_pass_{s_name}_{sys_suffix} -> bbyy_pass_{s_name}"
         + flags.Analysis.systematics_suffix_separator + sys_suffix]

    if (flags.Analysis.save_cutflow):
        cutList = flags.Analysis.CutList
        for cut in cutList:
            extra = ""
            if cut == "PASS_TRIGGER" or cut == "PASS_TRIGGER_MATCHING":
                extra = "SINGLE_OR_DIPHOTON"
            branches += [f"EventInfo.{cut}_{sys_suffix} -> bbyy_{cut}{extra}"
                         + flags.Analysis.systematics_suffix_separator + sys_suffix]

    if (flags.Analysis.save_pass_trigger_info):
        photon_triggers = [
            "pass_trigger_single_photon",
            "pass_trigger_diphoton",
            "pass_matching_trigger_single_photon",
            "pass_matching_trigger_diphoton"]

        for trigger in photon_triggers:
            branches += [f"EventInfo.{trigger}_{sys_suffix} -> {trigger}"
                         + flags.Analysis.systematics_suffix_separator + sys_suffix]

    return branches, float_variable_names, int_variable_names
