from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from EasyjetHub.algs.postprocessing.SelectorAlgConfig import (
    JetSelectorAlgCfg,)
from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches_variables,
)


def ZbbjCalib_cfg(flags, largejetkey,
                  float_variables=None, int_variables=None):
    if not float_variables:
        float_variables = []
    if not int_variables:
        int_variables = []

    cfg = ComponentAccumulator()

    selected_large_r = "XbbCalibLRJets_%SYS%"

    cfg.merge(
        JetSelectorAlgCfg(
            flags,
            name="LargeJetSelectorAlg",
            containerInKey=largejetkey,
            containerOutKey=selected_large_r,
            minPt=flags.Analysis.Large_R_jet.min_pT,
            maxEta=flags.Analysis.Large_R_jet.maxEta,
            minimumAmount=1,
        )
    )

    cfg.addEventAlgo(
        CompFactory.XBBCALIB.ZbbjCalibSelectorAlg(
            "ZbbjCalibSelectorAlg",
            largeRJets=selected_large_r,
            eventDecisionOutputDecoration="XbbCalib_pass_sr_%SYS%",
            bypass=flags.Analysis.bypass,
        )
    )

    cfg.addEventAlgo(
        CompFactory.XBBCALIB.BaselineVarsZbbjCalibAlg(
            "BaselineVarsZbbjCalibAlg",
            largeRJets=selected_large_r
        )
    )

    return cfg


def ZbbjCalib_branches(flags):
    branches = []

    # this will be all the variables that are calculated by the
    # BaselineVarsZbbjCalibAlg algorithm
    float_variable_names = []
    int_variable_names = []

    # these are the variables that will always be stored by easyjet specific
    # to XbbCalib. However, at this stage we have no varibles that are
    # calculated by this algorithm
    # so we will just make a placeholder list

    # These are the variables always saved with the objects selected
    # by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    object_level_branches, object_level_float_variables, \
        object_level_int_variables = \
        get_selected_objects_branches_variables(flags, "XbbCalib")
    float_variable_names += object_level_float_variables
    int_variable_names += object_level_int_variables

    branches += object_level_branches

    branches += [
        "EventInfo.XbbCalib_pass_sr_%SYS% -> XbbCalib_pass_SR"
        + flags.Analysis.systematics_suffix_separator
        + "%SYS%"
    ]

    return branches, float_variable_names, int_variable_names
