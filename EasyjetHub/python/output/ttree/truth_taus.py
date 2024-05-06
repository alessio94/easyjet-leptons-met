from EasyjetHub.output.ttree.branch_manager import BranchManager


def get_truthtau_branches(flags, input_container, output_prefix):
    truthtau_branches = BranchManager(
        input_container,
        output_prefix,
        do_overlap_removal=False,
    )

    truthtau_branches.variables = \
        ["px", "py", "pz", "e", "m", "numCharged", "IsHadronicTau"] \
        + [var + "_vis" for var in ["pt", "eta", "phi", "m"]] \
        + ["decay_vertex_" + var for var in ["x", "y", "z"]]

    return truthtau_branches.get_output_list()
