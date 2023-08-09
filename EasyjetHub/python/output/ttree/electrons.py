from EasyjetHub.output.ttree.branch_manager import BranchManager, SystOption


def get_electron_branches(flags, tree_flags, input_container, output_prefix):
    _syst_option = SystOption.ALL_SYST
    if flags.Analysis.disable_calib:
        _syst_option = SystOption.NONE

    electron_branches = BranchManager(
        input_container,
        output_prefix,
        do_overlap_removal=flags.Analysis.do_overlap_removal,
        systematics_option=_syst_option,
    )

    if tree_flags.write_object_systs_only_for_pt:
        electron_branches.syst_only_for = ["pt"]

    electron_branches.add_four_mom_branches(do_mass=False)
    electron_branches.variables += ["charge"]

    return electron_branches.get_output_list()
