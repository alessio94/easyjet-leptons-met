from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from EasyjetHub.output.ttree.selected_objects import (
    get_selected_objects_branches,
)


def boosted_cfg(flags, largejetkey, float_variables=None):
    if not float_variables:
        float_variables = []

    cfg = ComponentAccumulator()

    cfg.addEventAlgo(
        CompFactory.HH4B.BaselineVarsBoostedAlg(
            "BaselineVarsBoostedAlg",
            isMC=flags.Input.isMC,
            floatVariableList=float_variables,
        )
    )

    return cfg


def get_BaselineVarsBoostedAlg_variables(flags):
    float_variable_names = []
#    int_variable_names = []

    # Define the variables that will be saved in the output TTree
    boosted_vars = ["hh_m", "hh_pt", "hh_delta_eta", "hh_delta_phi"]

    variables = [
        "m", "pt", "eta", "phi", "E",
        "Tau32_wta", "GN2Xv01_disc",
        "GN2Xv01_phbb", "GN2Xv01_pqcd",
        "GN2Xv01_phcc", "GN2Xv01_ptop"
    ]
    for var in variables:
        for hc in ["h1", "h2"]:
            boosted_vars.append(f"{hc}_{var}")
    boosted_vars += ["h1_truthLabel", "h2_truthLabel"]
    boosted_vars = [f"boosted_{var}" for var in boosted_vars]
    float_variable_names += boosted_vars

    return float_variable_names


def boosted_branches(flags):
    branches = []
    float_variable_names = []

    baseline_float_variables = get_BaselineVarsBoostedAlg_variables(flags)
    float_variable_names += baseline_float_variables

    # These are the variables always saved with the objects selected by the analysis
    # This is tunable with the flags amount and variables
    # in the object configs.
    branches += get_selected_objects_branches(flags, "bbbb_boosted")

    for var in float_variable_names:
        if flags.Input.isMC and "truthLabel" in var:
            continue
        branches += [
            f"EventInfo.{var}_%SYS%"
            + f" -> bbbb_{var}"
            + flags.Analysis.systematics_suffix_separator + "%SYS%"
        ]

    return branches, float_variable_names
