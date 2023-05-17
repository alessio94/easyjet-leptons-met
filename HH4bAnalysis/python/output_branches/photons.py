from HH4bAnalysis.output_branches.branch_manager import BranchManager, SystOption


def get_photon_branches(flags, input_container, output_prefix):
    _syst_option = SystOption.ALL_SYST
    if flags.Analysis.disable_calib:
        _syst_option = SystOption.NONE

    photon_branches = BranchManager(
        input_container,
        output_prefix,
        do_overlap_removal=flags.Analysis.do_overlap_removal,
        systematics_option=_syst_option,
    )

    photon_branches.add_four_mom_branches(do_mass=False)

    return photon_branches.get_output_list()
