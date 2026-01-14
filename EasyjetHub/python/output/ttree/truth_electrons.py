from EasyjetHub.output.ttree.branch_manager import BranchManager


def get_truthelectron_branches(flags, input_container, output_prefix):
    truthelectron_branches = BranchManager(
        input_container,
        output_prefix,
    )

    truthelectron_branches.variables = ["px", "py", "pz", "e", "m", "pdgId"]

    return truthelectron_branches.get_output_list()
