from EasyjetHub.output.ttree.branch_manager import BranchManager


def get_common_jet_truth_labels(flags):
    truth_labels = []
    if not flags.Input.isPHYSLITE:
        truth_labels += [
            *[f"nTopTo{p}Children" for p in "BW"],
            *[f"parent{p}ParentsMask" for p in ["Higgs", "Scalar", "Top"]],
        ]

    return truth_labels


def get_small_R_jet_truth_labels(flags):
    # Add the standard b-tagging truth labels
    truth_labels = [
        "HadronConeExclTruthLabelID",
    ]
    truth_labels += get_common_jet_truth_labels(flags)

    return truth_labels


def get_large_R_jet_truth_labels(flags):
    parent_bosons = ["Higgs", "Scalar", "Top"]

    truth_labels = []
    if not flags.Input.isPHYSLITE:
        truth_labels += [
            f"parent{p}NMatchedChildren" for p in parent_bosons
        ]

    truth_labels += get_common_jet_truth_labels(flags)

    return truth_labels


def get_small_R_truthjet_branches(flags, input_container, output_prefix):
    small_R_truthjet_branches = BranchManager(
        input_container,
        output_prefix,
        do_overlap_removal=False,
    )

    small_R_truthjet_branches.add_four_mom_branches(do_mass=True)

    small_R_truthjet_branches.variables += [
        "PartonTruthLabelID",
        "HadronConeExclTruthLabelID",
    ]
    return small_R_truthjet_branches.get_output_list()


def get_large_R_truthjet_branches(flags, input_container, output_prefix):
    large_R_truthjet_branches = BranchManager(
        input_container,
        output_prefix,
        do_overlap_removal=False,
    )

    large_R_truthjet_branches.add_four_mom_branches(do_mass=True)

    large_R_truthjet_branches.variables += [
    ]
    return large_R_truthjet_branches.get_output_list()
