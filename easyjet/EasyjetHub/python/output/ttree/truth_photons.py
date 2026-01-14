from EasyjetHub.output.ttree.branch_manager import BranchManager


def get_truthphoton_branches(flags, input_container, output_prefix):
    truthphoton_branches = BranchManager(
        input_container,
        output_prefix,
    )

    truthphoton_branches.variables = ["px", "py", "pz", "e"]

    return truthphoton_branches.get_output_list()
