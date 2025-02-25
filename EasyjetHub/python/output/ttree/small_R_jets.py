from EasyjetHub.output.ttree.branch_manager import BranchManager, SystOption
from EasyjetHub.output.ttree.truth_jets import get_TopHiggs_jet_truth_labels
from EasyjetHub.steering.sample_metadata import get_valid_ami_tag


def get_small_R_jet_branches(
    flags, tree_flags, input_container, output_prefix
):
    _syst_option = SystOption.ALL_SYST
    if flags.Analysis.disable_calib:
        _syst_option = SystOption.NONE

    small_R_jet_branches = BranchManager(
        input_container,
        output_prefix,
        systematics_option=_syst_option,
        systematics_suffix_separator=flags.Analysis.systematics_suffix_separator,
        required_flags=[
            flags.Analysis.do_small_R_jets
        ],
    )

    if tree_flags.slim_variables_with_syst:
        small_R_jet_branches.syst_only_for = ["pt", "m", "jvt_selection"]
        if flags.Analysis.Small_R_jet.useFJvt:
            small_R_jet_branches.syst_only_for += ["fjvt_selection"]

    small_R_jet_branches.add_four_mom_branches(do_mass=True)

    # If object selector are run OR + JVT selection flags are combined into
    # isAnalysisJet
    if tree_flags.collection_options.small_R_jets.run_selection:
        small_R_jet_branches.variables += ["isAnalysisJet_%SYS%"]
        for index in range(flags.Analysis.Small_R_jet.amount_bjet):
            small_R_jet_branches.variables += [f"isbjet{index+1}_%SYS%"]
    else:
        if flags.Analysis.do_overlap_removal:
            small_R_jet_branches.variables += ["passesOR_%SYS%"]

        if flags.Analysis.Small_R_jet.jet_type != "reco4EMTopoJet":
            small_R_jet_branches.variables += ["jvt_selection"]
            if flags.Analysis.Small_R_jet.useFJvt:
                small_R_jet_branches.variables += ["fjvt_selection"]

    if flags.Analysis.Small_R_jet.jet_type != "reco4EMTopoJet":
        if flags.Input.isMC:
            # truth label used by Jet/Etmiss - always add it when running on MC
            small_R_jet_branches.variables += ["PartonTruthLabelID"]

        if tree_flags.collection_options.small_R_jets.btag_info:
            btag_wps = []
            if flags.Analysis.Small_R_jet.btag_wp != "":
                btag_wps += [flags.Analysis.Small_R_jet.btag_wp]
            if 'btag_extra_wps' in flags.Analysis.Small_R_jet:
                btag_wps += flags.Analysis.Small_R_jet.btag_extra_wps

            # Make sure PCBT is scheduled to get SF
            for tagger in ["DL1dv01", "GN2v01"]:
                add_PCBT = False
                for tagger_wp in btag_wps:
                    if tagger in tagger_wp:
                        add_PCBT = True
                        break
                if (tagger + "_Continuous") not in btag_wps and add_PCBT:
                    btag_wps += [tagger + "_Continuous"]

            small_R_jet_branches.variables += [
                f"ftag_select_{btag_wp}"
                for btag_wp in btag_wps if "Continuous" not in btag_wp
            ]
            small_R_jet_branches.variables += [
                f"ftag_quantile_{btag_wp}"
                for btag_wp in btag_wps if "Continuous" in btag_wp
            ]
            if flags.Input.isMC:
                # always add btag truth label if btag is used, when running on MC
                small_R_jet_branches.variables += ["HadronConeExclTruthLabelID"]
                for wp in btag_wps:
                    if "FixedCutBEff" in wp or "Continuous2D" in wp:
                        continue
                    small_R_jet_branches.variables += [
                        f"ftag_effSF_{wp}_%SYS%"
                    ]

        if flags.Analysis.Small_R_jet.runBJetPtCalib:
            if tree_flags.collection_options.small_R_jets.no_bjet_calib_p4:
                small_R_jet_branches.variables += ["n_muons_%SYS%"]
                if flags.Input.isMC:
                    small_R_jet_branches.variables += ["bJetTruthPt", "bJetTruthDR"]

                small_R_jet_branches.variables += [
                    f"NoBJetCalibMomentum_{var}"
                    for var in ["pt", "eta", "phi", "m"]
                ]
                small_R_jet_branches.variables += [
                    f"MuonCorrMomentum_{var}"
                    for var in ["pt", "eta", "phi", "m"]
                ]
                if tree_flags.slim_variables_with_syst:
                    small_R_jet_branches.syst_only_for += [
                        "NoBJetCalibMomentum_pt",
                        "NoBJetCalibMomentum_m",
                        "MuonCorrMomentum_pt",
                        "MuonCorrMomentum_m",
                    ]

        if tree_flags.collection_options.small_R_jets.JVT_details:
            small_R_jet_branches.variables += [
                "Jvt",
                "JvtRpt",
                "JVFCorr",
                "NNJvt",
                "NNJvtRpt",
            ]

            if flags.Input.isMC:
                small_R_jet_branches.variables += ["jvt_effSF_%SYS%"]
                if flags.Analysis.Small_R_jet.useFJvt:
                    small_R_jet_branches.variables += ["fjvt_effSF_%SYS%"]

    if (
        flags.Input.isMC
        and (tree_flags.collection_options.small_R_jets.truth_parent_info)
    ):
        small_R_jet_branches.variables += get_TopHiggs_jet_truth_labels(flags)

    if flags.Analysis.Small_R_jet.saveTriggerInfo:
        for trig in flags.Analysis.TriggerChains:
            trig = trig.replace("-", "_").replace(".", "p")
            if flags.Analysis.Small_R_jet.doL1Matching:
                small_R_jet_branches.variables += [
                    f'match{trig}_L1et',
                    f'match{trig}_L1eta',
                    f'match{trig}_L1phi',
                    f'match{trig}_L1dr',
                    f'match{trig}_L1thresholds'
                ]
            if flags.Analysis.Small_R_jet.doHLTMatching:
                small_R_jet_branches.variables += [
                    f'match{trig}_HLTpt',
                    f'match{trig}_HLTeta',
                    f'match{trig}_HLTphi',
                    f'match{trig}_HLTdr',
                    f'match{trig}_HLTthresholds',
                    f'match{trig}_HLTbtag'
                ]

    # ftag scores pb, pc, pl
    if tree_flags.collection_options.small_R_jets.btag_details:
        small_R_jet_branches.variables += [
            "DL1dv01_pb",
            "DL1dv01_pc",
            "DL1dv01_pu"
        ]

        split_tags = flags.Input.AMITag.split("_")
        gn2v00_valid_ptag = (get_valid_ami_tag(split_tags, "p", "p5855")
                             and not get_valid_ami_tag(split_tags, "p", "p6187"))
        if gn2v00_valid_ptag:
            small_R_jet_branches.variables += [
                "GN2v00_pb",
                "GN2v00_pc",
                "GN2v00_pu",
            ]

        gn2v01_valid_ptag = (
            (get_valid_ami_tag(split_tags, "p", "p6026") and not flags.Input.isPHYSLITE)
            or get_valid_ami_tag(split_tags, "p", "p6255"))
        if gn2v01_valid_ptag:
            small_R_jet_branches.variables += [
                "GN2v01_pb",
                "GN2v01_pc",
                "GN2v01_pu",
                "GN2v01_ptau",
            ]

    return small_R_jet_branches.get_output_list()
