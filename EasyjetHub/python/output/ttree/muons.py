from EasyjetHub.output.ttree.branch_manager import BranchManager, SystOption


def get_muon_branches(flags, input_container, output_prefix):
    _syst_option = SystOption.ALL_SYST
    if flags.Analysis.disable_calib:
        _syst_option = SystOption.NONE

    muon_branches = BranchManager(
        input_container,
        output_prefix,
        do_overlap_removal=flags.Analysis.do_overlap_removal,
        systematics_option=_syst_option,
    )

    muon_branches.add_four_mom_branches(do_mass=False)

    return muon_branches.get_output_list()
