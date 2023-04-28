from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from HH4bAnalysis.config.boosted_config import boosted_branches
from HH4bAnalysis.cpalgs.jets import (
    lr_jet_ghost_vr_jet_association_branches,
    lr_ufo_jet_ghost_vr_jet_association_branches,
)
from HH4bAnalysis.config.resolved_config import resolved_branches
from HH4bAnalysis.cpalgs.tree import tree_cfg
from HH4bAnalysis.config.sample_config import get_valid_ami_tag
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


def minituple_cfg(
    flags,
    trigger_chains,
    do_muons=True,
    do_PRW=False,
):
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

    for trig_chain in trigger_chains:
        trig_formatted = trig_chain.replace("-", "_").replace(".", "p")
        tree_branches.append(
            f"EventInfo.trigPassed_{trig_formatted} -> trigPassed_{trig_formatted}"
        )

    if do_PRW and not flags.Analysis.disable_calib:
        tree_branches += [
            "EventInfo.PileupWeight_%SYS% -> pileupWeight_%SYS%",
            "EventInfo.generatorWeight_%SYS% -> generatorWeight_%SYS%",
        ]
    else:
        tree_branches += [
            "EventInfo.mcEventWeights -> pileupWeight_NOSYS",
        ]

    # make dict with analysis container keys and ntuple alias values
    objectpairs = {
        containers["electrons"]: "el",
        containers["photons"]: "ph",
        containers["reco4Jet"]: "recojet_antikt4",
    }
    if do_muons:
        objectpairs[containers["muons"]] = "mu"

    for cont, alias in objectpairs.items():
        tree_branches += _get_four_mom_branches(cont, alias)
        tree_branches += _get_four_mom_branches(cont, alias, do_or=True)

    if flags.Input.isMC:
        parent_bosons = ["Higgs", "Scalar", "Top"]
        parent_labels = [
            "DRTruthParticle",
            "PdgId",
            "Barcode",
            "MatchingParticlePdgId"
        ]
        truth_labels = [
            "HadronConeExclTruthLabelID",
        ]
        if not is_physlite(flags):
            truth_labels += [
                f"parent{b}{l}" for l in parent_labels for b in parent_bosons
            ]
        for label in truth_labels:
            tree_branches += [
                (
                    f"{containers['reco4Jet']}.{label} ->"
                    f" recojet_antikt4_%SYS%_{label}"
                ),
                (
                    f"{containers['reco4Jet']}_OR.{label} ->"
                    f" recojet_antikt4_OR_%SYS%_{label}"
                ),
            ]
        tree_branches += _get_four_mom_branches(
            containers["truth4Jet"], "truthjet_antikt4", do_systematics=False
        )
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

        for var in reco10JetVars:
            tree_branches += [
                f"{containers['reco10Jet']}.{var} -> recojet_antikt10_%SYS%_{var}"
            ]
        # one after the other for better readability in the root file
        for var in reco10JetVars:
            tree_branches += [
                (
                    f"{containers['reco10Jet']}_OR.{var} -> recojet_antikt10_OR_%SYS%_{var}"  # noqa
                ),
            ]
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

        for v in reco10UFOJetVars:
            tree_branches += [
                (
                    f"{containers['reco10UFOJet']}.{v} -> recoUFOjet_antikt10_%SYS%_{v}"  # noqa
                ),
            ]

        tree_branches += [
            b.replace("%SYS%", "NOSYS")
            for b in lr_jet_ghost_vr_jet_association_branches(
                flags, containers["reco10Jet"]
            )
        ]

        tree_branches += lr_ufo_jet_ghost_vr_jet_association_branches(
            flags, containers["reco10UFOJet"]
        )

        tree_branches += _get_four_mom_branches(
            containers["reco10Jet"], "recojet_antikt10"
        )
        tree_branches += _get_four_mom_branches(
            containers["reco10Jet"], "recojet_antikt10", do_or=True
        )
        tree_branches += _get_four_mom_branches(
            containers["reco10UFOJet"], "recoUFOjet_antikt10"
        )

        # Restore SYS when LargeRJet alg supports systematics
        if flags.Input.isMC:
            tree_branches += [
                (
                    f"{containers['reco10Jet'].replace('%SYS%','NOSYS')}"
                    ".R10TruthLabel_R21Consolidated ->"
                    " R10TruthLabel_R21Consolidated_NOSYS"
                ),
                (
                    f"{containers['reco10Jet'].replace('%SYS%','NOSYS')}_OR"
                    ".R10TruthLabel_R21Consolidated ->"
                    " R10TruthLabel_R21Consolidated_OR_NOSYS"
                ),
                (
                    f"{containers['reco10UFOJet'].replace('%SYS%','NOSYS')}"
                    ".R10TruthLabel_R21Precision_2022v1 ->"
                    " UFO_R10TruthLabel_R21Precision_2022v1_NOSYS"
                ),
            ]
            # Just added this ptag check for now. Because older p-tag
            # derivations do not have these truth label for large-R jet.
            if "p5511" in flags.Input.AMITag:
                tree_branches += [
                    (
                        f"{containers['reco10UFOJet'].replace('%SYS%','NOSYS')}"
                        ".R10TruthLabel_R22v1 ->"
                        " UFO_R10TruthLabel_R22v1_NOSYS"
                    ),
                ]

    if not flags.Analysis.disable_calib:
        if flags.Input.isMC:
            tree_branches += _get_four_mom_branches(
                containers["truth10Jet"], "truthjet_antikt10", do_systematics=False
            )
            tree_branches += _get_four_mom_branches(
                containers["truth10UFOJet"],
                "truthUFOjet_antikt10",
                do_systematics=False,
            )

        if flags.Input.isMC:
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

        # B-jet WPs
        tree_branches += [
            f"{containers['reco4Jet']}.ftag_select_{btag_wp}"
            f" -> recojet_antikt4_%SYS%_{btag_wp}"
            for btag_wp in flags.Analysis.btag_wps
        ]
        tree_branches += [
            f"{containers['reco4Jet']}_OR.ftag_select_{btag_wp}"
            f" -> recojet_antikt4_OR_%SYS%_{btag_wp}"
            for btag_wp in flags.Analysis.btag_wps
        ]

        if do_muons:
            # B-jet momentum without correction
            tree_branches += [
                f"{containers['reco4Jet']}.NoBJetCalibMomentum_{var}"
                f" -> recojet_antikt4_%SYS%_nobjetcalib_{var}"
                for var in ["pt", "eta", "phi", "m"]
            ]
            tree_branches += [
                f"{containers['reco4Jet']}_OR.NoBJetCalibMomentum_{var}"
                f" -> recojet_antikt4_OR_%SYS%_nobjetcalib_{var}"
                for var in ["pt", "eta", "phi", "m"]
            ]

    split_tags = flags.Input.AMITag.split("_")
    is_valid_ptag = get_valid_ami_tag(split_tags, "p")

    # JVT
    jvt_branches = [
        "Jvt",
        "JvtRpt",
        "JVFCorr",
        "jvt_selection",
        "NNJvt",
        "NNJvtRpt",
        "NNJvtPass",
    ]

    if is_daod_physlite:
        jvt_branches = ["NNJvtPass"]

    elif not is_valid_ptag:
        # Skip the NNjvt variables for old mc20 samples
        jvt_branches = jvt_branches[:-4]

    tree_branches += [
        f"{containers['reco4Jet']}.{var} -> recojet_antikt4_%SYS%_{var}"
        for var in jvt_branches
    ] + [
        f"{containers['reco4Jet']}_OR.{var} -> recojet_antikt4_OR_%SYS%_{var}"
        for var in jvt_branches
    ]
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
