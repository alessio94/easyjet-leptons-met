from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from HH4bAnalysis.Algs.DiHiggsAnalysis import DiHiggsAnalysisAddBranches
from HH4bAnalysis.Algs.Tree import AnalysisTreeAlgCfg
from HH4bAnalysis.utils.containerNameHelper import get_container_names
from HH4bAnalysis.utils.inputsHelper import get_valid_ami_tag, is_physlite
from HH4bAnalysis.utils.logHelper import log


def MiniTupleCfg(
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
    ]

    for trig_chain in trigger_chains:
        cleaned = trig_chain.replace("-", "_")
        if "." in trig_chain:
            continue
        analysisTreeBranches.append(
            f"EventInfo.trigPassed_{cleaned} -> trigPassed_{cleaned}"
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
        reco10JetVars = [
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
        for var in reco10JetVars:
            analysisTreeBranches += [
                f"{containers['reco10Jet']}.{var} -> recojet_antikt10_%SYS%_{var}"
            ]
        # one after the other for better readability in the root file
        for var in reco10JetVars:
            analysisTreeBranches += [
                f"{containers['reco10Jet']}_OR.{var} -> recojet_antikt10_OR_%SYS%_{var}"
            ]

        analysisTreeBranches += getFourMomBranches(
            containers["reco10Jet"], "recojet_antikt10"
        )
        analysisTreeBranches += getFourMomBranches(
            containers["reco10Jet"], "recojet_antikt10", doOR=True
        )

    if not flags.Analysis.disable_calib:
        if flags.Input.isMC:
            analysisTreeBranches += getFourMomBranches(
                containers["truth10Jet"], "truthjet_antikt10", noSystematics=True
            )

        if flags.Input.isMC:
            analysisTreeBranches += getTruthFourMomBranches("EventInfo", "truth_H")
            analysisTreeBranches += getTruthFourMomBranches("EventInfo", "truth_S")
            analysisTreeBranches += getTruthFourMomBranches(
                "EventInfo", "truth_b_fromH"
            )
            analysisTreeBranches += getTruthFourMomBranches(
                "EventInfo", "truth_b_fromS"
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
        analysisTreeBranches += getFourMomBranches(containers["vrJet"], "vrjet")
        if flags.Input.isMC:
            analysisTreeBranches += [
                (
                    f"{containers['vrJet']}.HadronConeExclTruthLabelID ->"
                    " vrjet_%SYS%_HadronConeExclTruthLabelID"
                ),
            ]
        analysisTreeBranches += [
            f"{containers['vrJet']}.ftag_select_{btag_wp} -> vrjet_%SYS%_{btag_wp}"
            for btag_wp in flags.Analysis.vr_btag_wps
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

    if not is_valid_ptag:
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

    if flags.Analysis.do_dihiggs_analysis and not flags.Analysis.disable_calib:
        analysisTreeBranches += DiHiggsAnalysisAddBranches(flags)

    log.info("Add tree seq")
    cfg.merge(AnalysisTreeAlgCfg(flags, branches=analysisTreeBranches))

    return cfg
