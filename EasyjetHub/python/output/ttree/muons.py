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

    if tree_flags.collection_options.muons.id_iso_variables:
        id_wps = [f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}']
        if 'extra_wps' in flags.Analysis.Muon:
            for wp in flags.Analysis.Muon.extra_wps:
                id_wps.append(wp[0] + "_" + wp[1])

        muon_branches.variables += [
            f"baselineSelection_{id_wp}_%SYS%"
            for id_wp in id_wps
        ]

    return muon_branches.get_output_list()
