# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import (
    MuonSelectorAlgCfg, ElectronSelectorAlgCfg, LeptonOrderingAlgCfg,
    TauSelectorAlgCfg, JetSelectorAlgCfg)
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)
from EasyjetHub.steering.analysis_configuration import (
    get_trigger_legs_scale_factor_list)


def bbbbtt_cfg(flags, smalljetkey, muonkey, electronkey,
               taukey, float_variables=None, int_variables=None):
    if not float_variables:
        float_variables = []
    if not int_variables:
        int_variables = []

    cfg = ComponentAccumulator()

    # anti-iso lepton control region is not compatible with the other
    # regions at the moment:
    use_noniso_leptons = "AntiIsoLepHad" in flags.Analysis.channels
    if (use_noniso_leptons and len(flags.Analysis.channels) > 1):
        raise ValueError("Cannot run 'antiiso-lephad' with any other channels")

    # muons:
    cfg.merge(MuonSelectorAlgCfg(flags,
                                 containerInKey=muonkey,
                                 containerOutKey="bbbbttAnalysisMuons_%SYS%"))

    # electrons:
    cfg.merge(ElectronSelectorAlgCfg(flags,
                                     containerInKey=electronkey,
                                     containerOutKey="bbbbttAnalysisElectrons_%SYS%"))
    # leptons:
    cfg.merge(LeptonOrderingAlgCfg(flags,
                                   containerInEleKey=electronkey,
                                   containerInMuKey=muonkey))

    cfg.merge(TauSelectorAlgCfg(flags,
                                # Baseline always needed for anti-taus
                                containerInKey=taukey,
                                containerOutKey="bbbbttAnalysisTaus_%SYS%"))

    cfg.merge(JetSelectorAlgCfg(
        flags,
        containerInKey=smalljetkey,
        containerOutKey="bbbbttAnalysisJets_%SYS%",
        minPt=20 * Units.GeV,
        bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
        selectBjet=False,
        minimumAmount=2))

    muon_WPs = [f'{wp[0]}_{wp[1]}' for wp in flags.Analysis.Muon.extra_wps]
    ele_WPs = [f'{wp[0]}_{wp[1]}' for wp in flags.Analysis.Electron.extra_wps]
    cfg.addEventAlgo(
        CompFactory.HHHBBBBTT.HHHbbbbttSelectorAlg(
            "HHHbbbbttSelectorAlg",
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
            tauWP=flags.Analysis.Tau.extra_wps[0],
            muonWPs=muon_WPs,
            eleWPs=ele_WPs,
            useNonIsoLeptons=use_noniso_leptons,
            eventDecisionOutputDecoration=(
                "bbbbtt_pass_presel_noMMC_%SYS%" if flags.Analysis.enable_MMC_cut
                else "bbbbtt_pass_presel_%SYS%"),
            channel=flags.Analysis.channels,
            isMC=flags.Input.isMC,
            doAntiIDRegions=flags.Analysis.do_antiID_regions,
            do0BRegions=flags.Analysis.do_0B_regions,
            do1BRegions=flags.Analysis.do_1B_regions,
            do2BRegions=flags.Analysis.do_2B_regions,
            do3BRegions=flags.Analysis.do_3B_regions,
            useTriggerSelections=flags.Analysis.do_trigger_offline_filtering,
            bypass=flags.Analysis.bypass,
            saveCutFlow=flags.Analysis.save_cutflow,
            cutList=(
                flags.Analysis.CutList if hasattr(flags.Analysis, "CutList") else []),
            RootStreamName=('CBK' if flags.Analysis.splitCBK else
                            flags.Analysis.ttree_output.stream_name)
        )
    )

    # MMC decoration. Using same as bbtt
    if flags.Analysis.do_mmc:
        from EasyjetHub.algs.mmc_tool_config import MissingMassToolCfg
        mmcTool = cfg.popToolsAndMerge(
            MissingMassToolCfg(
                flags, CalibSet="2024",
                ParamFilePath="MMC_params_v051224_angle_noLikelihoodFit.root"))

        cfg.addEventAlgo(
            CompFactory.HHBBTT.MMCDecoratorAlg(
                "MMCDecoratorAlg",
                channel=flags.Analysis.channels,
                mmcTool=mmcTool,
                bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
                jets="bbbbttAnalysisJets_%SYS%",
                electrons="bbbbttAnalysisElectrons_%SYS%",
                muons="bbbbttAnalysisMuons_%SYS%",
                taus="bbbbttAnalysisTaus_%SYS%",
                met="AnalysisMET_%SYS%",
            )
        )

        if flags.Analysis.enable_MMC_cut:
            cfg.addEventAlgo(
                CompFactory.HHBBTT.MMCSelectorAlg(
                    "MMCSelectorAlg",
                    channel=flags.Analysis.channels,
                    MMC_min=60 * Units.GeV,
                    eventDecisionOutputDecoration="bbbbtt_pass_presel_%SYS%",
                    bypass=flags.Analysis.bypass,
                )
            )

    btag_pcbt_wps \
        = [wp for wp in flags.Analysis.Small_R_jet.btag_extra_wps if "Continuous" in wp] # noqa

    # calculate final bbbbtt vars
    if flags.Analysis.store_high_level_variables:
        cfg.addEventAlgo(
            CompFactory.HHHBBBBTT.BaselineVarsbbbbttAlg(
                "FinalVarsbbbbttAlg",
                isMC=flags.Input.isMC,
                useNonIsoLeptons=use_noniso_leptons,
                electrons=electronkey, eleWPs=ele_WPs,
                muons=muonkey, muonWPs=muon_WPs,
                taus=taukey, tauWP=flags.Analysis.Tau.extra_wps[0],
                doMMC=flags.Analysis.do_mmc,
                bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
                PCBTDecorList=["ftag_quantile_" + pcbt_wp for pcbt_wp in btag_pcbt_wps], # noqa
                floatVariableList=float_variables,
                intVariableList=int_variables
            )
        )

    # calculate event trigger SF
    if flags.Input.isMC:
        cfg.addEventAlgo(
            CompFactory.HHHBBBBTT.TriggerSFAlg(
                "TriggerSFAlg",
                eleTriggerSF=get_trigger_legs_scale_factor_list(flags, 'Electron'),
                muonTriggerSF=get_trigger_legs_scale_factor_list(flags, 'Muon'),
                tauTriggerSF=get_trigger_legs_scale_factor_list(flags, 'Tau'),
                electrons=electronkey,
                muons=muonkey,
                taus=taukey
            )
        )

    # include variables for kappa-reweighting
    if flags.Input.MCChannelNumber in flags.Analysis.Truth.DSID_HHH4b2tau_KappaReweight:
        reweightVars = get_ReweightingVars()
        cfg.addEventAlgo(
            CompFactory.HHHBBBBTT.KappaReweightingAlg(
                "KappaReweighting",
                reweightVars=reweightVars,
            )
        )

    return cfg


