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

    if tree_flags.collection_options.muons.iso_variables:
        muon_branches.variables += [
            "neflowisol20",
            "neflowisol20_CloseByCorr",
            "topoetcone20",
            "topoetcone20_CloseByCorr",
            "ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500",
            "ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500_CloseByCorr",
            "ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000",
            "ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000_CloseByCorr"
        ]

    if flags.Analysis.do_overlap_removal:
        muon_branches.variables += ["passesOR_%SYS%"]

    muon_branches.variables += [
        "d0_NOSYS", "d0sig_NOSYS", "z0sintheta_NOSYS", "z0sinthetasig_NOSYS"
    ]

    if flags.Analysis.Muon.do_reco_decoration:
        muon_branches.variables += ["author", "muonType"]

    if flags.Analysis.Muon.MergeLRT:
        muon_branches.variables += ["isLRT"]

    if flags.Analysis.Muon.do_track_decoration:
        muon_branches.variables += [
            "idtrack_pt",
            "idtrack_eta",
            "idtrack_phi",
            "idtrack_d0",
            "idtrack_z0",
            "idtrack_chi2OverDoF",
            "idtrack_nIBL",
            "idtrack_nPIX",
            "idtrack_nPIX_shared",
            "idtrack_nSCT",
            "idtrack_nSCT_shared",
            "cbtrack_d0",
            "cbtrack_z0",
            "cbtrack_chi2OverDoF",
            "numberOfPrecisionLayers",
            "numberOfPrecisionHoleLayers"
        ]

        if (flags.Input.ProcessingTags == ["StreamDAOD_LLP1"]):
            muon_branches.variables += [
                "idtrack_nNextToIBL",
                "idtrack_nPIX_split",
                "idtrack_nTRT"
            ]

    id_wps = [f'{flags.Analysis.Muon.ID}_{flags.Analysis.Muon.Iso}']
    consider_extra_wp = any(len(wp) == 4 for wp in flags.Analysis.Muon.extra_wps)

    if 'extra_wps' in flags.Analysis.Muon:
        for wp in flags.Analysis.Muon.extra_wps:
            if len(wp) == 4:
                id_wps.append(
                    (
                        wp[0] + "_" + wp[1] + "_"
                        + str(wp[2]) + "_" + str(wp[3])
                    ).replace('.', 'p')
                )
            elif consider_extra_wp:
                id_wps.append(
                    (
                        wp[0] + "_" + wp[1] + "_"
                        + str(flags.Analysis.Muon.maxD0Significance) + "_"
                        + str(flags.Analysis.Muon.maxDeltaZ0SinTheta)
                    ).replace('.', 'p')
                )
            else:
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

    if flags.Input.isMC:
        muon_branches.variables += [
            f"effSF_{id_wp}_%SYS%"
            for id_wp in id_wps
        ]

    # Requires MuonSelectorAlg to be run
    if tree_flags.collection_options.muons.run_selection:
        muon_branches.variables += ["isAnalysisMuon_%SYS%"]
        for index in range(flags.Analysis.Muon.amount):
            muon_branches.variables += [f"isMuon{index+1}_%SYS%"]
        for index in range(flags.Analysis.Lepton.amount):
            muon_branches.variables += [f"isLepton{index+1}_%SYS%"]

    return muon_branches.get_output_list()
