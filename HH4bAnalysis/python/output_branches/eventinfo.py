from HH4bAnalysis.output_branches.branch_manager import BranchManager


def get_event_info_branches(flags, do_PRW, trigger_chains):
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

    if do_PRW and not flags.Analysis.disable_calib:
        eventinfo_branches.variables += ["PileupWeight_%SYS%"]

    # Replace L1Topo characters, formatting as done by the
    # trigger selection CP alg
    trigger_branches = [
        f"trigPassed_{c.replace('-', '_').replace('.', 'p')}"
        for c in trigger_chains
    ]
    eventinfo_branches.variables += trigger_branches

    if flags.Analysis.write_truth_higgs and flags.Input.isMC:
        eventinfo_branches.variables += ["truth_H1_pdgId", "truth_H2_pdgId"]
        for truthpart in [
            "truth_H1", "truth_H2",
            "truth_bb_fromH1", "truth_bb_fromH2",
        ]:
            eventinfo_branches.variables += [
                f"{truthpart}_{var}"
                for var in ["pt", "eta", "phi", "m"]
            ]

    if flags.Analysis.write_large_R_Topo_jets and flags.Analysis.write_VR_jets:
        eventinfo_branches.required_flags.append(
            flags.Analysis.do_large_R_Topo_jets
        )
        eventinfo_branches.variables += [
            "passRelativeDeltaRToVRJetCutTopo"
        ]

    if flags.Analysis.write_large_R_UFO_jets and flags.Analysis.write_VR_jets:
        eventinfo_branches.required_flags.append(
            flags.Analysis.do_large_R_UFO_jets
        )
        eventinfo_branches.variables += [
            "passRelativeDeltaRToVRJetCutUFO"
        ]

    return eventinfo_branches.get_output_list()
