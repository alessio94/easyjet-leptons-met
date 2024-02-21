from EasyjetHub.output.ttree.branch_manager import BranchManager, SystOption
from EasyjetHub.output.ttree.truth_jets import get_small_R_jet_truth_labels
from EasyjetHub.steering.sample_metadata import get_valid_ami_tag


def get_small_R_jet_branches(
    flags, tree_flags, input_container, output_prefix
):
    _syst_option = SystOption.ALL_SYST
    if flags.Analysis.disable_calib:
        _syst_option = SystOption.NONE

    small_R_jet_branches = BranchManager(
        input_container,
        output_prefix,
        do_overlap_removal=flags.Analysis.do_overlap_removal,
        systematics_option=_syst_option,
        required_flags=[
            flags.Analysis.do_small_R_jets
        ],
    )

    if tree_flags.slim_variables_with_syst:
        small_R_jet_branches.syst_only_for = ["pt"]

    small_R_jet_branches.add_four_mom_branches(do_mass=True)

    if flags.Analysis.small_R_jet.jet_type != "reco4EMTopoJet":
        small_R_jet_branches.variables += ["NNJvtPass"]

        if tree_flags.collection_options.small_R_jets.btag_info:
            btag_wps = [flags.Analysis.small_R_jet.btag_wp]
            if 'btag_extra_wps' in flags.Analysis.small_R_jet:
                btag_wps += flags.Analysis.small_R_jet.btag_extra_wps
            small_R_jet_branches.variables += [
                f"ftag_select_{btag_wp}"
                for btag_wp in btag_wps
            ]
            if flags.Input.isMC:
                small_R_jet_branches.variables += [
                    f"ftag_effSF_{btag_wp}_%SYS%" for btag_wp in btag_wps
                ]

        if (
            tree_flags.collection_options.small_R_jets.no_bjet_calib_p4
            and flags.Analysis.do_muons
        ):
            small_R_jet_branches.variables += [
                f"NoBJetCalibMomentum_{var}"
                for var in ["pt", "eta", "phi", "m"]
            ]

        if tree_flags.collection_options.small_R_jets.JVT_details:
            small_R_jet_branches.variables += [
                "Jvt",
                "JvtRpt",
                "JVFCorr",
                "jvt_selection",
                "NNJvt",
                "NNJvtRpt",
            ]

            if flags.Input.isMC:
                small_R_jet_branches.variables += ["jvt_effSF_%SYS%"]

    if (
        flags.Input.isMC
        and tree_flags.collection_options.small_R_jets.higgs_parent_info
    ):
        small_R_jet_branches.variables += get_small_R_jet_truth_labels(flags)

    return small_R_jet_branches.get_output_list()


def get_small_R_bjet_branches(
    flags, tree_flags, input_container, output_prefix
):
    _syst_option = SystOption.ALL_SYST
    if flags.Analysis.disable_calib:
        _syst_option = SystOption.NONE

    small_R_bjet_branches = BranchManager(
        input_container,
        output_prefix,
        do_overlap_removal=flags.Analysis.do_overlap_removal,
        systematics_option=_syst_option,
        required_flags=[
            flags.Analysis.do_small_R_jets
        ],
    )

    # ftag scores pb, pc, pl
    if tree_flags.collection_options.small_R_jets.btag_details:
        small_R_bjet_branches.variables += [
            "DL1dv01_pb",
            "DL1dv01_pc",
            "DL1dv01_pu"
        ]

        split_tags = flags.Input.AMITag.split("_")
        gn2_valid_ptag = get_valid_ami_tag(split_tags, "p", "p5855")
        if gn2_valid_ptag:
            small_R_bjet_branches.variables += [
                "GN2v00_pb",
                "GN2v00_pc",
                "GN2v00_pu",
            ]

        if flags.Analysis.small_R_jet.doBtagExpVars:
            small_R_bjet_branches.variables += [
                "GN2v00_Db",
                "GN2v00_pcbtExp"
            ]

    return small_R_bjet_branches.get_output_list()
