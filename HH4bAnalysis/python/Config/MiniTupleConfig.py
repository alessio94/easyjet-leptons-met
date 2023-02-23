from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from HH4bAnalysis.Algs.BoostedAnalysis import BoostedTreeBranches
from HH4bAnalysis.Algs.Jets import (
    LargeJetGhostVRJetAssociationBranches,
    LargeUFOJetGhostVRJetAssociationBranches,
)
from HH4bAnalysis.Algs.ResolvedAnalysis import ResolvedTreeBranches
from HH4bAnalysis.Algs.Tree import AnalysisTreeAlgCfg
from HH4bAnalysis.Config.Base import get_valid_ami_tag
from HH4bAnalysis.utils.containerNameHelper import get_container_names
from HH4bAnalysis.utils.inputsHelper import is_physlite
from HH4bAnalysis.utils.logHelper import log


def MiniTupleCfg(
    flags,
    trigger_chains,
    doCBK=True,
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
            Output=[f"ANALYSIS DATAFILE='{flags.Analysis.outFile}', OPT='RECREATE'"]
        )
    )

    def getFourMomBranches(
        container, alias, doOR=False, noSystematics=False, doMass=False
    ):
        ORstr = "_OR" if doOR else ""
        SYSstr = "" if noSystematics else "_%SYS%"
        branches = []
        vars = ["pt", "eta", "phi"]
        if "Jets" in container or doMass:
            vars.append("m")
        for var in vars:
            branches += [
                f"{container}{ORstr}.{var}  -> {alias}{ORstr}{SYSstr}_{var}",
            ]
        return branches

    def getTruthFourMomBranches(container, alias):
        branches = []
        vars = ["pt", "eta", "phi", "m"]
        for var in vars:
            branches += [f"{container}.{alias}_{var}  ->  {alias}_{var}"]
        return branches

    analysisTreeBranches = [
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
        analysisTreeBranches.append(
            f"EventInfo.trigPassed_{trig_formatted} -> trigPassed_{trig_formatted}"
        )

    if do_PRW and not flags.Analysis.disable_calib:
        analysisTreeBranches += [
            "EventInfo.PileupWeight_%SYS% -> pileupWeight_%SYS%",
            "EventInfo.generatorWeight_%SYS% -> generatorWeight_%SYS%",
        ]
    else:
        analysisTreeBranches += [
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
        analysisTreeBranches += getFourMomBranches(cont, alias)
        analysisTreeBranches += getFourMomBranches(cont, alias, doOR=True)

    if flags.Input.isMC:
        analysisTreeBranches += [
            (
                f"{containers['reco4Jet']}.HadronConeExclTruthLabelID ->"
                " recojet_antikt4_%SYS%_HadronConeExclTruthLabelID"
            ),
            (
                f"{containers['reco4Jet']}_OR.HadronConeExclTruthLabelID ->"
                " recojet_antikt4_OR_%SYS%_HadronConeExclTruthLabelID"
            ),
        ]

    if flags.Input.isMC:
        analysisTreeBranches += getFourMomBranches(
            containers["truth4Jet"], "truthjet_antikt4", noSystematics=True
        )
        analysisTreeBranches += [
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
            analysisTreeBranches += [
                f"{containers['reco10Jet']}.{var} -> recojet_antikt10_%SYS%_{var}"
            ]
        # one after the other for better readability in the root file
        for var in reco10JetVars:
            analysisTreeBranches += [
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
            analysisTreeBranches += [
                (
                    f"{containers['reco10UFOJet']}.{v} -> recoUFOjet_antikt10_%SYS%_{v}"  # noqa
                ),
            ]

        analysisTreeBranches += [
            b.replace('%SYS%','NOSYS') for b in
            LargeJetGhostVRJetAssociationBranches
            (
                flags, containers["reco10Jet"]
            )
        ]

        analysisTreeBranches += LargeUFOJetGhostVRJetAssociationBranches(
            flags, containers["reco10UFOJet"]
        )

        analysisTreeBranches += getFourMomBranches(
            containers["reco10Jet"], "recojet_antikt10"
        )
        analysisTreeBranches += getFourMomBranches(
            containers["reco10Jet"], "recojet_antikt10", doOR=True
        )
        analysisTreeBranches += getFourMomBranches(
            containers["reco10UFOJet"], "recoUFOjet_antikt10"
        )

        # Restore SYS when LargeRJet alg supports systematics
        if flags.Input.isMC:
            analysisTreeBranches += [
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
                analysisTreeBranches += [
                    (
                        f"{containers['reco10UFOJet'].replace('%SYS%','NOSYS')}"
                        ".R10TruthLabel_R22v1 ->"
                        " UFO_R10TruthLabel_R22v1_NOSYS"
                    ),
                ]

    if not flags.Analysis.disable_calib:
        if flags.Input.isMC:
            analysisTreeBranches += getFourMomBranches(
                containers["truth10Jet"], "truthjet_antikt10", noSystematics=True
            )
            analysisTreeBranches += getFourMomBranches(
                containers["truth10UFOJet"], "truthUFOjet_antikt10", noSystematics=True
            )

        if flags.Input.isMC:
            analysisTreeBranches += ["EventInfo.truth_H1_pdgId -> truth_H1_pdgId"]
            analysisTreeBranches += getTruthFourMomBranches("EventInfo", "truth_H1")
            analysisTreeBranches += getTruthFourMomBranches(
                "EventInfo", "truth_bb_fromH1"
            )
            analysisTreeBranches += ["EventInfo.truth_H2_pdgId -> truth_H2_pdgId"]
            analysisTreeBranches += getTruthFourMomBranches("EventInfo", "truth_H2")
            analysisTreeBranches += getTruthFourMomBranches(
                "EventInfo", "truth_bb_fromH2"
            )

        # B-jet WPs
        analysisTreeBranches += [
            f"{containers['reco4Jet']}.ftag_select_{btag_wp}"
            f" -> recojet_antikt4_%SYS%_{btag_wp}"
            for btag_wp in flags.Analysis.btag_wps
        ]
        analysisTreeBranches += [
            f"{containers['reco4Jet']}_OR.ftag_select_{btag_wp}"
            f" -> recojet_antikt4_OR_%SYS%_{btag_wp}"
            for btag_wp in flags.Analysis.btag_wps
        ]

        if do_muons:
            # B-jet momentum without correction
            analysisTreeBranches += [
                f"{containers['reco4Jet']}.NoBJetCalibMomentum_{var}"
                f" -> recojet_antikt4_%SYS%_nobjetcalib_{var}"
                for var in ["pt", "eta", "phi", "m"]
            ]
            analysisTreeBranches += [
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

    analysisTreeBranches += [
        f"{containers['reco4Jet']}.{var} -> recojet_antikt4_%SYS%_{var}"
        for var in jvt_branches
    ] + [
        f"{containers['reco4Jet']}_OR.{var} -> recojet_antikt4_OR_%SYS%_{var}"
        for var in jvt_branches
    ]
    # No calibration algs -- remove all systematics expressions in the input
    # and label as NOSYS in output
    if flags.Analysis.disable_calib:
        _tmp = list(analysisTreeBranches)
        analysisTreeBranches = []
        for b in _tmp:
            source, output = b.split("->")
            analysisTreeBranches.append(
                "->".join(
                    [source.replace("_%SYS", ""), output.replace("_%SYS%", "_NOSYS")]
                )
            )  # noqa

    if flags.Analysis.do_resolved_dihiggs_analysis and not flags.Analysis.disable_calib:
        analysisTreeBranches += ResolvedTreeBranches(flags)

    if flags.Analysis.do_boosted_dihiggs_analysis and not flags.Analysis.disable_calib:
        analysisTreeBranches += BoostedTreeBranches(flags)

    log.info("Add tree seq")
    cfg.merge(AnalysisTreeAlgCfg(flags, branches=analysisTreeBranches))

    return cfg
