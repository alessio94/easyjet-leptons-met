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

    if flags.Analysis.write_object_systs_only_for_pt:
        muon_branches.syst_only_for = ["pt"]

    muon_branches.add_four_mom_branches(do_mass=False)
    muon_branches.variables += ["charge"]

    return muon_branches.get_output_list()
