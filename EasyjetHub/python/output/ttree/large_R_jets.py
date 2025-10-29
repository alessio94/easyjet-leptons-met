from EasyjetHub.output.ttree.branch_manager import BranchManager, SystOption
from EasyjetHub.output.ttree.truth_jets import get_TopHiggs_jet_truth_labels
from EasyjetHub.steering.sample_metadata import get_valid_ami_tag


def get_large_R_jet_branches(
    flags, tree_flags,
    input_container,
    output_prefix,
    lr_jet_type
):
    _syst_option = SystOption.ALL_SYST
    if flags.Analysis.disable_calib:
        _syst_option = SystOption.NONE

    jet_output_flags = tree_flags.collection_options.large_R_jets

    large_R_jet_branches = BranchManager(
        input_container,
        output_prefix,
        systematics_option=_syst_option,
        systematics_suffix_separator=flags.Analysis.systematics_suffix_separator
    )

    if tree_flags.slim_variables_with_syst:
        large_R_jet_branches.syst_only_for = ["pt", "m"]

    if lr_jet_type == "Topo":
        large_R_jet_branches.required_flags.append(flags.Analysis.do_large_R_Topo_jets)
    if lr_jet_type == "UFO":
        large_R_jet_branches.required_flags.append(flags.Analysis.do_large_R_UFO_jets)

    large_R_jet_branches.add_four_mom_branches(do_mass=True)

    # Similar run_selection as small_R_jets to avoid ordering issues
    if jet_output_flags.run_selection:
        large_R_jet_branches.variables += ["isAnalysisJet_%SYS%"]
        for index in range(flags.Analysis.Large_R_jet.amount_leadingjet):
            large_R_jet_branches.variables += [f"isjet{index+1}_%SYS%"]
    else:
        if flags.Analysis.do_overlap_removal:
            large_R_jet_branches.variables += ["passesOR_%SYS%"]

    if flags.Analysis.Large_R_jet.runMuonJetPtCorr:
        if jet_output_flags.no_bjet_calib_p4:
            large_R_jet_branches.variables += ["n_muons_%SYS%"]

            large_R_jet_branches.variables += [
                f"NoBJetCalibMomentum_{var}"
                for var in ["pt", "eta", "phi", "m"]
            ]
            if tree_flags.slim_variables_with_syst:
                large_R_jet_branches.syst_only_for += [
                    "NoBJetCalibMomentum_pt",
                    "NoBJetCalibMomentum_m",
                ]

    if flags.Input.isMC and jet_output_flags.truth_labels:
        large_R_jet_branches.variables += [
            "GhostBHadronsFinalCount",
        ] + get_large_R_jet_truth_labels(flags)

        if lr_jet_type == "Topo":
            large_R_jet_branches.variables += [
                "R10TruthLabel_R21Consolidated",
            ]
        if lr_jet_type == "UFO":
            large_R_jet_branches.variables += [
                "GhostCHadronsFinalCount",
                "R10TruthLabel_R21Precision_2022v1",
                "R10TruthLabel_R22v1",
            ]

    if jet_output_flags.substructure_info:
        large_R_jet_branches.variables += get_substructure_branches(flags, lr_jet_type)

    if lr_jet_type == "UFO" and \
        flags.Analysis.Large_R_jet.wtag_type and \
            flags.Analysis.Large_R_jet.wtag_wp:
        large_R_jet_branches.variables += get_wtag_branches(flags)

    split_tags = flags.Input.AMITag.split("_")
    is_valid_ptag = get_valid_ami_tag(split_tags, "p", "p5834")
    is_valid_for_v02 = get_valid_ami_tag(split_tags, "p", "p6490")
    is_valid_for_bjr_v01 = get_valid_ami_tag(split_tags, "p", "p6697")
    is_valid_for_gn3x = get_valid_ami_tag(split_tags, "p", "p7017")
    if lr_jet_type == "UFO" and is_valid_ptag:
        if jet_output_flags.btag_details:
            large_R_jet_branches.variables += get_large_R_gn2_branches(
                is_valid_for_v02
            )
        if flags.Analysis.Large_R_jet.GN2X_hbb_wps:
            large_R_jet_branches.variables += get_large_R_gn2_tag_branches(flags)
    if (
        lr_jet_type == "UFO"
        and is_valid_for_bjr_v01
        and jet_output_flags.bjet_regression
    ):
        large_R_jet_branches.variables += [
            "bJR10v00_pt",
            "bJR10v00_mass",
            "bJR10v01_pt",
            "bJR10v01_mass",
        ]

    if (
        lr_jet_type == "UFO"
        and is_valid_for_gn3x
        and jet_output_flags.btag_details
    ):
        large_R_jet_branches.variables += [
            "GN3XPV01_ptop",
            "GN3XPV01_pqcdbx",
            "GN3XPV01_phcc",
            "GN3XPV01_pqcdcx",
            "GN3XPV01_pWqq",
            "GN3XPV01_phbb",
            "GN3XPV01_pqcdll",
            "GN3XPV01_pqcdbb",
        ]
    large_R_jet_branches.variables += jet_output_flags.extra_variables
    if flags.Input.isMC:
        large_R_jet_branches.variables += jet_output_flags.mc_extra_variables

    return large_R_jet_branches.get_output_list()


