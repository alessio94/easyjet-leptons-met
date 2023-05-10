from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from HH4bAnalysis.config.boosted_config import boosted_branches
from HH4bAnalysis.cpalgs.jets import (
    lr_jet_ghost_vr_jet_association_branches,
    lr_ufo_jet_ghost_vr_jet_association_branches,
)
from HH4bAnalysis.config.resolved_config import resolved_branches
from HH4bAnalysis.cpalgs.tree import tree_cfg
from HH4bAnalysis.config.sample_config import (
    get_valid_ami_tag,
    SampleTypes,
)
from HH4bAnalysis.config.container_names import get_container_names
from HH4bAnalysis.utils.inputs_helper import is_physlite
from HH4bAnalysis.utils.log_helper import log


def _get_four_mom_branches(
    container, alias, do_or=False, do_systematics=True, do_mass=False
):
    or_postfix = "_OR" if do_or else ""
    sys_postfix = "_%SYS%" if do_systematics else ""
    branches = []
    vars = ["pt", "eta", "phi"]
    if "Jets" in container or do_mass:
        vars.append("m")
    for var in vars:
        branches += [
            f"{container}{or_postfix}.{var}  -> {alias}{or_postfix}{sys_postfix}_{var}",
        ]
    return branches


def _get_truth_four_mom_branches(container, alias):
    branches = []
    vars = ["pt", "eta", "phi", "m"]
    for var in vars:
        branches += [f"{container}.{alias}_{var}  ->  {alias}_{var}"]
    return branches