def get_ReweightingVars():
    reweightVars = [
        "k3_0_k4_0",
        "k3_1_k4_m1",
        "k3_m1_k4_1",
        "k3_1_k4_0",
        "k3_0_k4_1",
        "k3_m1_k4_0",
        "k3_0_k4_m1",
        "k3_0p5_k4_0",
        "k3_m0p5_k4_0",
    ]

    return reweightVars


def get_BaselineVarsbbbbttAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    combined_particles = [
        "HH_bbbb",
        "H_vis_tautau",
        "HHH_vis",
    ]

    if flags.Analysis.do_mmc:
        combined_particles += ["HHH"]

    for particle in combined_particles:
        for var in ["pt", "eta", "phi", "m"]:
            float_variable_names.append(f"{particle}_{var}")
    return float_variable_names, int_variable_names


def get_BaselineVarsbbbbttAlg_highlevelvariables(flags):

    high_level_float_variables = []
    high_level_int_variables = []

    if flags.Analysis.do_mmc:
        high_level_float_variables += [
        ]

    return high_level_float_variables, high_level_int_variables


def bbbbtt_branches(flags):
    # a list of strings which maps the variable name in the c++ code
    # to the outputname:
    # "EventInfo.<var> -> <outputvarname>"
    branches = []

    # this will be all the variables that are calculated by the
    # BaselineVarsbbbbttAlg algorithm
    all_baseline_variable_names = []
    float_variable_names = []
    int_variable_names = []

    # these are the variables that will always be stored by easyjet specific to HHbbtt
    # further below there are more high level variables which can be
    # stored using the flag
    if flags.Analysis.store_high_level_variables:
        baseline_float_variables, baseline_int_variables \
            = get_BaselineVarsbbbbttAlg_variables(flags)
        float_variable_names += baseline_float_variables
        int_variable_names += baseline_int_variables

    if flags.Input.isMC:
        all_baseline_variable_names.append("eventTriggerSF")

    if flags.Analysis.do_mmc:
        # do not append mmc variables to float_variable_names
        # or int_variable_names as they are stored by the
        # mmc algortithm not BaselineVarsbbbbttAlg
        for var in ["status", "pt", "eta", "phi", "m"]:
            all_baseline_variable_names.append(f"mmc_{var}")
        for var in ["pt", "eta", "phi", "m"]:
            all_baseline_variable_names.append(f"mmc_nu1_{var}")
            all_baseline_variable_names.append(f"mmc_nu2_{var}")

    if flags.Analysis.store_high_level_variables:
        high_level_float_variables, high_level_int_variables \
            = get_BaselineVarsbbbbttAlg_highlevelvariables(flags)
        float_variable_names += high_level_float_variables
        int_variable_names += high_level_int_variables

    all_baseline_variable_names += [*float_variable_names, *int_variable_names]

    for var in all_baseline_variable_names:
        branches += [f"EventInfo.{var}_%SYS% -> bbbbtt_{var}"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    object_level_branches, object_level_float_variables, object_level_int_variables \
        = get_selected_objects_branches_variables(flags, "bbbbtt")
    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    if flags.Analysis.store_high_level_variables:
        branches += object_level_branches

    branches += ["EventInfo.bbbbtt_pass_presel_%SYS% -> bbbbtt_pass_presel"
                 + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    # trigger variables do not need to be added to variable_names
    # as it is written out in HHHbbbbttSelectorAlg
    for var in ["_trigger_", "_baseline_"]:
        for cat in ["SR", "SLT", "LTT", "STT", "DTT",
                    "DTT_2016", "DTT_4J12", "DTT_L1Topo",
                    "DTT_4J12_delayed", "DTT_L1Topo_delayed", "DBT"]:
            if (var == "_baseline_"
                    and cat in ["DTT_4J12_delayed", "DTT_L1Topo_delayed"]):
                continue
            branches += [f"EventInfo.pass{var}{cat}_%SYS% -> "
                         f"bbbbtt_pass{var}{cat}"
                         + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    for cat in ["SR", "SLT", "LTT", "STT", "DTT",
                "DTT_2016", "DTT_4J12", "DTT_L1Topo",
                "DTT_4J12_delayed", "DTT_L1Topo_delayed", "DBT",
                "LepHad", "HadHad"]:
        for nb in ["0B", "1B", "2B", "3B", "4B"]:
            branches += [f"EventInfo.pass_{cat}_{nb}_%SYS% ->"
                         f"bbbbtt_pass_{cat}_{nb}"
                         + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    for cat in ["baseline_LepHad", "baseline_HadHad", "LepHad", "HadHad",
                "ZCR", "TopEMuCR", "AntiIsoLepHad"]:
        branches += [f"EventInfo.pass_{cat}_%SYS% -> bbbbtt_pass_{cat}"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    for var in ["N_LEPTONS_CUT_LEPHAD", "N_LEPTONS_CUT_ANTIISOLEPHAD",
                "N_LEPTONS_CUT_HADHAD"]:
        branches += [f"EventInfo.{var}_%SYS% -> bbbbtt_{var.lower()}"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    if flags.Input.MCChannelNumber in flags.Analysis.Truth.DSID_HHH4b2tau_KappaReweight:
        reweight_vars = get_ReweightingVars()
        for var in reweight_vars:
            branches += [f"EventInfo.{var} -> KappaReweighting__{var}"]

    return branches, float_variable_names, int_variable_names
