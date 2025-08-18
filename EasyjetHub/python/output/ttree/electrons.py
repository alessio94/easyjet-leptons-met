from EasyjetHub.output.ttree.branch_manager import BranchManager, SystOption


def get_electron_branches(flags, tree_flags, input_container, output_prefix):
    _syst_option = SystOption.ALL_SYST
    if flags.Analysis.disable_calib:
        _syst_option = SystOption.NONE

    el_output_flags = tree_flags.collection_options.electrons

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

    if el_output_flags.iso_variables:
        electron_branches.variables += [
            "topoetcone20",
            "ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000"
        ]

    if el_output_flags.id_variables:
        if not flags.Input.isPHYSLITE:
            electron_branches.variables += [
                "DFCommonElectronsDNN_pel",
                "DFCommonElectronsDNN_pcf",
                "DFCommonElectronsDNN_ppc",
                "DFCommonElectronsDNN_phf",
                "DFCommonElectronsDNN_ple",
                "DFCommonElectronsDNN_plh"
            ]

    if flags.Analysis.Electron.do_reco_decoration:
        electron_branches.variables += ["author"]

    if flags.Analysis.do_overlap_removal:
        electron_branches.variables += ["passesOR_%SYS%"]

    electron_branches.variables += [
        "d0_NOSYS", "d0sig_NOSYS", "z0sintheta_NOSYS", "z0sinthetasig_NOSYS"
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
        electron_branches.variables += [
            "IFFClass_NOSYS",
            "truthType",
            "truthOrigin",
            "firstEgMotherTruthType",
            "firstEgMotherTruthOrigin",
            "firstEgMotherPdgId"
        ]

    if flags.Input.isMC and el_output_flags.truth_parent_info:
        truth_labels = []
        if not flags.Input.isPHYSLITE:
            truth_labels += [
                *[f"parent{p}ParentsMask" for p in ["Higgs", "Z", "Top"]],
            ]
        electron_branches.variables += truth_labels

    if flags.Input.isMC:
        electron_branches.variables += [
            f"effSF_{id_wp}_%SYS%"
            for id_wp in id_wps
            if not ("DNN" in id_wp or "NoPix" in id_wp)
        ]

    # Requires ElectronSelectorAlg to be run
    if el_output_flags.run_selection:
        electron_branches.variables += ["isAnalysisElectron_%SYS%"]
        for index in range(flags.Analysis.Electron.amount):
            electron_branches.variables += [f"isElectron{index+1}_%SYS%"]
        for index in range(flags.Analysis.Lepton.amount):
            electron_branches.variables += [f"isLepton{index+1}_%SYS%"]

    electron_branches.variables += el_output_flags.extra_variables
    if flags.Input.isMC:
        electron_branches.variables += el_output_flags.mc_extra_variables

    return electron_branches.get_output_list()
