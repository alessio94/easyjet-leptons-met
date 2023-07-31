from EasyjetHub.output.ttree.branch_manager import BranchManager, SystOption
from EasyjetHub.output.ttree.truth_jets import get_small_R_jet_truth_labels


def get_small_R_jet_branches(flags, input_container, output_prefix):
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
        ]
    )

    if flags.Analysis.write_object_systs_only_for_pt:
        small_R_jet_branches.syst_only_for = ["pt"]

    small_R_jet_branches.add_four_mom_branches(do_mass=True)

    small_R_jet_branches.variables += ["NNJvtPass"]

    if flags.Analysis.write_small_R_btag:
        small_R_jet_branches.variables += [
            f"ftag_select_{btag_wp}"
            for btag_wp in flags.Analysis.btag_wps
        ]

    if (
        flags.Analysis.write_small_R_no_bjet_calib
        and flags.Analysis.do_muons
    ):
        small_R_jet_branches.variables += [
            f"NoBJetCalibMomentum_{var}"
            for var in ["pt", "eta", "phi", "m"]
        ]

    if flags.Analysis.write_small_R_JVT_details:
        small_R_jet_branches.variables += [
            "Jvt",
            "JvtRpt",
            "JVFCorr",
            "jvt_selection",
            "NNJvt",
            "NNJvtRpt",
        ]

    if flags.Input.isMC and flags.Analysis.write_small_R_higgs_parent_info:
        small_R_jet_branches.variables += get_small_R_jet_truth_labels(flags)

    return small_R_jet_branches.get_output_list()


def get_small_R_bjet_branches(flags, input_container, output_prefix):
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
        ]
    )

    # GN2 scores
    # not available in PHYSLITE... yet
    # TODO: Handle this properly (drop PHYSLITE check when ptag updated)
    if (
        flags.Analysis.write_small_R_gn2_branches
        and not flags.Input.isPHYSLITE
    ):
        small_R_bjet_branches.variables += [
            "GN2v00_pb",
            "GN2v00_pc",
            "GN2v00_pu",
            "DL1dv01_pb",
            "DL1dv01_pc",
            "DL1dv01_pu",
            "DL1r_pb",
            "DL1r_pc",
            "DL1r_pu",
        ]
    return small_R_bjet_branches.get_output_list()