def minituple_cfg(flags, trigger_chains, do_PRW=False, do_OR=False):
    cfg = ComponentAccumulator()
    is_daod_physlite = is_physlite(flags)

    containers = get_container_names(flags)["outputs"]
    log.debug(f"Containers requested in dataset: {containers}")

    ########################################################################
    # Create analysis mini-ntuple
    ########################################################################

    # Add an instance of THistSvc, to create the output file and associated stream.
    # This is needed so that the alg can register its output TTree.
    # The syntax for the output is:
    #   Stream name: "ANALYSIS" (default assumed by AthHistogramAlgorithm)
    #   Output file name: specified by setting "DATAFILE"
    #   File I/O option: specified by setting "OPT" and passed to the TFile constructor
    #      "RECREATE" will (over)write the specified file name with a new file
    cfg.addService(
        CompFactory.THistSvc(
            Output=[f"ANALYSIS DATAFILE='{flags.Analysis.out_file}', OPT='RECREATE'"]
        )
    )

    tree_branches = [
        "EventInfo.runNumber     -> runNumber",
        "EventInfo.eventNumber   -> eventNumber",
        "EventInfo.lumiBlock   -> lumiBlock",
        "EventInfo.mcEventWeights   -> mcEventWeights",
        "EventInfo.averageInteractionsPerCrossing -> averageInteractionsPerCrossing",
        "EventInfo.actualInteractionsPerCrossing -> actualInteractionsPerCrossing",
        "EventInfo.mcChannelNumber -> mcChannelNumber",
    ]

    if do_PRW and not flags.Analysis.disable_calib:
        tree_branches += [
            "EventInfo.PileupWeight_%SYS% -> pileupWeight_%SYS%",
        ]

    for trig_chain in trigger_chains:
        trig_formatted = trig_chain.replace("-", "_").replace(".", "p")
        tree_branches.append(
            f"EventInfo.trigPassed_{trig_formatted} -> trigPassed_{trig_formatted}"
        )

    if flags.Analysis.write_electrons:
        tree_branches += _get_four_mom_branches(
            container=containers["electrons"], alias="el", do_or=do_OR
        )

    if flags.Analysis.write_photons:
        tree_branches += _get_four_mom_branches(
            container=containers["photons"], alias="ph", do_or=do_OR
        )
    if flags.Analysis.write_muons and flags.Analysis.do_muons:
        tree_branches += _get_four_mom_branches(
            container=containers["muons"], alias="mu", do_or=do_OR
        )

    if flags.Analysis.write_small_R_jets:
        tree_branches = add_small_R_branches(
            flags, is_daod_physlite, containers, tree_branches, do_or=do_OR
        )

    if flags.Analysis.write_large_R_Topo_jets:
        tree_branches = add_large_R_Topo_branches(
            flags, is_daod_physlite, containers, tree_branches, do_or=do_OR
        )

    if flags.Analysis.write_large_R_UFO_jets:
        tree_branches = add_large_R_UFO_branches(
            flags, is_daod_physlite, containers, tree_branches, do_or=do_OR
        )

    if flags.Input.isMC and flags.Analysis.write_truth_small_R_jets:
        tree_branches += _get_four_mom_branches(
            containers["truth4Jet"], "truthjet_antikt4", do_systematics=False
        )

    if flags.Input.isMC and not flags.Analysis.disable_calib:
        if flags.Analysis.write_truth_large_R_jets:
            if flags.Analysis.write_large_R_Topo_jets:
                tree_branches += _get_four_mom_branches(
                    containers["truth10Jet"],
                    "truthjet_antikt10",
                    do_systematics=False,
                )
            if flags.Analysis.write_large_R_UFO_jets:
                tree_branches += _get_four_mom_branches(
                    containers["truth10UFOJet"],
                    "truthUFOjet_antikt10",
                    do_systematics=False,
                )
        if flags.Analysis.write_truth_higgs:
            tree_branches += ["EventInfo.truth_H1_pdgId -> truth_H1_pdgId"]
            tree_branches += _get_truth_four_mom_branches("EventInfo", "truth_H1")
            tree_branches += _get_truth_four_mom_branches(
                "EventInfo", "truth_bb_fromH1"
            )
            tree_branches += ["EventInfo.truth_H2_pdgId -> truth_H2_pdgId"]
            tree_branches += _get_truth_four_mom_branches("EventInfo", "truth_H2")
            tree_branches += _get_truth_four_mom_branches(
                "EventInfo", "truth_bb_fromH2"
            )

    # No calibration algs -- remove all systematics expressions in the input
    # and label as NOSYS in output
    if flags.Analysis.disable_calib:
        _tmp = list(tree_branches)
        tree_branches = []
        for b in _tmp:
            source, output = b.split("->")
            tree_branches.append(
                "->".join(
                    [source.replace("_%SYS", ""), output.replace("_%SYS%", "_NOSYS")]
                )
            )  # noqa

    if flags.Analysis.do_resolved_dihiggs_analysis and not flags.Analysis.disable_calib:
        tree_branches += resolved_branches(flags)

    if flags.Analysis.do_boosted_dihiggs_analysis and not flags.Analysis.disable_calib:
        tree_branches += boosted_branches(flags)

    log.info("Add tree seq")
    cfg.merge(tree_cfg(flags, branches=tree_branches))

    return cfg


def get_jvt_detailed_branches(flags, is_daod_physlite):
    # JVT
    jvt_branches = [
        "Jvt",
        "JvtRpt",
        "JVFCorr",
        "jvt_selection",
        "NNJvt",
        "NNJvtRpt",
    ]

    return jvt_branches


def get_jet_truth_labels(flags):
    # first part is adding the truth matching to parent particles
    parent_bosons = ["Higgs", "Scalar", "Top"]
    parent_labels = [
        "DRTruthParticle",
        "PdgId",
        "MatchingParticlePdgId",
    ]
    small_r_labels = [
        "HadronConeExclTruthLabelID",
    ]

    if is_physlite(flags):
        truth_labels = []
    else:
        truth_labels = [f"parent{b}{l}" for l in parent_labels for b in parent_bosons]

    truths = []
    if flags.Analysis.write_small_R_jets:
        truths.append((4, "", small_r_labels))
    if flags.Analysis.write_large_R_UFO_jets:
        truths.append((10, "UFO", []))

    containers = get_container_names(flags)["outputs"]

    tree_branches = []
    for width, constit, additional in truths:
        for label in truth_labels + additional:
            jet_container = containers[f"reco{width}{constit}Jet"]
            tree_branches.append(
                f"{jet_container}.{label} ->"
                f" reco{constit}jet_antikt{width}_%SYS%_{label}"
            )
            if flags.Analysis.do_overlap_removal:
                tree_branches.append(
                    f"{jet_container}_OR.{label} ->"
                    f" reco{constit}jet_antikt{width}_OR_%SYS%_{label}"
                )

    # Also add the standard b-tagging truth labels
    tree_branches += [
        (
            f"{containers['truth4Jet']}.PartonTruthLabelID ->"
            " truthjet_antikt4_PartonTruthLabelID"
        ),
        (
            f"{containers['truth4Jet']}.HadronConeExclTruthLabelID ->"
            " truthjet_antikt4_HadronConeExclTruthLabelID"
        ),
    ]
    return tree_branches


