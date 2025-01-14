from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)


def JJ_cfg(flags, float_variables=None, int_variables=None):
    if not float_variables:
        float_variables = []
    if not int_variables:
        int_variables = []

    cfg = ComponentAccumulator()

    # signal large-R jets
    cfg.addEventAlgo(
        CompFactory.VBSVV4q.SignalJetsSelectorAlg(
            "SignalJetsSelectorAlg",
            SmallRJets="VBSVV4qAnalysisJets_%SYS%",
            LargeRJets="VBSVV4qAnalysisLargeJets_%SYS%",
            SigLargeRJets="VBSVV4qAnalysisSigLargeJets_%SYS%",
        )
    )

    # tagging jets
    cfg.addEventAlgo(
        CompFactory.VBSVV4q.VBSJetsSelectorAlg(
            "VBSJetsSelectorAlg",
            smallRjets="VBSVV4qAnalysisJets_%SYS%",
            largeRjets="VBSVV4qAnalysisLargeJets_%SYS%",
            TagJetsCriteria=flags.Analysis.TagJetsCriteria,
            DeltaRJj=flags.Analysis.DeltaRJj,
            VBSJetsContainerOutKey="VBSVV4qAnalysisVBSJets_%SYS%",
            noVBSJetsContainerOutKey="VBSVV4qAnalysisNoVBSJets_%SYS%",
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
        )
    )

    # Selection
    cfg.addEventAlgo(
        CompFactory.VBSVV4q.JJSelectorAlg(
            "JJSelectorAlg",
            SmallRJets="VBSVV4qAnalysisJets_%SYS%",
            LargeRJets="VBSVV4qAnalysisLargeJets_%SYS%",
            vbsjets="VBSVV4qAnalysisVBSJets_%SYS%",
            eventDecisionOutputDecoration="VBSVV4q_pass_sel_%SYS%",
            cutList=flags.Analysis.CutList,
            saveCutFlow=flags.Analysis.save_cutflow,
            bypass=(flags.Analysis.bypass if hasattr(flags.Analysis, 'bypass')
                    else False),
        )
    )

    # calculate final VBSVV4q vars
    cfg.addEventAlgo(
        CompFactory.VBSVV4q.BaselineVarsJJAlg(
            "BaselineVarsJJAlg",
            SmallRJets="VBSVV4qAnalysisJets_%SYS%",
            LargeRJets="VBSVV4qAnalysisLargeJets_%SYS%",
            SigLargeRJets="VBSVV4qAnalysisSigLargeJets_%SYS%",
            vbsjets="VBSVV4qAnalysisVBSJets_%SYS%",
            isMC=flags.Input.isMC,
            bTagWPDecorName="ftag_select_" + flags.Analysis.Small_R_jet.btag_wp,
            floatVariableList=float_variables,
            intVariableList=int_variables,
            loadGN2x=flags.Analysis.loadGN2x
        )
    )
    return cfg


def get_BaselineVarsJJAlg_variables(flags):

    int_variable_names = []
    float_variable_names = []

    # event level
    int_variable_names += ["nLargeRJets", "nJets", ]
    int_variable_names += ["nBJets", "nCentralJets", "nForwardJets"]

    # tagging jets
    for object in ["TagJet1", "TagJet2"]:
        for var in ["pT", "eta", "phi", "E"]:
            float_variable_names.append(f"{object}_{var}")

    for object in ["TagJets"]:
        for var in ["pT", "eta", "phi", "M", "deta", "DR", "dphi"]:
            float_variable_names.append(f"{object}_{var}")

    # signal large-R jets
    jet_vars = ["pT", "eta", "phi", "E", "M"]
    jet_vars += ["DXbb", "phbb", "phcc", "pqcd", "ptop"]
    jet_vars += ["D2", "ECF1", "ECF2", "ECF3", "Split12", "Split23", ]
    jet_vars += ["Tau1_wta", "Tau2_wta", "Tau3_wta"]
    jet_vars += ["ZCut12", "KtDR", "Angularity"]
    jet_vars += ["FoxWolfram0", "FoxWolfram2", "Aplanarity", "PlanarFlow", "Qw"]
    for object in ["SigJet1", "SigJet2"]:
        for var in jet_vars:
            float_variable_names.append(f"{object}_{var}")

    # signal dijets large-R
    for object in ["JJ"]:
        for var in ["pT", "eta", "phi", "M", "DR", "dphi", "deta"]:
            float_variable_names.append(f"{object}_{var}")

    return float_variable_names, int_variable_names


def JJ_branches(flags):
    branches = []

    # this will be all the variables that are calculated by the
    # BaselineVarsJJAlg algorithm
    all_baseline_variable_names = []
    float_variable_names = []
    int_variable_names = []

    # these are the variables that will always be stored by easyjet specific to HHbbtt
    # further below there are more high level variables which can be
    # stored using the flag
    # flags.Analysis.store_high_level_variables
    baseline_float_variables, baseline_int_variables \
        = get_BaselineVarsJJAlg_variables(flags)
    float_variable_names += baseline_float_variables
    int_variable_names += baseline_int_variables

    all_baseline_variable_names += [*float_variable_names, *int_variable_names]

    for var in all_baseline_variable_names:
        branches += [f"EventInfo.{var}_%SYS% -> {var}_%SYS%"]

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    object_level_branches, object_level_float_variables, object_level_int_variables \
        = get_selected_objects_branches_variables(flags, "JJ")
    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    branches += object_level_branches
    branches += ["EventInfo.VBSVV4q_pass_sel_%SYS% -> pass_sel_%SYS%"]

    if (flags.Analysis.save_cutflow):
        cutList = flags.Analysis.CutList
        for cut in cutList:
            branches += [f"EventInfo.{cut}_%SYS% -> {cut}_%SYS%"]

    for cat in ["SJT"]:
        branches += \
            [f"EventInfo.pass_trigger_{cat}_%SYS% -> pass_trigger_{cat}"
                + flags.Analysis.systematics_suffix_separator + "%SYS%"]

    return branches, float_variable_names, int_variable_names
