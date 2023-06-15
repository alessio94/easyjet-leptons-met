from EasyjetHub.output.ttree.branch_manager import BranchManager, SystOption


def get_electron_branches(flags, input_container, output_prefix):
    _syst_option = SystOption.ALL_SYST
    if flags.Analysis.disable_calib:
        _syst_option = SystOption.NONE

    electron_branches = BranchManager(
        input_container,
        output_prefix,
        do_overlap_removal=flags.Analysis.do_overlap_removal,
        systematics_option=_syst_option,
    )

    electron_branches.add_four_mom_branches(do_mass=False)

    return electron_branches.get_output_list()
