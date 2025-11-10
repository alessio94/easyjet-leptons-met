from EasyjetHub.output.ttree.branch_manager import BranchManager, SystOption


def get_ditau_branches(flags, tree_flags, input_container, output_prefix):
    _syst_option = SystOption.ALL_SYST
    if flags.Analysis.disable_calib:
        _syst_option = SystOption.NONE

    ditau_branches = BranchManager(
        input_container,
        output_prefix,
        systematics_option=_syst_option,
        systematics_suffix_separator=flags.Analysis.systematics_suffix_separator
    )

    if tree_flags.slim_variables_with_syst:
        ditau_branches.syst_only_for = ["pt"]

    ditau_branches.add_four_mom_branches(do_mass=True)

    id_wps = [flags.Analysis.DiTau.ID]
    if 'extra_wps' in flags.Analysis.DiTau:
        id_wps += flags.Analysis.DiTau.extra_wps

    ditau_branches.variables += [
        f"baselineSelection_{id_wp}_%SYS%"
        for id_wp in id_wps
    ]

    if flags.Analysis.DiTau.score_branches:
        ditau_branches.variables += ["omni_score"]

    return ditau_branches.get_output_list()
