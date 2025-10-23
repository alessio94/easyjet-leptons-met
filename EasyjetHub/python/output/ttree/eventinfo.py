from EasyjetHub.output.ttree.branch_manager import BranchManager, SystOption
from AthenaConfiguration.Enums import LHCPeriod
from EasyjetHub.steering.sample_metadata import get_valid_ami_tag
from EasyjetHub.steering.sample_metadata import STXS_info
from EasyjetHub.steering.analysis_configuration import get_trigger_chains_scale_factor


def get_event_info_branches(flags, tree_flags, trigger_chains):
    _syst_option = SystOption.ALL_SYST
    if flags.Analysis.disable_calib:
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
            "nPrimaryVertices"
        ],
        # Need syst_only_for not to be empty to avoid applying SYST on all
        # branches, except those with %SYS% explicitly in the provided variable name
        # (which we do for all SFs and trig-matching branches here)
        syst_only_for=[''],
    )

    if flags.Analysis.do_primary_vertex_pos:
        eventinfo_branches.variables += [
            "beamPosX",
            "beamPosY",
            "beamPosZ",
            "PrimaryVertexPosX",
            "PrimaryVertexPosY",
            "PrimaryVertexPosZ",
        ]
    if flags.Input.isMC:
        eventinfo_branches.variables += [
            "mcChannelNumber",
            "RandomRunNumber",
            "generatorWeight_%SYS%",
        ]

        if not flags.Input.isPHYSLITE:
            eventinfo_branches.variables += ["RandomLumiBlockNumber"]

        if flags.Input.MCChannelNumber in flags.Analysis.Truth.DSID_HSTP_samples:
            eventinfo_branches.variables += ["PassHSTP"]

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

        if flags.Analysis.Truth.do_STXS:
            eventinfo_branches.variables += [
                "HTXS_Category_Stage1_2_pTjet30",
                "HTXS_Category_Stage1_2_Fine_pTjet30",
                "HTXS_Category_Stage1_2_pTjet25",
                "HTXS_Category_Stage1_2_Fine_pTjet25",
            ]
            _, _, has_STXS_unc = STXS_info(flags.Input.MCChannelNumber)
            if has_STXS_unc:
                eventinfo_branches.variables += ["HTXS_Weights_Stage1_2_pTjet30"]

        if flags.Analysis.doPRW:
            PRW_config = [flags.Analysis.PileupReweighting]
            PRW_config += flags.Analysis.PileupReweighting.extra_prw
            altConfig = False
            for prw in PRW_config:
                postfix = ("_" + prw.postfix) if altConfig else ""
                weight = "PileupWeight" + postfix + "_%SYS%"
                eventinfo_branches.variables += [weight]
                altConfig = True

    # Replace L1Topo characters, formatting as done by the
    # trigger selection CP alg
    trigger_branches = [
        f"trigPassed_{c.replace('-', '_').replace('.', 'p')}"
        for c in trigger_chains
    ]

    if flags.Analysis.Trigger.writeOutput:
        eventinfo_branches.variables += trigger_branches

    # Event-level scale factors
    trg_vars = []
    if flags.Input.isMC and flags.Analysis.Trigger.scale_factor.doSF:
        trg_vars.append('globalTriggerEffSF_%SYS%')
    if flags.Analysis.Trigger.scale_factor.do_trigger_match:
        trg_vars.append('globalTriggerMatch_%SYS%')
    if trg_vars:
        # Only dump trigger SF and matching flag if computed before
        for year in flags.Analysis.Years:
            if not flags.Analysis.TriggerChainsSF[str(year)]:
                trg_vars = []

        eventinfo_branches.variables += trg_vars

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

        # Make sure PCBT is scheduled to get SF
        if (
            btag_wps
            and "GN2v01_Continuous2D" not in btag_wps
            and "GN2v01_Continuous" not in btag_wps
        ):
            btag_wps += ["GN2v01_Continuous"]

        for wp in btag_wps:
            if "FixedCutBEff" in wp:
                continue
            eventinfo_branches.variables += [f"ftag_effSF_{wp}_%SYS%"]

            trigSF_flags = flags.Analysis.Trigger.scale_factor
            if trigSF_flags.doSF and hasattr(trigSF_flags, 'bjet'):
                triggerChainsPerYear = get_trigger_chains_scale_factor(flags, 'bjet')
                for triggerChains in triggerChainsPerYear.values():
                    for chain in triggerChains:
                        eventinfo_branches.variables += [
                            f"ftag_effSF_{wp}_{chain}_%SYS%"
                        ]

        # jvt is effSF is now centrally calculated by CP tools
        eventinfo_branches.variables += ["jvt_effSF_%SYS%"]
        if flags.Analysis.Small_R_jet.useFJvt:
            eventinfo_branches.variables += ["fjvt_effSF_%SYS%"]

    if tree_flags.truth_outputs.higgs_particle and flags.Input.isMC:
        eventinfo_branches.variables += [
            variable
            for fmt in [
                "truth_H{:d}_{:s}",
                "truth_children_fromH{:d}_{:s}",
                "truth_initial_children_fromH{:d}_{:s}",
            ]
            for var in ["pt", "eta", "phi", "m", "pdgId"]
            for i in range(flags.Analysis.Truth.nHiggses)
            for variable in [fmt.format(i + 1, var)]
        ]

        eventinfo_branches.variables += [
            f"truth_HH_{var}"
            for var in ["pt", "eta", "phi", "m"]
        ]

        eventinfo_branches.variables += ["truth_HH_average_pt",
                                         "truth_HH_average_eta",
                                         "truth_HH_abs_cos_theta_star"]

        if flags.Analysis.Truth.recordGrandchildren:
            eventinfo_branches.variables += [
                variable
                for fmt in [
                    "truth_grandchildren_fromH{:d}_{:s}",
                    "truth_initial_grandchildren_fromH{:d}_{:s}",
                ]
                for var in ["pt", "eta", "phi", "m", "pdgId"]
                for i in range(flags.Analysis.Truth.nHiggses)
                for variable in [fmt.format(i + 1, var)]
            ]

    if flags.Analysis.GRL.store_decoration:
        from GoodRunsLists.GoodRunsListsDictionary import getGoodRunsLists
        for key in getGoodRunsLists().keys():
            for year in flags.Analysis.Years:
                if str(year) in key:
                    eventinfo_branches.variables += [key]
    # Add VgammaOR if the DSID is relevant&filtering is not applied
    if (
        flags.Input.MCChannelNumber in flags.Analysis.Truth.DSID_vgammaOR
        and flags.Analysis.bypass_VGammaOR and flags.Analysis.do_VgammaOR
    ):
        eventinfo_branches.variables += ["in_vgamma_overlap_%SYS%"]
    return eventinfo_branches.get_output_list()
