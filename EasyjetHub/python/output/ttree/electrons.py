from EasyjetHub.output.ttree.branch_manager import BranchManager, SystOption
from AthenaConfiguration.Enums import LHCPeriod


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

    if flags.Analysis.Electron.do_reco_decoration:
        electron_branches.variables += ["author"]

    if flags.Analysis.do_overlap_removal:
        electron_branches.variables += ["passesOR_%SYS%"]

    electron_branches.variables += [
        "d0sig_NOSYS", "z0sintheta_NOSYS"
    ]

    if flags.Analysis.Electron.MergeLRT:
        electron_branches.variables += ["isLRT"]

    id_wps = [f'{flags.Analysis.Electron.ID}_{flags.Analysis.Electron.Iso}']

    if 'extra_wps' in flags.Analysis.Electron:
        for wp in flags.Analysis.Electron.extra_wps:
            id_wps.append(wp[0] + "_" + wp[1])

    electron_branches.variables += [
        f"baselineSelection_{id_wp}_%SYS%"
        for id_wp in id_wps
    ]

    if flags.Analysis.Electron.do_track_decoration:
        electron_branches.variables += [
            "mllConv",
            "mllConvAtConvV",
            "radiusConv",
            "separationMinDCT",
        ]
        if not flags.Input.isPHYSLITE:
            electron_branches.variables += [
                "DFCommonAddAmbiguity",
                "ambiguityType"
            ]

    if flags.Input.isMC and \
       flags.Analysis.Electron.do_IFF_decoration:
        electron_branches.variables += ["IFFClass_NOSYS"]

    if flags.Input.isMC and \
       tree_flags.collection_options.electrons.truth_parent_info:
        truth_labels = []
        if not flags.Input.isPHYSLITE:
            truth_labels += [
                *[f"parent{p}ParentsMask" for p in ["Higgs", "Z", "Top"]],
            ]
        electron_branches.variables += truth_labels

    if flags.Input.isMC:
        # No Run 2 SF yet
        if flags.GeoModel.Run is LHCPeriod.Run3:
            electron_branches.variables += [
                f"effSF_{id_wp}_%SYS%"
                for id_wp in id_wps
                if not ("DNN" in id_wp or "NoPix" in id_wp)
            ]

    # Requires ElectronSelectorAlg to be run
    if tree_flags.collection_options.electrons.run_selection:
        electron_branches.variables += ["isAnalysisElectron_%SYS%"]
        for index in range(flags.Analysis.Electron.amount):
            electron_branches.variables += [f"isElectron{index+1}_%SYS%"]
        for index in range(flags.Analysis.Lepton.amount):
            electron_branches.variables += [f"isLepton{index+1}_%SYS%"]

    return electron_branches.get_output_list()
