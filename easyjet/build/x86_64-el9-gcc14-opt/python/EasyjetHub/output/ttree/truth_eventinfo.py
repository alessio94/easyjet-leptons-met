from EasyjetHub.output.ttree.branch_manager import BranchManager


def get_truth_event_info_branches(flags):

    truth_eventinfo_branches = BranchManager(
        input_container="TruthEvents",
        output_prefix="truth",
        variables=[
            "PDFID1", "PDFID2",
            "PDGID1", "PDGID2",
            "Q",
            "X1", "X2"
        ]
    )

    return truth_eventinfo_branches.get_output_list()
