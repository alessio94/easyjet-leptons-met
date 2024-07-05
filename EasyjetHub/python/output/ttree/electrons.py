from EasyjetHub.output.ttree.branch_manager import BranchManager, SystOption


def get_electron_branches(flags, tree_flags, input_container, output_prefix):
    _syst_option = SystOption.ALL_SYST
    if flags.Analysis.disable_calib:
        _syst_option = SystOption.NONE

    electron_branches = BranchManager(
        input_container,
        output_prefix,
        systematics_option=_syst_option,
        systematics_suffix_separator=flags.Analysis.systematics_suffix_separator
    )

    if tree_flags.slim_variables_with_syst:
        electron_branches.syst_only_for = ["pt"]

    electron_branches.add_four_mom_branches(do_mass=False)
    electron_branches.variables += ["charge"]

    if flags.Analysis.do_overlap_removal:
        electron_branches.variables += ["passesOR_%SYS%"]

    if tree_flags.collection_options.electrons.id_iso_variables:
        id_wps = [f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}']
        if 'extra_wps' in flags.Analysis.Electron:
            for wp in flags.Analysis.Electron.extra_wps:
                id_wps.append(wp[0] + "_" + wp[1])

        electron_branches.variables += [
            f"baselineSelection_{id_wp}_%SYS%"
            for id_wp in id_wps
        ]

    if tree_flags.collection_options.electrons.run_selection:
        electron_branches.variables += ["isAnalysisElectron_%SYS%"]
        for index in range(flags.Analysis.Lepton.amount):
            electron_branches.variables += [f"isElectron{index+1}_%SYS%"]

    return electron_branches.get_output_list()
