from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from HH4bAnalysis.Algs.DiHiggsAnalysis import DiHiggsAnalysisAddBranches
from HH4bAnalysis.Algs.Tree import AnalysisTreeAlgCfg
from HH4bAnalysis.utils.containerNameHelper import get_container_names
from HH4bAnalysis.utils.inputsHelper import get_valid_ami_tag, is_physlite
from HH4bAnalysis.utils.logHelper import log


def MiniTupleCfg(
    flags,
    outfname,
    trigger_chains,
    working_points,
    do_muons=True,
    do_PRW=False,
    do_dihiggs_analysis=False,
    disable_calib=False,
):
    cfg = ComponentAccumulator()
    is_daod_physlite = is_physlite(flags)
    containers = get_container_names(flags, disable_calib)["outputs"]

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
        CompFactory.THistSvc(Output=[f"ANALYSIS DATAFILE='{outfname}', OPT='RECREATE'"])
    )

    def getFourMomBranches(container, alias, doOR=False):
        ORstr = "_OR" if doOR else ""

        branches = []
        vars = ["pt", "eta", "phi"]
        if "Jets" in container:
            vars.append("m")
        for var in vars:
            branches += [
                f"{container}{ORstr}.{var}  -> {alias}{ORstr}_%SYS%_{var}",
            ]
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

    if do_PRW and not disable_calib:
        analysisTreeBranches += [
            "EventInfo.PileupWeight_%SYS% -> pileupWeight_%SYS%",
            "EventInfo.generatorWeight_%SYS% -> generatorWeight_%SYS%",
        ]
    else:
        analysisTreeBranches += [
            "EventInfo.mcEventWeights -> pileupWeight_NOSYS",
        ]

    objectpairs = {
        containers["electrons"]: "el",
        containers["photons"]: "ph",
        containers["reco4Jet"]: "recojet_antikt4",
    }
    if do_muons:
        containers["muons"] = "mu"

    for cont, alias in objectpairs.items():
        analysisTreeBranches += getFourMomBranches(cont, alias)
        analysisTreeBranches += getFourMomBranches(cont, alias, doOR=True)

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

    if not disable_calib:
        # B-jet WPs
        analysisTreeBranches += [
            f"{containers['reco4Jet']}.ftag_select_{btag_wp}"
            f" -> recojet_antikt4_%SYS%_{btag_wp}"
            for btag_wp in working_points["ak4"]
        ]
        analysisTreeBranches += [
            f"{containers['reco4Jet']}_OR.ftag_select_{btag_wp}"
            f" -> recojet_antikt4_OR_%SYS%_{btag_wp}"
            for btag_wp in working_points["ak4"]
        ]
        analysisTreeBranches += getFourMomBranches(containers["vrJet"], "vrjet")
        analysisTreeBranches += [
            f"{containers['vrJet']}.ftag_select_{btag_wp}"
            f" -> vrjet_%SYS%_{btag_wp}"  # noqa
            for btag_wp in working_points["vr"]
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
    if disable_calib:
        _tmp = list(analysisTreeBranches)
        analysisTreeBranches = []
        for b in _tmp:
            source, output = b.split("->")
            analysisTreeBranches.append(
                "->".join(
                    [source.replace("_%SYS", ""), output.replace("_%SYS%", "_NOSYS")]
                )
            )  # noqa

    if do_dihiggs_analysis:
        analysisTreeBranches += DiHiggsAnalysisAddBranches(flags, working_points)

    log.info("Add tree seq")
    cfg.merge(AnalysisTreeAlgCfg(flags, branches=analysisTreeBranches))

    return cfg
