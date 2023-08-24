from EasyjetHub.output.ttree.branch_manager import BranchManager, SystOption


def get_met_branches(flags, input_container, output_prefix):
    _syst_option = SystOption.ALL_SYST
    if flags.Analysis.disable_calib:
        _syst_option = SystOption.NONE

    met_branches = BranchManager(
        input_container,
        output_prefix,
        do_overlap_removal=False,
        systematics_option=_syst_option,
    )

    met_branches.variables += [
        "met", "phi", "sumet"
    ]

    return met_branches.get_output_list()
