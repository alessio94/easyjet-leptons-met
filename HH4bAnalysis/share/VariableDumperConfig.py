#!/bin/env python

#
# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
#

#
# VariableDumperConfig.py
# A simple CA file to create a tree of variables
#

import sys
from pathlib import Path

from AthenaCommon import Logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AutoConfigFlags import GetFileMD
from HH4bAnalysis.Config.Base import pileupConfigFiles, cache_metadata, update_metadata
from HH4bAnalysis.Algs.Event import (
    GeneratorAnalysisSequenceCfg,
    TriggerAnalysisSequenceCfg,
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
        nargs="*",
        default=[
            "DL1dv00_FixedCutBEff_77",
            "DL1dv00_FixedCutBEff_85",
        ],
        help="Btag working points default %(default)s",
    )
    parser.add_argument(
        "--vr-btag-wps",
        type=str,
        nargs="*",
        default=[
            # "DL1r_FixedCutBEff_77",
            # "DL1r_FixedCutBEff_85",
        ],
        help="VR Jets btag working points default %(default)s",
    )
    parser.add_argument(
        "--trigger-list",
        type=str,
        default="Run3",
        help="Trigger list to use, default: %(default)s",
    )
    parser.add_argument(
        "-c",
        "--meta-cache",
        type=Path,
        default=None,
        nargs="?",
        const=Path("metadata.json"),
        help="use metadata cache file, defaults to %(const)s",
    )
    parser.add_argument(
        "-o",
        "--loose",
        action="store_true",
        help="use loose event cleaning (to get something to pass)",
    )
    return parser


def _is_physlite(flags):
    return flags.Input.ProcessingTags == ["StreamDAOD_PHYSLITE"]


def _is_mc_phys(flags):
    return flags.Input.isMC and not _is_physlite(flags)


def _get_container_names(flags):
    is_daod_physlite = _is_physlite(flags)
    inputs = dict(
        reco4Jet=getContainerName("Reco4PFlowJets", is_daod_physlite),
        reco10Jet=getContainerName("Reco10PFlowJets", is_daod_physlite),
        vrJet=getContainerName("VRJets", is_daod_physlite),
        muons=getContainerName("Muons", is_daod_physlite),
        electrons=getContainerName("Electrons", is_daod_physlite),
        photons=getContainerName("Photons", is_daod_physlite),
    )
    outputs = dict(
        reco4Jet=f"Analysis{inputs['reco4Jet']}_%SYS%",
        muons=f"Analysis{inputs['muons']}_%SYS%",
        electrons=f"Analysis{inputs['electrons']}_%SYS%",
        photons=f"Analysis{inputs['photons']}_%SYS%",
    )
    if inputs["reco10Jet"]:
        outputs["reco10Jet"] = f"Analysis{inputs['reco10Jet']}_%SYS%"
    else:
        outputs["reco10Jet"] = ""
    if inputs["vrJet"]:
        outputs["vrJet"] = f"Analysis{inputs['vrJet']}_%SYS%"
    else:
        outputs["vrJet"] = ""
    return {"inputs": inputs, "outputs": outputs}


