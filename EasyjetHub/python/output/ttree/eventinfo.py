from EasyjetHub.output.ttree.branch_manager import BranchManager, SystOption


def get_event_info_branches(flags, tree_flags, do_PRW, trigger_chains):
    _syst_option = SystOption.ALL_SYST
    if flags.Analysis.disable_calib or not flags.Input.isMC:
        _syst_option = SystOption.NONE

    eventinfo_branches = BranchManager(
        input_container="EventInfo",
        output_prefix="",
        systematics_option=_syst_option,
        systematics_suffix_separator=flags.Analysis.systematics_suffix_separator,
        variables=[
            "runNumber",
            "eventNumber",
            "lumiBlock",
            "dataTakingYear",
            "averageInteractionsPerCrossing",
            "actualInteractionsPerCrossing",
        ]
    )
    if flags.Input.isMC:
        eventinfo_branches.variables += [
            "mcEventWeights",
            "mcChannelNumber",
            "RandomRunNumber",
            "generatorWeight_%SYS%",
            "PileupWeight_%SYS%"
        ]
        # Need syst_only_for not to be empty to avoid applying SYST on all
        # other branches
        # Any variable with %SYS% will anyway get systematics applied, so the list
        # doesn't need to be exhaustive with the extra variables added in the config
        eventinfo_branches.syst_only_for = ["generatorWeight_%SYS%",
                                            "PileupWeight_%SYS%"]

    # Replace L1Topo characters, formatting as done by the
    # trigger selection CP alg
    trigger_branches = [
        f"trigPassed_{c.replace('-', '_').replace('.', 'p')}"
        for c in trigger_chains
    ]
    eventinfo_branches.variables += trigger_branches

    # Event-level scale factors
    if flags.Input.isMC and flags.Analysis.trigger.scale_factor.doSF:

        var = ["globalTriggerEffSF_%SYS%"]
        # Only dump trigger SF if computed before
        for year in flags.Analysis.Years:
            if not flags.Analysis.TriggerChainsSF[str(year)]:
                var = []

        eventinfo_branches.variables += var

    if (
        flags.Input.isMC
        and flags.Analysis.small_R_jet.jet_type != "reco4EMTopoJet"
    ):
        btag_wps = [flags.Analysis.small_R_jet.btag_wp]
        if 'btag_extra_wps' in flags.Analysis.small_R_jet:
            btag_wps += flags.Analysis.small_R_jet.btag_extra_wps

        for wp in btag_wps:
            eventinfo_branches.variables += [f"ftag_effSF_{wp}_%SYS%"]

        # jvt is effSF is now centrally calculated by CP tools
        eventinfo_branches.variables += ["jvt_effSF_%SYS%"]

    if tree_flags.truth_outputs.higgs_particle and flags.Input.isMC:
        eventinfo_branches.variables += ["truth_H1_pdgId", "truth_H2_pdgId",
                                         "truth_children_fromH1_pdgId",
                                         "truth_children_fromH2_pdgId",
                                         "truth_initial_children_fromH1_pdgId",
                                         "truth_initial_children_fromH2_pdgId"]
        for truthpart in [
            "truth_H1", "truth_H2",
            "truth_children_fromH1", "truth_children_fromH2",
            "truth_HH",
            "truth_initial_children_fromH1", "truth_initial_children_fromH2"
        ]:
            eventinfo_branches.variables += [
                f"{truthpart}_{var}"
                for var in ["pt", "eta", "phi", "m"]
            ]

    if tree_flags.reco_outputs.large_R_Topo_jets and tree_flags.reco_outputs.VR_jets:
        eventinfo_branches.required_flags.append(
            flags.Analysis.do_large_R_Topo_jets
        )
        eventinfo_branches.variables += [
            "passRelativeDeltaRToVRJetCutTopo"
        ]

    if tree_flags.reco_outputs.large_R_UFO_jets and tree_flags.reco_outputs.VR_jets:
        eventinfo_branches.required_flags.append(
            flags.Analysis.do_large_R_UFO_jets
        )
        eventinfo_branches.variables += [
            "passRelativeDeltaRToVRJetCutUFO"
        ]

    if flags.Input.MCChannelNumber in flags.Analysis.DSID_nWLep_samples:
        eventinfo_branches.variables += ["nWLep"]

    return eventinfo_branches.get_output_list()
