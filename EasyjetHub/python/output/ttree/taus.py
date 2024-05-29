from EasyjetHub.output.ttree.branch_manager import BranchManager, SystOption


def get_tau_branches(flags, tree_flags, input_container, output_prefix):
    _syst_option = SystOption.ALL_SYST
    if flags.Analysis.disable_calib:
        _syst_option = SystOption.NONE

    tau_branches = BranchManager(
        input_container,
        output_prefix,
        do_overlap_removal=flags.Analysis.do_overlap_removal,
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
            tau_branches.variables += [f"tau_effSF_{tau_id}_%SYS%"]

    if tree_flags.collection_options.taus.RNN_branches:
        tau_branches.variables += [
            "RNNJetScoreSigTrans",
            "RNNEleScoreSigTrans"
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

    return tau_branches.get_output_list()
