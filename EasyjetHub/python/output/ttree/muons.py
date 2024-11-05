from EasyjetHub.output.ttree.branch_manager import BranchManager, SystOption


def get_muon_branches(flags, tree_flags, input_container, output_prefix):
    _syst_option = SystOption.ALL_SYST
    if flags.Analysis.disable_calib:
        _syst_option = SystOption.NONE

    muon_branches = BranchManager(
        input_container,
        output_prefix,
        systematics_option=_syst_option,
        systematics_suffix_separator=flags.Analysis.systematics_suffix_separator
    )

    if tree_flags.slim_variables_with_syst:
        muon_branches.syst_only_for = ["pt"]

    muon_branches.add_four_mom_branches(do_mass=False)
    muon_branches.variables += ["charge"]

    if flags.Analysis.do_overlap_removal:
        muon_branches.variables += ["passesOR_%SYS%"]

    id_wps = [f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}']

    muon_branches.variables += [
        f"d0sig_{id_wps[0]}"
    ]
    muon_branches.variables += [
        f"z0sintheta_{id_wps[0]}"
    ]

    if 'extra_wps' in flags.Analysis.Muon:
        for wp in flags.Analysis.Muon.extra_wps:
            id_wps.append(wp[0] + "_" + wp[1])

    muon_branches.variables += [
        f"baselineSelection_{id_wp}_%SYS%"
        for id_wp in id_wps
    ]

    if flags.Input.isMC and \
       flags.Analysis.Muon.do_IFF_decoration:
        muon_branches.variables += ["IFFClass_NOSYS"]

    if flags.Input.isMC and \
       tree_flags.collection_options.muons.truth_parent_info:
        truth_labels = []
        if not flags.Input.isPHYSLITE:
            truth_labels += [
                *[f"parent{p}ParentsMask" for p in ["Higgs", "Z", "Top"]],
            ]
        muon_branches.variables += truth_labels

    # Requires MuonSelectorAlg to be run
    if tree_flags.collection_options.muons.run_selection:
        if flags.Input.isMC:
            muon_branches.variables += [
                f"muon_effSF_{id_wp}_%SYS%"
                for id_wp in id_wps
            ]

        muon_branches.variables += ["isAnalysisMuon_%SYS%"]
        for index in range(flags.Analysis.Lepton.amount):
            muon_branches.variables += [f"isMuon{index+1}_%SYS%"]
            muon_branches.variables += [f"isLepton{index+1}_%SYS%"]

    return muon_branches.get_output_list()
