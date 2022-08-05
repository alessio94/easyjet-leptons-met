#!/bin/env python

#
# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
#

#
# VariableDumperConfig.py
# A simple CA file to create a tree of variables
#

import sys

from AthenaCommon import Logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AutoConfigFlags import GetFileMD
from HH4bAnalysis.Config.Base import pileupConfigFiles
from HH4bAnalysis.Algs.Event import (
    TriggerAnalysisAlgsCfg,
    PileupAnalysisSequenceCfg,
    EventSelectionAnalysisSequenceCfg,
)
from HH4bAnalysis.Algs.Electrons import ElectronAnalysisSequenceCfg
from HH4bAnalysis.Algs.Photons import PhotonAnalysisSequenceCfg
from HH4bAnalysis.Algs.Muons import MuonAnalysisSequenceCfg
from HH4bAnalysis.Algs.Jets import (
    JetAnalysisSequenceCfg,
    FatJetAnalysisSequenceCfg,
    VRJetAnalysisSequenceCfg,
)
from HH4bAnalysis.Algs.Postprocessing import OverlapAnalysisSequenceCfg
from HH4bAnalysis.Algs.Tree import AnalysisTreeAlgCfg
from HH4bAnalysis.utils.containerNameHelper import getContainerName

log = Logging.logging.getLogger("VariableDumperConfig")


def defineArgs(ConfigFlags):
    # Generate a parser and add an output file argument, then retrieve the args
    parser = ConfigFlags.getArgumentParser()
    parser.add_argument(
        "--outFile",
        type=str,
        default="analysis-variables.root",
        help="Output file name",
    )
    parser.add_argument(
        "--btag-wps",
        type=str,
        nargs="+",
        default=[
            "DL1dv00_FixedCutBEff_77",
            "DL1dv00_FixedCutBEff_85",
        ],
        help="Btag working points default %(default)s",
    )
    parser.add_argument(
        "--vr-btag-wps",
        type=str,
        nargs="+",
        default=[
            "DL1r_FixedCutBEff_77",
            "DL1r_FixedCutBEff_85",
        ],
        help="VR Jets btag working points default %(default)s",
    )
    parser.add_argument(
        "--trigger-list",
        type=str,
        default="Run3",
        help="Trigger list to use, default: %(default)",
    )
    return parser


