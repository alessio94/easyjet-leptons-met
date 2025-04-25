# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units
from AthenaConfiguration.Enums import LHCPeriod

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import (
    MuonSelectorAlgCfg, ElectronSelectorAlgCfg, LeptonOrderingAlgCfg,
    TauSelectorAlgCfg, JetSelectorAlgCfg)
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)
from EasyjetHub.steering.analysis_configuration import (
    get_trigger_legs_scale_factor_list)


def bbtt_cfg(flags, smalljetkey, muonkey, electronkey,
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
                                 containerOutKey="bbttAnalysisMuons_%SYS%"))

    # electrons:
    cfg.merge(ElectronSelectorAlgCfg(flags,
                                     containerInKey=electronkey,
                                     containerOutKey="bbttAnalysisElectrons_%SYS%"))
    # leptons:
    cfg.merge(LeptonOrderingAlgCfg(flags,
                                   containerInEleKey=electronkey,
                                   containerInMuKey=muonkey))

    cfg.merge(TauSelectorAlgCfg(flags,
                                # Baseline always needed for anti-taus
                                containerInKey=taukey,
                                containerOutKey="bbttAnalysisTaus_%SYS%"))

    cfg.merge(JetSelectorAlgCfg(
        flags,
        containerInKey=smalljetkey,
        containerOutKey="bbttAnalysisJets_%SYS%",
        minPt=20 * Units.GeV,
        bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
        selectBjet=False,
        minimumAmount=2))

    GN2X_WP = ""
    if flags.Analysis.do_large_R_UFO_jets:
        cfg.merge(
            JetSelectorAlgCfg(
                flags,
                name="LargeRJetSelectorAlg",
                containerInKey=flags.Analysis.container_names.output.reco10UFOJet,
                containerOutKey="bbttAnalysisLargeRJets_%SYS%",
                bTagWPDecorName="",
                selectBjet=False,
                minPt=200.0 * Units.GeV,
                maxPt=3000.0 * Units.GeV,
                maxMass=600.0 * Units.GeV,
                maxEta=2.0,
                jetAmount=flags.Analysis.Large_R_jet.amount_leadingjet,
            )
        )
        # Use the loosest working point for selection here.
        GN2X_WP = f"xbb_select_GN2Xv01_{flags.Analysis.Large_R_jet.GN2X_hbb_wps[0]}"

    muon_WPs = [f'{wp[0]}_{wp[1]}' for wp in flags.Analysis.Muon.extra_wps]
    ele_WPs = [f'{wp[0]}_{wp[1]}' for wp in flags.Analysis.Electron.extra_wps]
    cfg.addEventAlgo(
        CompFactory.HHBBTT.HHbbttSelectorAlg(
            "HHbbttSelectorAlg",
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
            tauWP=flags.Analysis.Tau.extra_wps[0],
            muonWPs=muon_WPs,
            eleWPs=ele_WPs,
            useNonIsoLeptons=use_noniso_leptons,
            eventDecisionOutputDecoration=(
                "bbtt_pass_presel_noMMC_%SYS%" if flags.Analysis.enable_MMC_cut
                else "bbtt_pass_presel_%SYS%"),
            channel=flags.Analysis.channels,
            isMC=flags.Input.isMC,
            doAntiIDRegions=flags.Analysis.do_antiID_regions,
            do1BRegions=flags.Analysis.do_1B_regions,
            useTriggerSelections=flags.Analysis.do_trigger_offline_filtering,
            bypass=flags.Analysis.bypass,
            saveCutFlow=flags.Analysis.save_cutflow,
            cutList=(
                flags.Analysis.CutList if hasattr(flags.Analysis, "CutList") else []),
            RootStreamName=('CBK' if flags.Analysis.splitCBK else
                            flags.Analysis.ttree_output.stream_name),
            doLargeRJets=flags.Analysis.do_large_R_UFO_jets,
            GN2X_WP=GN2X_WP,
        )
    )

    # MMC decoration
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
                bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
                mmcTool=mmcTool
            )
        )

        if flags.Analysis.enable_MMC_cut:
            cfg.addEventAlgo(
                CompFactory.HHBBTT.MMCSelectorAlg(
                    "MMCSelectorAlg",
                    channel=flags.Analysis.channels,
                    MMC_min=60 * Units.GeV,
                    eventDecisionOutputDecoration="bbtt_pass_presel_%SYS%",
                    bypass=flags.Analysis.bypass,
                )
            )

    btag_pcbt_wps \
        = [wp for wp in flags.Analysis.Small_R_jet.btag_extra_wps if "Continuous" in wp] # noqa

    # calculate final bbtt vars
    if flags.Analysis.store_high_level_variables:

        if flags.Analysis.do_boosted_dihiggs:
            cfg.addEventAlgo(
                CompFactory.HHBBTT.BaselineVarsBoostedbbttAlg(
                    "BaselineVarsBoostedbbttAlg",
                    isMC=flags.Input.isMC,
                    floatVariableList=float_variables,
                    GN2X_WP=GN2X_WP,
                )
            )
        else:
            cfg.addEventAlgo(
                CompFactory.HHBBTT.BaselineVarsbbttAlg(
                    "FinalVarsbbttAlg",
                    isMC=flags.Input.isMC,
                    useNonIsoLeptons=use_noniso_leptons,
                    electrons=electronkey, eleWPs=ele_WPs,
                    saveDummyEleSF=flags.GeoModel.Run is LHCPeriod.Run2,
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
    # TODO: __BOOSTED__ for the moment we don't have SF for boosted Jets
    if flags.Input.isMC and not flags.Analysis.do_boosted_dihiggs:
        cfg.addEventAlgo(
            CompFactory.HHBBTT.TriggerSFAlg(
                "TriggerSFAlg",
                eleTriggerSF=get_trigger_legs_scale_factor_list(flags, 'Electron'),
                muonTriggerSF=get_trigger_legs_scale_factor_list(flags, 'Muon'),
                tauTriggerSF=get_trigger_legs_scale_factor_list(flags, 'Tau'),
                electrons=electronkey,
                muons=muonkey,
                taus=taukey
            )
        )

    return cfg


def get_BaselineVarsbbttAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    combined_particles = [
        "H_bb",
        "H_vis_tautau",
        "HH_vis",
    ]

    if flags.Analysis.do_mmc:
        combined_particles += ["HH"]

    for particle in combined_particles:
        for var in ["pt", "eta", "phi", "m"]:
            float_variable_names.append(f"{particle}_{var}")
    return float_variable_names, int_variable_names


def get_BaselineVarsbbttAlg_highlevelvariables(flags):
    high_level_float_variables = []
    high_level_int_variables = []

    high_level_float_variables += [
        "HH_vis_delta_phi",
    ]

    if flags.Analysis.do_mmc:
        high_level_float_variables += [
            "HH_delta_phi",
        ]

    return high_level_float_variables, high_level_int_variables


def bbtt_branches(flags):
    # a list of strings which maps the variable name in the c++ code
    # to the outputname:
    # "EventInfo.<var> -> <outputvarname>"
    branches = []

    # this will be all the variables that are calculated by the
    # BaselineVarsbbttAlg algorithm
    all_baseline_variable_names = []
    float_variable_names = []
    int_variable_names = []

    # these are the variables that will always be stored by easyjet specific to HHbbtt
    # further below there are more high level variables which can be
    # stored using the flag
    if flags.Analysis.store_high_level_variables:
        baseline_float_variables, baseline_int_variables \
            = get_BaselineVarsbbttAlg_variables(flags)
        float_variable_names += baseline_float_variables
        int_variable_names += baseline_int_variables

    if flags.Input.isMC:
        all_baseline_variable_names.append("eventTriggerSF")

    if flags.Analysis.do_mmc:
        # do not append mmc variables to float_variable_names
        # or int_variable_names as they are stored by the
        # mmc algortithm not BaselineVarsbbttAlg
        for var in ["status", "pt", "eta", "phi", "m"]:
            all_baseline_variable_names.append(f"mmc_{var}")
        for var in ["pt", "eta", "phi", "m"]:
            all_baseline_variable_names.append(f"mmc_nu1_{var}")
            all_baseline_variable_names.append(f"mmc_nu2_{var}")

    if flags.Analysis.store_high_level_variables:
        high_level_float_variables, high_level_int_variables \
            = get_BaselineVarsbbttAlg_highlevelvariables(flags)
        float_variable_names += high_level_float_variables
        int_variable_names += high_level_int_variables

    all_baseline_variable_names += [*float_variable_names, *int_variable_names]

    for var in all_baseline_variable_names:
        branches += [f"EventInfo.{var}_%SYS% -> bbtt_{var}"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    object_level_branches, object_level_float_variables, object_level_int_variables \
        = get_selected_objects_branches_variables(flags, "bbtt")
    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    if flags.Analysis.store_high_level_variables:
        branches += object_level_branches

    if flags.Analysis.store_high_level_variables:
        branches += ["EventInfo.bbtt_pass_presel_%SYS% -> bbtt_pass_presel"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    # special jet branches only for bbtautau
    smalljetkey = flags.Analysis.container_names.output.reco4PFlowJet
    small_R_prefix = (
        "recojet_antikt4"
        + flags.Analysis.Small_R_jet.jet_type.replace("reco4", "").replace("Jet", "")  # noqa: E501
    )
    branches += [
        f'{smalljetkey}.trigMatch_L1_onlinept'
        f'->{small_R_prefix}_trigMatch_L1_onlinept'
        + flags.Analysis.systematics_suffix_separator + "%SYS%",
        f'{smalljetkey}.trigMatch_L1_onlineeta'
        f'->{small_R_prefix}_trigMatch_L1_onlineeta'
        + flags.Analysis.systematics_suffix_separator + "%SYS%"
    ]
    if flags.GeoModel.Run is LHCPeriod.Run3:
        branches += [
            f'{smalljetkey}.trigMatch_HLT_threshold'
            f'->{small_R_prefix}_trigMatch_HLT_threshold'
            + flags.Analysis.systematics_suffix_separator + "%SYS%",
            f'{smalljetkey}.trigMatch_HLT_onlinept'
            f'->{small_R_prefix}_trigMatch_HLT_onlinept'
            + flags.Analysis.systematics_suffix_separator + "%SYS%"
        ]

    # trigger variables do not need to be added to variable_names
    # as it is written out in HHbbttSelectorAlg
    if flags.Analysis.store_high_level_variables:
        for var in ["_trigger_", "_baseline_"]:
            for cat in ["SR", "SLT", "LTT", "STT", "DTT",
                        "DTT_2016", "DTT_4J12", "DTT_L1Topo",
                        "DTT_4J12_delayed", "DTT_L1Topo_delayed", "DBT"]:
                if (var == "_baseline_"
                        and cat in ["DTT_4J12_delayed", "DTT_L1Topo_delayed"]):
                    continue
                branches += [f"EventInfo.pass{var}{cat}_%SYS% -> "
                             f"bbtt_pass{var}{cat}"
                             + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    for cat in ["SR", "SLT", "LTT", "STT", "DTT",
                "DTT_2016", "DTT_4J12", "DTT_L1Topo",
                "DTT_4J12_delayed", "DTT_L1Topo_delayed", "DBT",
                "LepHad", "HadHad"]:
        for nb in ["1B", "2B"]:
            branches += [f"EventInfo.pass_{cat}_{nb}_%SYS% ->"
                         f"bbtt_pass_{cat}_{nb}"
                         + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    if flags.Analysis.store_high_level_variables:
        for cat in ["baseline_LepHad", "baseline_HadHad"]:
            branches += [f"EventInfo.pass_{cat}_%SYS% -> bbtt_pass_{cat}"
                         + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    for cat in ["LepHad", "HadHad",
                "ZCR", "TopEMuCR", "AntiIsoLepHad"]:
        branches += [f"EventInfo.pass_{cat}_%SYS% -> bbtt_pass_{cat}"
                     + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    if "Boosted" in flags.Analysis.channels:
        branches += [
            "EventInfo.pass_Boosted_%SYS% -> bbtt_pass_Boosted"
            + flags.Analysis.systematics_suffix_separator + "%SYS%"
        ]

    return branches, float_variable_names, int_variable_names