def get_small_R_gn2_branches():
    # GN2 scores (not included in CDI)
    small_R_gn2_branches = [
        "GN2v00_pc",
        "GN2v00_pu",
        "GN2v00_pb",
    ]
    return small_R_gn2_branches


def add_small_R_branches(flags, is_daod_physlite, containers, tree_branches, do_or):
    tree_branches += _get_four_mom_branches(
        container=containers["reco4Jet"], alias="recojet_antikt4", do_or=do_or
    )
    if flags.Input.isMC and flags.Analysis.write_small_R_higgs_parent_info:
        tree_branches += get_jet_truth_labels(flags)

    jvt_branches = get_jvt_detailed_branches(flags, is_daod_physlite)

    if (
        flags.Analysis.do_small_R_jets
        and flags.Analysis.do_muons
        and not flags.Analysis.disable_calib
    ):
        if do_or:
            # B-jet WPs
            if flags.Analysis.write_small_R_btag:
                tree_branches += [
                    f"{containers['reco4Jet']}_OR.ftag_select_{btag_wp}"
                    f" -> recojet_antikt4_OR_%SYS%_{btag_wp}"
                    for btag_wp in flags.Analysis.btag_wps
                ]
            # B-jet momentum without correction
            if flags.Analysis.write_small_R_no_bjet_calib:
                tree_branches += [
                    f"{containers['reco4Jet']}_OR.NoBJetCalibMomentum_{var}"
                    f" -> recojet_antikt4_OR_%SYS%_nobjetcalib_{var}"
                    for var in ["pt", "eta", "phi", "m"]
                ]

            # JVT
            tree_branches += [
                f"{containers['reco4Jet']}_OR.NNJvtPass ->"
                " recojet_antikt4_OR_%SYS%_NNJvtPass"
            ]
            if flags.Analysis.write_small_R_JVT_details:
                tree_branches += [
                    f"{containers['reco4Jet']}_OR.{var} ->"
                    f" recojet_antikt4_OR_%SYS%_{var}"
                    for var in jvt_branches
                ]

        else:
            if flags.Analysis.write_small_R_btag:
                tree_branches += [
                    f"{containers['reco4Jet']}.ftag_select_{btag_wp}"
                    f" -> recojet_antikt4_%SYS%_{btag_wp}"
                    for btag_wp in flags.Analysis.btag_wps
                ]

            if (
                not flags.Analysis.fast_test
                and flags.Analysis.write_small_R_no_bjet_calib
            ):
                tree_branches += [
                    f"{containers['reco4Jet']}.NoBJetCalibMomentum_{var}"
                    f" -> recojet_antikt4_%SYS%_nobjetcalib_{var}"
                    for var in ["pt", "eta", "phi", "m"]
                ]
            tree_branches += [
                f"{containers['reco4Jet']}.NNJvtPass -> recojet_antikt4_%SYS%_NNJvtPass"
            ]
            if flags.Analysis.write_small_R_JVT_details:
                tree_branches += [
                    f"{containers['reco4Jet']}.{var} -> recojet_antikt4_%SYS%_{var}"
                    for var in jvt_branches
                ]

    # GN2 scores
    # not available in PHYSLITE... yet
    # NB these are read from BTagging collection
    # so cannot be compared to thinned list of jets!
    if flags.Analysis.write_small_R_gn2_branches and not is_daod_physlite:
        small_R_gn2_branches = get_small_R_gn2_branches()
        tree_branches += [
            f"BTagging_AntiKt4EMPFlow.{var} -> recojet_antikt4_NOSYS_{var}"
            for var in small_R_gn2_branches
        ]

    return tree_branches