def get_substructure_branches(flags, lr_jet_type):
    substructure_vars = [
        "Tau1_wta",
        "Tau2_wta",
        "Tau3_wta",
        "ECF1",
        "ECF2",
        "ECF3",
        "Split12",
        "Split23",
    ]
    # Trimmed jet vars
    if lr_jet_type == "Topo":
        substructure_vars += [
            "NTrimSubjets",
            "TrackSumPt",
        ]
    if flags.Input.isPHYSLITE:
        substructure_vars += [
            "D2",
        ]
    return substructure_vars


def get_large_R_gn2_branches(is_valid_for_v02):
    gn2_branches = [
        "GN2Xv01_phbb",
        "GN2Xv01_phcc",
        "GN2Xv01_ptop",
        "GN2Xv01_pqcd",
    ]
    if is_valid_for_v02:
        gn2_branches += [
            "GN2Xv02_phbb",
            "GN2Xv02_phcc",
            "GN2Xv02_ptop",
            "GN2Xv02_pqcd",
        ]
    return gn2_branches


def get_large_R_gn2_tag_branches(flags):
    gn2_tag_branches = []
    for wp in flags.Analysis.Large_R_jet.GN2X_hbb_wps:
        gn2_tag_branches += ["xbb_select_GN2Xv01_" + wp]
        # Disable SF for now to avoid warnings with preliminary json
        # if flags.Input.isMC:
        #    gn2_tag_branches += ["xbb_effSF_GN2Xv01_" + wp + "_NOSYS"]
    return gn2_tag_branches


def get_large_R_jet_truth_labels(flags):
    parent_bosons = ["Higgs", "Scalar", "Top"]

    truth_labels = []
    if not flags.Input.isPHYSLITE and flags.Analysis.Large_R_jet.do_parent_decoration:
        truth_labels += [
            f"parent{p}NMatchedChildren" for p in parent_bosons
        ]

    truth_labels += get_TopHiggs_jet_truth_labels(flags)

    return truth_labels


def get_wtag_branches(flags):
    wtag_branches = []

    wtag_type = flags.Analysis.Large_R_jet.wtag_type
    wtag_wp = flags.Analysis.Large_R_jet.wtag_wp
    wtag_extra_wps = list(flags.Analysis.Large_R_jet.wtag_extra_wps or [])

    for wp in [wtag_wp] + wtag_extra_wps:
        wtag_branches += [
            f"{wtag_type}{wp}Tagger_Tagged",
            f"{wtag_type}{wp}Tagger_Score",
        ]

    return wtag_branches
