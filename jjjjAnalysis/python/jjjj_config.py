from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import (
    JetSelectorAlgCfg)
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)


def jjjj_cfg(flags, smalljetkey, float_variables=None, int_variables=None):
    if not float_variables:
        float_variables = []
    if not int_variables:
        int_variables = []

    cfg = ComponentAccumulator()
    cfg.merge(JetSelectorAlgCfg(flags, name="SmallJetSelectorAlg",
                                containerInKey=smalljetkey,
                                containerOutKey="jjjjAnalysisJets_%SYS%",
                                bTagWPDecorName="",
                                selectBjet=False,
                                minPt=20 * Units.GeV,
                                minimumAmount=4))  # -1 means ignores this

    from EasyjetHub.algs.postprocessing.trigger_matching import TriggerMatchingToolCfg

    trigger_branches = [
        f"trigPassed_{c}"
        for c in flags.Analysis.TriggerChains
    ]
    # Selection with passing trigger and applying pT cut
    cfg.addEventAlgo(
        CompFactory.jjjj.jjjjSelectorAlg(
            "jjjjSelectorAlg",
            eventDecisionOutputDecoration="jjjj_pass_sr_%SYS%",
            cutList=flags.Analysis.CutList,
            saveCutFlow=flags.Analysis.save_cutflow,
            isMC=flags.Input.isMC,
            triggerLists=trigger_branches,
            trigMatchingTool=cfg.popToolsAndMerge(TriggerMatchingToolCfg(flags)),
            bypass=(flags.Analysis.bypass if hasattr(flags.Analysis, 'bypass')
                    else False),
        )
    )
    # Calculate final variables
    cfg.addEventAlgo(
        CompFactory.jjjj.BaselineVarsjjjjAlg(
            "BaselineVarsjjjjAlg",
            SmallRJets="jjjjAnalysisJets_%SYS%",
            floatVariableList=float_variables,
            intVariableList=int_variables,
            isMC=flags.Input.isMC,
        )
    )
    return cfg


def get_BaselineVarsjjjjAlg_variables(flags):
    float_variable_names = []
    int_variable_names = []

    # basic p4() of 4 jets
    for object in ["jjjj_Jet1", "jjjj_Jet2", "jjjj_Jet3", "jjjj_Jet4"]:
        if flags.Input.isMC:
            int_variable_names.append(f"{object}_PTLID")
    int_variable_names += ["nJets"]

    # event shape variables fo QG search
    QG_vars = ["Q1", "Q2", "Q3"]
    QG_vars += ["transverseSphericity", "transverseThrust", "thrustMinor"]
    for var in QG_vars:
        float_variable_names.append(f"{var}")
    return float_variable_names, int_variable_names


def jjjj_branches(flags):
    branches = []

    all_baseline_variable_names = []
    float_variable_names = []
    int_variable_names = []
    baseline_float_variables, baseline_int_variables \
        = get_BaselineVarsjjjjAlg_variables(flags)
    float_variable_names += baseline_float_variables
    int_variable_names += baseline_int_variables

    all_baseline_variable_names += [*float_variable_names, *int_variable_names]

    for var in all_baseline_variable_names:
        branches += [f"EventInfo.{var}_%SYS% -> {var}_%SYS%"]
    object_level_branches, object_level_float_variables, object_level_int_variables \
        = get_selected_objects_branches_variables(flags, "jjjj")
    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    branches += object_level_branches

    # trigger info in ntuple
    for cat in ["SJT"]:
        branches += \
            [f"EventInfo.pass_trigger_{cat}_%SYS% -> pass_trigger_{cat}"
             + flags.Analysis.systematics_suffix_separator + "%SYS%"]
    return branches, float_variable_names, int_variable_names
