from EasyjetHub.output.ttree.branch_manager import BranchManager, SystOption
from AthenaConfiguration.Enums import LHCPeriod
from EasyjetHub.steering.sample_metadata import get_valid_ami_tag


def get_event_info_branches(flags, tree_flags, trigger_chains):
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
            "mcChannelNumber",
            "RandomRunNumber",
            "generatorWeight_%SYS%",
        ]

        if flags.Analysis.doPRW:
            for prw in flags.Analysis.PileupReweighting:
                postfix = ("_" + prw.postfix) if prw.postfix else ""
                eventinfo_branches.variables += [
                    "PileupWeight" + postfix + "_%SYS%"
                ]

        if flags.GeoModel.Run is LHCPeriod.Run2:
            eventinfo_branches.variables += ["beamSpotWeight"]

        split_tags = flags.Input.AMITag.split("_")
        HF_valid_ptag = (
            get_valid_ami_tag(split_tags, "p", "p6255"))
        if (flags.Input.isAnalysisFormat
                and HF_valid_ptag
                and flags.Input.MCChannelNumber
                in flags.Analysis.Truth.DSID_HF_class_samples):
            eventinfo_branches.variables += [
                "HF_SimpleClassification",
                "HF_Classification"
            ]
        if flags.Input.MCChannelNumber in flags.Analysis.Truth.DSID_nWLep_samples:
            eventinfo_branches.variables += ["nWLep"]

        # Need syst_only_for not to be empty to avoid applying SYST on all
        # other branches
        # Any variable with %SYS% will anyway get systematics applied, so the list
        # doesn't need to be exhaustive with the extra variables added in the config
        eventinfo_branches.syst_only_for = ["generatorWeight_%SYS%"]
        if flags.Analysis.doPRW:
            for prw in flags.Analysis.PileupReweighting:
                postfix = ("_" + prw.postfix) if prw.postfix else ""
                eventinfo_branches.syst_only_for += [
                    "PileupWeight" + postfix + "_%SYS%"
                ]

    # Replace L1Topo characters, formatting as done by the
    # trigger selection CP alg
    trigger_branches = [
        f"trigPassed_{c.replace('-', '_').replace('.', 'p')}"
        for c in trigger_chains
    ]
    eventinfo_branches.variables += trigger_branches

    # Event-level scale factors
    if flags.Input.isMC and flags.Analysis.Trigger.scale_factor.doSF:

        var = ["globalTriggerEffSF_%SYS%"]
        # Only dump trigger SF if computed before
        for year in flags.Analysis.Years:
            if not flags.Analysis.TriggerChainsSF[str(year)]:
                var = []

        eventinfo_branches.variables += var

    if (
        flags.Input.isMC
        and flags.Analysis.Small_R_jet.jet_type != "reco4EMTopoJet"
        and flags.Analysis.do_small_R_jets
    ):
        btag_wps = []
        if flags.Analysis.Small_R_jet.btag_wp != "":
            btag_wps += [flags.Analysis.Small_R_jet.btag_wp]
        if 'btag_extra_wps' in flags.Analysis.Small_R_jet:
            btag_wps += flags.Analysis.Small_R_jet.btag_extra_wps

        for wp in btag_wps:
            # Until AFT-748 is solved
            if "GN2v01" in wp:
                continue
            eventinfo_branches.variables += [f"ftag_effSF_{wp}_%SYS%"]

        # jvt is effSF is now centrally calculated by CP tools
        eventinfo_branches.variables += ["jvt_effSF_%SYS%"]
        if flags.Analysis.Small_R_jet.useFJvt:
            eventinfo_branches.variables += ["fjvt_effSF_%SYS%"]

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

    if flags.Analysis.GRL.store_decoration and not flags.Input.isMC:
        from GoodRunsLists.GoodRunsListsDictionary import getGoodRunsLists
        for key in getGoodRunsLists().keys():
            for year in flags.Analysis.Years:
                if str(year) in key:
                    eventinfo_branches.variables += [key]

    return eventinfo_branches.get_output_list()