def add_large_R_Topo_branches(
    flags, is_daod_physlite, containers, tree_branches, do_or
):
    if not is_daod_physlite:
        reco10JetVars = (
            [
                "NTrimSubjets",
                "TrackSumPt",
                "Tau1_wta",
                "Tau2_wta",
                "Tau3_wta",
                "ECF1",
                "ECF2",
                "ECF3",
                "Split12",
                "Split23",
                "JetConstitScaleMomentum_pt",
                "JetConstitScaleMomentum_eta",
                "JetConstitScaleMomentum_phi",
                "JetConstitScaleMomentum_m",
            ]
            + ["GhostBHadronsFinalCount"]
            if flags.Input.isMC
            else []
        )

        if do_or:
            if flags.Analysis.write_large_R_substructure:
                for var in reco10JetVars:
                    # one after the other for better readability in the root file
                    tree_branches += [
                        f"{containers['reco10Jet']}_OR.{var} ->"
                        f" recojet_antikt10_OR_%SYS%_{var}"
                    ]
            tree_branches += _get_four_mom_branches(
                containers["reco10Jet"], "recojet_antikt10", do_or=True
            )
            # Restore SYS when LargeRJet alg supports systematics
            if flags.Input.isMC:
                tree_branches += [
                    (
                        f"{containers['reco10Jet'].replace('%SYS%','NOSYS')}_OR"
                        ".R10TruthLabel_R21Consolidated ->"
                        " recojet_antikt10_OR_NOSYS_R10TruthLabel_R21Consolidated"
                    ),
                ]
            tree_branches += [
                b.replace("%SYS%", "NOSYS")
                for b in lr_jet_ghost_vr_jet_association_branches(
                    flags, f"{containers['reco10Jet']}_OR", do_OR=True
                )
            ]
        else:
            if flags.Analysis.write_large_R_substructure:
                for var in reco10JetVars:
                    tree_branches += [
                        f"{containers['reco10Jet']}.{var} ->"
                        f" recojet_antikt10_%SYS%_{var}"
                    ]
            tree_branches += _get_four_mom_branches(
                containers["reco10Jet"], "recojet_antikt10", do_or=False
            )
            if flags.Input.isMC:
                tree_branches += [
                    (
                        f"{containers['reco10Jet'].replace('%SYS%','NOSYS')}"
                        ".R10TruthLabel_R21Consolidated ->"
                        " recojet_antikt10_NOSYS_R10TruthLabel_R21Consolidated"
                    ),
                ]

            tree_branches += [
                b.replace("%SYS%", "NOSYS")
                for b in lr_jet_ghost_vr_jet_association_branches(
                    flags, containers["reco10Jet"], do_OR=False
                )
            ]

    return tree_branches


def get_ufo_large_R_gn2_branches():
    # GN2 scores (not included in CDI)
    ufo_large_R_gn2_branches = [
        "GN2Xv00_phbb",
        "GN2Xv00_phcc",
        "GN2Xv00_ptop",
        "GN2Xv00_pqcd",
        "GN2XWithMassv00_phbb",
        "GN2XWithMassv00_phcc",
        "GN2XWithMassv00_ptop",
        "GN2XWithMassv00_pqcd",
    ]
    return ufo_large_R_gn2_branches