# Generate the algorithm to do the dumping.
# AthAlgSequence does not respect filter decisions,
# so we will need to add a new sequence to the CA
def AnalysisAlgsCfg(
    flags,
    btag_wps,
    vr_btag_wps,
    trigger_chains=[],
    do_muons=True,
    metadata_cache=None,
    do_loose=False,
):
    fileMD = GetFileMD(flags.Input.Files[0])
    if metadata_cache:
        update_metadata(metadata_cache)
    dataType = "mc" if flags.Input.isMC else "data"
    is_daod_physlite = _is_physlite(flags)
    log.info(
        f"Self-configured: dataType: '{dataType}', is PHYSLITE? {is_daod_physlite}"
    )

    log.debug(f"Containers available in dataset: {flags.Input.Collections}")

    # TODO: no DL1d branches in PHYSLITE yet
    if is_daod_physlite:
        btag_wps = [wp.replace("DL1dv00", "DL1r") for wp in btag_wps]

    cfg = ComponentAccumulator()

    # Create SystematicsSvc explicitly:
    cfg.addService(CompFactory.getComp("CP::SystematicsSvc")("SystematicsSvc"))

    log.info("Adding trigger analysis algs")
    # Removes events failing trigger and adds variable to EventInfo
    # if trigger passed or not, for example:
    # EventInfo.trigger_name
    cfg.merge(TriggerAnalysisSequenceCfg(flags, dataType, trigger_chains))

    log.info("Add DQ event filter sequence")
    # Remove events failing DQ criteria
    cfg.merge(
        EventSelectionAnalysisSequenceCfg(flags, dataType, grlFiles=[], loose=do_loose)
    )

    doPRW = _is_mc_phys(flags)
    log.info(
        f"Do PRW is {doPRW}. " f"{'Add' if doPRW else 'Skip'} pileup re-weight sequence"
    )
    if doPRW:
        try:
            prwFiles, lumicalcFiles = pileupConfigFiles(fileMD)
            # Adds variable to EventInfo if for pileup weight, for example:
            # EventInfo.PileWeight_%SYS$
            cfg.merge(
                PileupAnalysisSequenceCfg(
                    flags,
                    dataType=dataType,
                    prwFiles=prwFiles,
                    lumicalcFiles=lumicalcFiles,
                )
            )

            log.info("Adding generator analysis sequence")
            # Adds variable to EventInfo if for generator weight, for example:
            # EventInfo.generatorWeight_%SYS%
            cfg.merge(GeneratorAnalysisSequenceCfg(flags, dataType))

        except LookupError as err:
            log.error(err)
            doPRW = False

    containers = _get_container_names(flags)

    log.info("Add electron seq")
    cfg.merge(
        ElectronAnalysisSequenceCfg(
            flags,
            dataType=dataType,
            inputContainerName=containers["inputs"]["electrons"],
            outputContainerName=containers["outputs"]["electrons"],
        )
    )

    log.info("Add photon seq")
    cfg.merge(
        PhotonAnalysisSequenceCfg(
            flags,
            dataType=dataType,
            inputContainerName=containers["inputs"]["photons"],
            outputContainerName=containers["outputs"]["photons"],
        )
    )

    if do_muons:
        log.info("Add muon seq")
        cfg.merge(
            MuonAnalysisSequenceCfg(
                flags,
                dataType=dataType,
                inputContainerName=containers["inputs"]["muons"],
                outputContainerName=containers["outputs"]["muons"],
            )
        )

    log.info("Add jet seq")
    cfg.merge(
        JetAnalysisSequenceCfg(
            flags,
            dataType=dataType,
            inputContainerName=containers["inputs"]["reco4Jet"],
            outputContainerName=containers["outputs"]["reco4Jet"],
            workingPoints=btag_wps,
            is_daod_physlite=is_daod_physlite,
        )
    )

    if is_daod_physlite:
        log.warning("On PHYSLITE, skip large-R jet sequence for now")
    else:
        log.info("Add large-R jet seq")
        cfg.merge(
            FatJetAnalysisSequenceCfg(
                flags,
                dataType=dataType,
                inputContainerName=containers["inputs"]["reco10Jet"],
                outputContainerName=containers["outputs"]["reco10Jet"],
            )
        )

    if is_daod_physlite:
        log.warning("On PHYSLITE, skip VR jet sequence for now")
    else:
        log.info("Add VR jet seq")
        cfg.merge(
            VRJetAnalysisSequenceCfg(
                flags,
                dataType=dataType,
                inputContainerName=containers["inputs"]["vrJet"],
                outputContainerName=containers["outputs"]["vrJet"],
                workingPoints=vr_btag_wps,
            )
        )

    ########################################################################
    # Begin postprocessing
    ########################################################################

    log.info("Add Overlap Removal sequence")
    overlapInputNames = {
        "electrons": containers["outputs"]["electrons"],
        "photons": containers["outputs"]["photons"],
        "jets": containers["outputs"]["reco4Jet"],
    }
    if do_muons:
        overlapInputNames["muons"] = containers["outputs"]["muons"]

    if not is_daod_physlite:
        overlapInputNames["fatJets"] = containers["outputs"]["reco10Jet"]

    overlapOutputNames = {k: f"{v}_OR" for k, v in overlapInputNames.items()}

    cfg.merge(
        OverlapAnalysisSequenceCfg(
            flags,
            dataType=dataType,
            inputNames=overlapInputNames,
            outputNames=overlapOutputNames,
            doFatJets=not is_daod_physlite,
            doMuons=do_muons,
        )
    )

    if metadata_cache:
        cache_metadata(metadata_cache)

    return cfg


def MiniTupleCfg(
    flags,
    outfname,
    trigger_chains,
    working_points,
    do_muons=True,
):
    cfg = ComponentAccumulator()
    is_daod_physlite = _is_physlite(flags)
    doPRW = _is_mc_phys(flags)
    containers = _get_container_names(flags)["outputs"]

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

    if doPRW:
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

    if not is_daod_physlite:
        analysisTreeBranches += getFourMomBranches(
            containers["reco10Jet"], "recojet_antikt10"
        )
        analysisTreeBranches += getFourMomBranches(
            containers["reco10Jet"], "recojet_antikt10", doOR=True
        )
        analysisTreeBranches += getFourMomBranches(containers["vrJet"], "vrjet")
        analysisTreeBranches += [
            f"{containers['vrJet']}.ftag_select_{btag_wp}" f" -> vrjet_%SYS%_{btag_wp}"
            for btag_wp in working_points["vr"]
        ]

    log.info("Add tree seq")
    cfg.merge(AnalysisTreeAlgCfg(flags, branches=analysisTreeBranches))

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

        from EventBookkeeperTools.EventBookkeeperToolsConfig import (
            CutFlowSvcCfg,
            BookkeeperToolCfg,
        )

        # Create CutFlowSvc otherwise the default CutFlowSvc that has only
        # one CutflowBookkeeper object, and can't deal with multiple weights
        cfg.merge(CutFlowSvcCfg(ConfigFlags))
        cfg.merge(BookkeeperToolCfg(ConfigFlags))

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

        trigger_chains = TriggerLists[args.trigger_list]
        do_muons = not args.meta_cache

        cfg.addSequence(CompFactory.AthSequencer("HH4bSeq"), "AthAlgSeq")
        cfg.merge(
            AnalysisAlgsCfg(
                ConfigFlags,
                btag_wps=args.btag_wps,
                vr_btag_wps=args.vr_btag_wps,
                trigger_chains=trigger_chains,
                metadata_cache=args.meta_cache,
                do_muons=do_muons,
                do_loose=args.loose,
            ),
            "HH4bSeq",
        )
        cfg.merge(
            MiniTupleCfg(
                ConfigFlags,
                outfname=args.outFile,
                trigger_chains=trigger_chains,
                working_points={"ak4": args.btag_wps, "vr": args.vr_btag_wps},
                do_muons=do_muons,
            ),
            "HH4bSeq",
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
