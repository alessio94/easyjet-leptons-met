from EasyjetHub.output.ttree.branch_manager import BranchManager


def get_truthmuon_branches(flags, input_container, output_prefix):
    truthmuon_branches = BranchManager(
        input_container,
        output_prefix,
    )

    truthmuon_branches.variables = ["px", "py", "pz", "e", "m", "charge"]

    return truthmuon_branches.get_output_list()
