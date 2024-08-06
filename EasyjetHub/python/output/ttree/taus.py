from EasyjetHub.output.ttree.branch_manager import BranchManager, SystOption
from EasyjetHub.steering.sample_metadata import get_valid_ami_tag
from EasyjetHub.output.ttree.truth_taus import get_TopHiggs_tau_truth_labels


def get_tau_branches(flags, tree_flags, input_container, output_prefix):
    _syst_option = SystOption.ALL_SYST
    if flags.Analysis.disable_calib:
        _syst_option = SystOption.NONE

    tau_branches = BranchManager(
        input_container,
        output_prefix,
        systematics_option=_syst_option,
        systematics_suffix_separator=flags.Analysis.systematics_suffix_separator
    )

    if tree_flags.slim_variables_with_syst:
        tau_branches.syst_only_for = ["pt"]

    tau_branches.add_four_mom_branches(do_mass=False)
    tau_branches.variables += ["charge", "nProng", "decayMode"]
    if flags.Analysis.do_bbtt_analysis:
        tau_branches.variables += ["isIDTau", "isAntiTau"]

    if flags.Analysis.do_overlap_removal:
        tau_branches.variables += ["passesOR_%SYS%"]

    if flags.Input.isMC:
        for tau_id in [flags.Analysis.Tau.ID]:
            tau_branches.variables += [f"tau_Reco_effSF_{tau_id}_%SYS%",
                                       f"tau_ID_effSF_{tau_id}_%SYS%"]
            if "noeleid" not in tau_id:
                tau_branches.variables += [f"tau_EvetoFakeTau_effSF_{tau_id}_%SYS%",
                                           f"tau_EvetoTrueTau_effSF_{tau_id}_%SYS%"]

    if tree_flags.collection_options.taus.score_branches:
        tau_branches.variables += [
            "RNNJetScoreSigTrans",
            "RNNEleScoreSigTrans"
        ]

        split_tags = flags.Input.AMITag.split("_")
        gntau_valid_ptag = (
            get_valid_ami_tag(split_tags, "p", "p6266") and not flags.Input.isPHYSLITE)
        if gntau_valid_ptag:
            tau_branches.variables += [
                "GNTauScoreSigTrans_v0"
            ]

    if flags.Input.isMC and tree_flags.collection_options.taus.truth_branches:
        tau_branches.variables += [
            "truth_pt_vis",
            "truth_eta_vis",
            "truth_phi_vis",
            "truth_m_vis",
            "truth_pdgId",
            "truth_IsHadronicTau",
            "truthType"
        ]

    if flags.Input.isMC and tree_flags.collection_options.taus.truth_parent_info:
        tau_branches.variables += get_TopHiggs_tau_truth_labels(flags)

    if tree_flags.collection_options.taus.run_selection:
        tau_branches.variables += ["isAnalysisTau_%SYS%"]
        for index in range(flags.Analysis.Tau.amount):
            tau_branches.variables += [f"isTau{index+1}_%SYS%"]

    return tau_branches.get_output_list()
