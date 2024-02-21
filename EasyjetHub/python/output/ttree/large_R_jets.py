from EasyjetHub.output.ttree.branch_manager import BranchManager, SystOption
from EasyjetHub.output.ttree.truth_jets import get_large_R_jet_truth_labels
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

    large_R_jet_branches = BranchManager(
        input_container,
        output_prefix,
        do_overlap_removal=flags.Analysis.do_overlap_removal,
        systematics_option=_syst_option,
    )

    if tree_flags.slim_variables_with_syst:
        large_R_jet_branches.syst_only_for = ["pt"]

    # The LargeJetGhostVRJetAssociationAlg does not support systematics
    # Implement systematics handles, then remove this extra BM
    large_R_jet_NOSYS_branches = BranchManager(
        input_container,
        output_prefix,
        do_overlap_removal=flags.Analysis.do_overlap_removal,
        systematics_option=SystOption.NO_SYST,
    )

    if lr_jet_type == "Topo":
        large_R_jet_branches.required_flags.append(flags.Analysis.do_large_R_Topo_jets)
    if lr_jet_type == "UFO":
        large_R_jet_branches.required_flags.append(flags.Analysis.do_large_R_UFO_jets)

    large_R_jet_branches.add_four_mom_branches(do_mass=True)

    if flags.Input.isMC and tree_flags.collection_options.large_R_jets.truth_labels:
        large_R_jet_branches.variables += [
            "GhostBHadronsFinalCount",
        ] + get_large_R_jet_truth_labels(flags)

        if lr_jet_type == "Topo":
            large_R_jet_NOSYS_branches.variables += [
                "R10TruthLabel_R21Consolidated",
            ]
        if lr_jet_type == "UFO":
            large_R_jet_NOSYS_branches.variables += [
                "GhostCHadronsFinalCount",
                "R10TruthLabel_R21Precision_2022v1",
                "R10TruthLabel_R22v1",
            ]

    if tree_flags.reco_outputs.VR_jets:
        large_R_jet_branches.variables += get_ghost_vr_branches(flags)

    if tree_flags.collection_options.large_R_jets.substructure_info:
        large_R_jet_branches.variables += get_substructure_branches(flags, lr_jet_type)

    split_tags = flags.Input.AMITag.split("_")
    is_valid_ptag = get_valid_ami_tag(split_tags, "p", "p5657")
    if lr_jet_type == "UFO" and is_valid_ptag:
        large_R_jet_branches.variables += get_large_R_gn2_branches()

    return (
        large_R_jet_branches.get_output_list()
        + large_R_jet_NOSYS_branches.get_output_list()
    )


def get_ghost_vr_branches(flags):
    vr_vars = [
        "goodVRTrackJets",
        "minRelativeDeltaRToVRJet",
        "leadingVRTrackJetsPt",
        "leadingVRTrackJetsEta",
        "leadingVRTrackJetsPhi",
        "leadingVRTrackJetsM",
        "leadingVRTrackJetsDeltaR12",
        "leadingVRTrackJetsDeltaR13",
        "leadingVRTrackJetsDeltaR32",
    ] + [
        f"leadingVRTrackJetsBtag_{wp}"
        for wp in flags.Analysis.large_R_jet.vr_btag_wps
    ]
    if flags.Input.isMC:
        vr_vars += [
            "VRTrackJetsTruthLabel"
            # previously leadingVRTrackJets_HadronConeExclTruthLabelID
        ]
    return vr_vars


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


def get_large_R_gn2_branches():
    gn2_branches = [
        "GN2Xv01_phbb",
        "GN2Xv01_phcc",
        "GN2Xv01_ptop",
        "GN2Xv01_pqcd",
    ]
    return gn2_branches
