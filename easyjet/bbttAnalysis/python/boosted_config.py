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
            flags, "bbtt"
        )
        for var in boosted_variables():
            branches += [
                f"EventInfo.{var}_%SYS%"
                + f" -> bbtt_{var}"
                + flags.Analysis.systematics_suffix_separator
                + "%SYS%"
            ]
            float_variables.append(var)

    # Add selection decision
    branches += [
        "EventInfo.bbtt_pass_presel_%SYS% -> bbtt_pass_presel_"
        + flags.Analysis.systematics_suffix_separator
        + "%SYS%"
    ]

    for cat in ["Boosted", "baseline_HadHad", "HadHad"]:
        branches += [
            f"EventInfo.pass_{cat}_%SYS% -> bbtt_pass_{cat}"
            + flags.Analysis.systematics_suffix_separator
            + "%SYS%"
        ]

    return branches, float_variables
