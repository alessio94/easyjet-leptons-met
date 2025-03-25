import AthenaCommon.SystemOfUnits as Units
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from EasyjetHub.algs.postprocessing.SelectorAlgConfig import (
    JetSelectorAlgCfg,
    TauSelectorAlgCfg,
)
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)


def boosted_variables():
    """Get the basic variables for the boosted analysis."""
    variables = ["hh_m", "hh_pt", "hh_delta_eta", "hh_delta_phi"]
    h_variables = [
        "m",
        "pt",
        "eta",
        "phi",
        "E",
        # "truthLabel",
    ]
    for var in h_variables:
        for hc in ["h1", "h2"]:
            variables.append(f"{hc}_{var}")
    return [f"boosted_{var}" for var in variables]


def boosted_branches_variables(flags):
    """Add here the additional branches for the boosted analysis."""
    branches = []
    float_variables = []
    if flags.Analysis.store_high_level_variables:
        branches, float_variables, _ = get_selected_objects_branches_variables(
            flags, "bbtt_boosted"
        )
        for var in boosted_variables():
            # if flags.Input.isMC and "truthLabel" in var:
            #     continue
            branches += [
                f"EventInfo.{var}_%SYS%"
                + f" -> bbtt_{var}"
                + flags.Analysis.systematics_suffix_separator
                + "%SYS%"
            ]
            float_variables.append(var)

    # Add selection decision
    branches += [
        "EventInfo.pass_preselection_%SYS% -> pass_preselection_"
        + flags.Analysis.systematics_suffix_separator
        + "%SYS%"
    ]
    return branches, float_variables


def boosted_cfg(flags, smalljetkey, largeRjetkey, float_variables):
    """Add the variable decoration for the boosted analysis."""
    cfg = ComponentAccumulator()

    if flags.Analysis.do_taus:
        cfg.merge(
            TauSelectorAlgCfg(
                flags,
                containerInKey="AnalysisTauJets_%SYS%",
                containerOutKey="bbttAnalysisTaus_%SYS%",
                minPt=20 * Units.GeV,
                minimumAmount=2,
                truncateAtAmount=4,
            )
        )

    if flags.Analysis.do_small_R_jets:
        cfg.merge(
            JetSelectorAlgCfg(
                flags,
                containerInKey=smalljetkey,
                containerOutKey="bbttAnalysisJets_%SYS%",
                minPt=20 * Units.GeV,
                bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
                selectBjet=False,
                minimumAmount=2,
            )
        )

    cfg.merge(
        JetSelectorAlgCfg(
            flags,
            name="LargeRJetSelectorAlg",
            containerInKey=largeRjetkey,
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

    # TODO: decide which WP we want to use
    GN2X_WP = f"xbb_select_GN2Xv01_{flags.Analysis.Large_R_jet.GN2X_hbb_wps[4]}"
    cfg.addEventAlgo(
        CompFactory.HHBBTT.HHbbttSelectorAlg(
            "HHbbttSelectorAlg",
            channel=flags.Analysis.channels,
            isMC=flags.Input.isMC,
            eventDecisionOutputDecoration="pass_preselection_%SYS%",
            bypass=flags.Analysis.bypass,
            saveCutFlow=flags.Analysis.save_cutflow,
            cutList=flags.Analysis.CutList,
            RootStreamName=(
                "CBK"
                if flags.Analysis.splitCBK
                else flags.Analysis.ttree_output.stream_name
            ),
            doBoostedAnalysis=True,
            GN2X_WP=GN2X_WP,
            # TODO: not implememented or used
            # useTriggerSelections=flags.Analysis.do_trigger_offline_filtering,
            # do1BRegions=False,
            # useNonIsoLeptons=False,
        )
    )

    # calculate final bbtt vars
    if flags.Analysis.store_high_level_variables:
        cfg.addEventAlgo(
            CompFactory.HHBBTT.BaselineVarsBoostedbbttAlg(
                "BaselineVarsBoostedbbttAlg",
                isMC=flags.Input.isMC,
                floatVariableList=float_variables,
            )
        )

    # TODO: for the moment we don't have SF for boosted Jets
    return cfg
