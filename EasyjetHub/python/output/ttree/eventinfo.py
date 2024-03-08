from EasyjetHub.output.ttree.branch_manager import BranchManager


def get_event_info_branches(flags, tree_flags, do_PRW, trigger_chains):
    eventinfo_branches = BranchManager(
        input_container="EventInfo",
        output_prefix="",
        variables=[
            "runNumber",
            "eventNumber",
            "lumiBlock",
            "mcEventWeights",
            "averageInteractionsPerCrossing",
            "actualInteractionsPerCrossing",
            "mcChannelNumber",
        ]
    )
    if flags.Input.isMC:
        eventinfo_branches.variables += ["generatorWeight_%SYS%"]
        eventinfo_branches.variables += ["PileupWeight_%SYS%"]

    # Replace L1Topo characters, formatting as done by the
    # trigger selection CP alg
    trigger_branches = [
        f"trigPassed_{c.replace('-', '_').replace('.', 'p')}"
        for c in trigger_chains
    ]
    eventinfo_branches.variables += trigger_branches

    # Event-level scale factors
    if flags.Input.isMC:
        btag_wps = [flags.Analysis.small_R_jet.btag_wp]
        if 'btag_extra_wps' in flags.Analysis.small_R_jet:
            btag_wps += flags.Analysis.small_R_jet.btag_extra_wps

        for wp in btag_wps:
            # No GN2v01 SF in CDI for now
            if "GN2v01" in wp:
                continue
            eventinfo_branches.variables += [f"ftag_effSF_{wp}_%SYS%"]

        # jvt is effSF is now centrally calculated by CP tools
        eventinfo_branches.variables += ["jvt_effSF_%SYS%"]

    if tree_flags.truth_outputs.higgs_particle and flags.Input.isMC:
        eventinfo_branches.variables += ["truth_H1_pdgId", "truth_H2_pdgId",
                                         "truth_children_fromH1_pdgId",
                                         "truth_children_fromH2_pdgId"]
        for truthpart in [
            "truth_H1", "truth_H2",
            "truth_children_fromH1", "truth_children_fromH2",
            "truth_HH"
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

    return eventinfo_branches.get_output_list()