# Generate the algorithm to do the dumping.
# AthAlgSequence does not respect filter decisions,
# so we will need to add a new sequence to the CA
def VariableDumperCfg(flags, outfname, btag_wps, vr_btag_wps, trigger_chains=[]):
    fileMD = GetFileMD(flags.Input.Files[0])
    dataType = "mc" if flags.Input.isMC else "data"
    is_daod_physlite = fileMD.get("processingTags", []) == ["StreamDAOD_PHYSLITE"]
    log.info(
        f"Self-configured: dataType: '{dataType}', is PHYSLITE? {is_daod_physlite}"
    )

    log.debug(f"Containers available in dataset: {flags.Input.Collections}")

    # TODO: no DL1d branches in PHYSLITE yet
    if is_daod_physlite:
        btag_wps = [wp.replace("DL1dv00", "DL1r") for wp in btag_wps]

    reco4JetInputContainerName = getContainerName("Reco4PFlowJets", is_daod_physlite)
    reco10JetInputContainerName = getContainerName("Reco10PFlowJets", is_daod_physlite)
    vrJetInputContainerName = getContainerName("VRJets", is_daod_physlite)
    muonsInputContainerName = getContainerName("Muons", is_daod_physlite)
    electronsInputContainerName = getContainerName("Electrons", is_daod_physlite)
    photonsInputContainerName = getContainerName("Photons", is_daod_physlite)

    reco4JetOutputContainerName = f"Analysis{reco4JetInputContainerName}_%SYS%"
    muonsOutputContainerName = f"Analysis{muonsInputContainerName}_%SYS%"
    electronsOutputContainerName = f"Analysis{electronsInputContainerName}_%SYS%"
    photonsOutputContainerName = f"Analysis{photonsInputContainerName}_%SYS%"
    if reco10JetInputContainerName:
        reco10JetOutputContainerName = f"Analysis{reco10JetInputContainerName}_%SYS%"
    else:
        reco10JetOutputContainerName = ""
    if vrJetInputContainerName:
        vrJetOutputContainerName = f"Analysis{vrJetInputContainerName}_%SYS%"
    else:
        vrJetOutputContainerName = ""

    cfg = ComponentAccumulator()
    cfg.addSequence(CompFactory.AthSequencer("HH4bSeq"))

    # Every CA should include all its dependencies, apart from the global ones
    # included in the main function.
    #
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
    # Create SystematicsSvc explicitly:
    cfg.addService(CompFactory.getComp("CP::SystematicsSvc")("SystematicsSvc"))

    log.info("Adding trigger analysis algs")
    # Adds variable to EventInfo if trigger passed or not, for example:
    # EventInfo.[trigger name] -> 1 # or 0
    if trigger_chains:
        cfg.merge(TriggerAnalysisAlgsCfg(flags, trigger_chains), "HH4bSeq")

    log.info("Add DQ Event Filter Alg")
    # Remove events failing DQ criteria
    cfg.merge(EventSelectionAnalysisSequenceCfg(flags, dataType), "HH4bSeq")

    doPRW = flags.Input.isMC and not is_daod_physlite
    if doPRW:
        try:
            # Include, and then set up the pileup analysis sequence:
            prwFiles, lumicalcFiles = pileupConfigFiles(fileMD)

            # Adds variable to EventInfo if for pileup weight, for example:
            # EventInfo.PileWeight_%SYS$ -> ?
            cfg.merge(
                PileupAnalysisSequenceCfg(
                    flags,
                    dataType=dataType,
                    prwFiles=prwFiles,
                    lumicalcFiles=lumicalcFiles,
                ),
                "HH4bSeq",
            )

        except LookupError as err:
            log.error(err)
            doPRW = False

    log.info(f"Do PRW is {doPRW}")

    log.info("Add electron seq")
    cfg.merge(
        ElectronAnalysisSequenceCfg(
            flags,
            dataType=dataType,
            inputContainerName=electronsInputContainerName,
            outputContainerName=electronsOutputContainerName,
        ),
        "HH4bSeq",
    )

    log.info("Add photon seq")
    cfg.merge(
        PhotonAnalysisSequenceCfg(
            flags,
            dataType=dataType,
            inputContainerName=photonsInputContainerName,
            outputContainerName=photonsOutputContainerName,
        ),
        "HH4bSeq",
    )

    log.info("Add muon seq")
    cfg.merge(
        MuonAnalysisSequenceCfg(
            flags,
            dataType=dataType,
            inputContainerName=muonsInputContainerName,
            outputContainerName=muonsOutputContainerName,
        ),
        "HH4bSeq",
    )

    log.info("Add jet seq")
    cfg.merge(
        JetAnalysisSequenceCfg(
            flags,
            dataType=dataType,
            inputContainerName=reco4JetInputContainerName,
            outputContainerName=reco4JetOutputContainerName,
            workingPoints=btag_wps,
            is_daod_physlite=is_daod_physlite,
        ),
        "HH4bSeq",
    )

    if is_daod_physlite:
        log.warning("On PHYSLITE, skip large-R jet sequence for now")
    else:
        log.info("Add large-R jet seq")
        cfg.merge(
            FatJetAnalysisSequenceCfg(
                flags,
                dataType=dataType,
                inputContainerName=reco10JetInputContainerName,
                outputContainerName=reco10JetOutputContainerName,
            ),
            "HH4bSeq",
        )

    if is_daod_physlite:
        log.warning("On PHYSLITE, skip VR jet sequence for now")
    else:
        log.info("Add VR jet seq")
        cfg.merge(
            VRJetAnalysisSequenceCfg(
                flags,
                dataType=dataType,
                inputContainerName=vrJetInputContainerName,
                outputContainerName=vrJetOutputContainerName,
                workingPoints=vr_btag_wps,
            ),
            "HH4bSeq",
        )

    ########################################################################
    # Begin postprocessing
    ########################################################################

    log.info("Add Overlap Removal sequence")
    overlapInputNames = {
        "electrons": electronsOutputContainerName,
        "photons": photonsOutputContainerName,
        "muons": muonsOutputContainerName,
        "jets": reco4JetOutputContainerName,
    }
    overlapOutputNames = {
        "electrons": f"{electronsOutputContainerName}_OR",
        "photons": f"{photonsOutputContainerName}_OR",
        "muons": f"{muonsOutputContainerName}_OR",
        "jets": f"{reco4JetOutputContainerName}_OR",
    }
    if not is_daod_physlite:
        overlapInputNames["fatJets"] = reco10JetOutputContainerName
        overlapOutputNames["fatJets"] = f"{reco10JetOutputContainerName}_OR"

    cfg.merge(
        OverlapAnalysisSequenceCfg(
            flags,
            dataType=dataType,
            inputNames=overlapInputNames,
            outputNames=overlapOutputNames,
            doFatJets=not is_daod_physlite,
        ),
        "HH4bSeq",
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

    ########################################################################
    # Create analysis mini-ntuple
    ########################################################################

    analysisTreeBranches = [
        "EventInfo.runNumber     -> runNumber",
        "EventInfo.eventNumber   -> eventNumber",
        "EventInfo.mcEventWeights   -> mcEventWeights",
        "EventInfo.averageInteractionsPerCrossing -> averageInteractionsPerCrossing",
    ]

    analysisTreeBranches += [
        f"EventInfo.trigPassed_{trig_chain.replace('-','_')} -> trigPassed_{trig_chain.replace('-','_')}"  # noqa
        for trig_chain in trigger_chains
    ]

    if doPRW:
        analysisTreeBranches += [
            "EventInfo.PileupWeight_%SYS% -> pileupWeight_%SYS%",
        ]
    else:
        analysisTreeBranches += [
            "EventInfo.mcEventWeights -> pileupWeight_NOSYS",
        ]

    objectpairs = {
        electronsOutputContainerName: "el",
        photonsOutputContainerName: "ph",
        muonsOutputContainerName: "mu",
        reco4JetOutputContainerName: "recojet_antikt4",
    }
    for cont, alias in objectpairs.items():
        analysisTreeBranches += getFourMomBranches(cont, alias)
        analysisTreeBranches += getFourMomBranches(cont, alias, doOR=True)

    # B-jet WPs
    analysisTreeBranches += [
        f"{reco4JetOutputContainerName}.ftag_select_{btag_wp}"
        f" -> recojet_antikt4_%SYS%_{btag_wp}"
        for btag_wp in btag_wps
    ]
    analysisTreeBranches += [
        f"{reco4JetOutputContainerName}_OR.ftag_select_{btag_wp}"
        f" -> recojet_antikt4_OR_%SYS%_{btag_wp}"
        for btag_wp in btag_wps
    ]

    if not is_daod_physlite:
        analysisTreeBranches += getFourMomBranches(
            reco10JetOutputContainerName, "recojet_antikt10"
        )
        analysisTreeBranches += getFourMomBranches(
            reco10JetOutputContainerName, "recojet_antikt10", doOR=True
        )
        analysisTreeBranches += getFourMomBranches(vrJetOutputContainerName, "vrjet")
        analysisTreeBranches += [
            f"{vrJetOutputContainerName}.ftag_select_{btag_wp}"
            f" -> vrjet_%SYS%_{btag_wp}"
            for btag_wp in vr_btag_wps
        ]

    log.info("Add tree seq")
    cfg.merge(AnalysisTreeAlgCfg(flags, branches=analysisTreeBranches), "HH4bSeq")

    return cfg


# CA modules are intended to be executable, to facilitate easy testing.
# We define a "main function" that will run a test job if the module
# is executed rather than imported.
def main():
    # Import the job configuration flags, some of which will be autoconfigured.
    # These are used for steering the job, and include e.g. the input file (list).
    from AthenaConfiguration.AllConfigFlags import ConfigFlags

    # Get the arguments, defined at the top for easy browsing
    parser = defineArgs(ConfigFlags)
    args = ConfigFlags.fillFromArgs([], parser)
    # Lock the flags so that the configuration of job subcomponents cannot
    # modify them silently/unpredictably.
    # Workaround for buggy glob, needed prior
    # to https://gitlab.cern.ch/atlas/athena/-/merge_requests/55561
    if ConfigFlags.Input.Files[0] == "_ATHENA_GENERIC_INPUTFILE_NAME_":
        ConfigFlags.Input.Files = ConfigFlags.Input.Files[1:]
    log.info(f"Operating on input files {ConfigFlags.Input.Files}")
    ConfigFlags.lock()

    # Get a ComponentAccumulator setting up the standard components
    # needed to run an Athena job.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    # Setting temporarily needed for Run 3 code, to generate python
    # Configurable objects for deduplication
    from AthenaCommon.Configurable import ConfigurableRun3Behavior

    with ConfigurableRun3Behavior():

        cfg = MainServicesCfg(ConfigFlags)
        # Adjust the loop manager to announce the event number less frequently.
        # Makes a big difference if running over many events
        if ConfigFlags.Concurrency.NumThreads > 0:
            cfg.addService(
                CompFactory.AthenaHiveEventLoopMgr(EventPrintoutInterval=500)
            )
        else:
            cfg.addService(CompFactory.AthenaEventLoopMgr(EventPrintoutInterval=500))

        from HH4bAnalysis.Config.xAODEventSelectorConfig import xAODReadCfg

        cfg.merge(xAODReadCfg(ConfigFlags))

        # Add our VariableDumper CA, calling the function defined above.
        from HH4bAnalysis.Config.TriggerLists import TriggerLists

        cfg.merge(
            VariableDumperCfg(
                ConfigFlags,
                outfname=args.outFile,
                btag_wps=args.btag_wps,
                vr_btag_wps=args.vr_btag_wps,
                trigger_chains=TriggerLists[args.trigger_list],
            )
        )

        # Print the full job configuration
        cfg.printConfig(summariseProps=False)

    # Execute the job defined in the ComponentAccumulator.
    # The number of events is specified by `args.evtMax`
    return cfg.run(args.evtMax)


# Execute the main function if this file was executed as a script
if __name__ == "__main__":
    code = main()
    sys.exit(0 if code.isSuccess() else 1)