def add_large_R_UFO_branches(flags, is_daod_physlite, containers, tree_branches, do_or):
    if not is_daod_physlite:
        reco10UFOJetVars = (
            [
                "Tau1_wta",
                "Tau2_wta",
                "Tau3_wta",
                "ECF1",
                "ECF2",
                "ECF3",
                "Split12",
                "Split23",
                "JetConstitScaleMomentum_pt",
                "JetConstitScaleMomentum_eta",
                "JetConstitScaleMomentum_phi",
                "JetConstitScaleMomentum_m",
                "GhostBHadronsFinalCount",
            ]
            + [
                # TODO: I have no idea what this boi is doing in the
                # data-only section, but it's not in the data we're
                # testing for now...
                "Tau4_wta"
            ]
            if flags.Input.isMC
            else []
        )

        # GN2, save scores
        # only available after p5658, and not in PHYSLITE
        ufo_large_R_gn2_branches = get_ufo_large_R_gn2_branches()
        split_tags = flags.Input.AMITag.split("_")
        is_valid_ptag = get_valid_ami_tag(split_tags, "p", SampleTypes.mc20x)
        if do_or:
            if flags.Analysis.write_large_R_substructure:
                for v in reco10UFOJetVars:
                    tree_branches += [
                        (
                            f"{containers['reco10UFOJet']}_OR.{v} ->"
                            f" recoUFOjet_antikt10_OR_%SYS%_{v}"
                        ),
                    ]
            tree_branches += _get_four_mom_branches(
                containers["reco10UFOJet"],
                "recoUFOjet_antikt10",
                do_or=True,
            )
            # Restore SYS when LargeRJet alg supports systematics
            if flags.Input.isMC:
                tree_branches += [
                    (
                        f"{containers['reco10UFOJet'].replace('%SYS%','NOSYS')}_OR"
                        ".R10TruthLabel_R21Precision_2022v1 ->"
                        " recoUFOjet_antikt10_OR_NOSYS_R10TruthLabel_R21Precision_2022v1"  # noqa
                    ),
                    (
                        f"{containers['reco10UFOJet'].replace('%SYS%','NOSYS')}_OR"
                        ".R10TruthLabel_R22v1 ->"
                        " recoUFOjet_antikt10_OR_NOSYS_R10TruthLabel_R22v1"
                    ),
                ]

            tree_branches += lr_ufo_jet_ghost_vr_jet_association_branches(
                flags, f"{containers['reco10UFOJet']}_OR", do_OR=True
            )
            if is_valid_ptag:
                tree_branches += [
                    f"{containers['reco10UFOJet']}.{var} -> recoUFOjet_antikt10_OR_%SYS%_{var}"  # noqa
                    for var in ufo_large_R_gn2_branches
                ]
        else:
            if flags.Analysis.write_large_R_substructure:
                for v in reco10UFOJetVars:
                    tree_branches += [
                        (
                            f"{containers['reco10UFOJet']}.{v} ->"
                            f" recoUFOjet_antikt10_%SYS%_{v}"
                        ),
                    ]
            tree_branches += _get_four_mom_branches(
                containers["reco10UFOJet"],
                "recoUFOjet_antikt10",
                do_or=False,
            )
            # Restore SYS when LargeRJet alg supports systematics
            if flags.Input.isMC:
                tree_branches += [
                    (
                        f"{containers['reco10UFOJet'].replace('%SYS%','NOSYS')}"
                        ".R10TruthLabel_R21Precision_2022v1 ->"
                        " recoUFOjet_antikt10_NOSYS_R10TruthLabel_R21Precision_2022v1"
                    ),
                    (
                        f"{containers['reco10UFOJet'].replace('%SYS%','NOSYS')}"
                        ".R10TruthLabel_R22v1 ->"
                        " recoUFOjet_antikt10_NOSYS_R10TruthLabel_R22v1"
                    ),
                ]
            tree_branches += lr_ufo_jet_ghost_vr_jet_association_branches(
                flags, containers["reco10UFOJet"], do_OR=False
            )
            if is_valid_ptag:
                tree_branches += [
                    f"{containers['reco10UFOJet']}.{var} -> recoUFOjet_antikt10_%SYS%_{var}"  # noqa
                    for var in ufo_large_R_gn2_branches
                ]

    return tree_branches
