from EasyjetHub.output.ttree.branch_manager import BranchManager, SystOption


def get_muon_branches(flags, tree_flags, input_container, output_prefix):
    _syst_option = SystOption.ALL_SYST
    if flags.Analysis.disable_calib:
        _syst_option = SystOption.NONE

    muon_branches = BranchManager(
        input_container,
        output_prefix,
        do_overlap_removal=flags.Analysis.do_overlap_removal,
        systematics_option=_syst_option,
        systematics_suffix_separator=flags.Analysis.systematics_suffix_separator
    )

    if tree_flags.slim_variables_with_syst:
        muon_branches.syst_only_for = ["pt"]

    muon_branches.add_four_mom_branches(do_mass=False)
    muon_branches.variables += ["charge"]

    if flags.Analysis.do_overlap_removal:
        muon_branches.variables += ["passesOR_%SYS%"]

    return muon_branches.get_output_list()
