#!/bin/env python
################################################################################
# VariableDumperConfig.py
# A simple CA file to create a tree of variables
#
# Author: Victor Ruelas

# Basic setup
from AthenaCommon import Logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from utils.argsHelper import checkArgs

variabledumperlog = Logging.logging.getLogger("VariableDumperConfig")


# Generate the algorithm to do the histogramming.
# AthAlgSequence does not respect filter decisions,
# so we will need to add a new sequence to the CA
def VariableDumperCfg(flags, daodphyslite, outfname):
    cfg = ComponentAccumulator()

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

    reco4JetContainerName = "AnalysisJetsBTAG" if daodphyslite else "AntiKt4EMPFlowJets"
    reco10JetContainerName = (
        "AnalysisLargeRRecoJets"
        if daodphyslite
        else "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets"
    )
    truth4JetContainerName = "?" if daodphyslite else "AntiKt4TruthDressedWZJets"
    truth10JetContainerName = (
        "?" if daodphyslite else "AntiKt10TruthTrimmedPtFrac5SmallR20Jets"
    )
    muonsContainerName = "AnalysisMuons" if daodphyslite else "Muons"

    # Define and configure a tool instance
    # Properties can be set as keyword arguments to the tool constructor
    bTagSelectionTool = CompFactory.BTaggingSelectionTool(
        "bTagSelectionTool",
        FlvTagCutDefinitionsFileName=(
            "xAODBTaggingEfficiency/13TeV/2021-22-13TeV-MC16-CDI-2021-12-02_v2.root"
        ),
        TaggerName="DL1dv00",
        OperatingPoint="FixedCutBEff_77",
        JetAuthor=reco4JetContainerName,
        MinPt=20e3,
        MaxEta=2.5,
    )

    cfg.addEventAlgo(
        CompFactory.HH4B.VariableDumperAlg(
            "VariableDumper",
            EventInfoKey="EventInfo",
            Reco4JetsKey=reco4JetContainerName,
            Reco10JetsKey=reco10JetContainerName,
            MuonsKey=muonsContainerName,
            # Needs to be implemented
            # ElectronsKey="AnalysisElectrons" if daodphyslite else "Electrons",
            RootStreamName="ANALYSIS",
            # RootDirName="Reco",
            BTaggingSelectionTool=bTagSelectionTool,
        )
    )

    return cfg


# CA modules are intended to be executable, to facilitate easy testing.
# We define a "main function" that will run a test job if the module
# is executed rather than imported.
def main():
    # Setting temporarily needed for Run 3 code, to generate python
    # Configurable objects for deduplication
    from AthenaCommon.Configurable import ConfigurableRun3Behavior

    with ConfigurableRun3Behavior():

        # Import the job configuration flags, some of which will be autoconfigured.
        # These are used for steering the job, and include e.g. the input file (list).
        from AthenaConfiguration.AllConfigFlags import ConfigFlags

        # Generate a parser and add an output file argument, then retrieve the args
        parser = ConfigFlags.getArgumentParser()
        parser.add_argument(
            "--outFile",
            type=str,
            default="analysis-variables.root",
            help="Output file name",
        )
        parser.add_argument(
            "--mc",
            action="store_true",
            help="Input is Monte Carlo",
        )
        parser.add_argument(
            "--daod-physlite",
            action="store_true",
            help="Input is DAOD_PHYSLITE",
        )
        args = ConfigFlags.fillFromArgs([], parser)
        # Lock the flags so that the configuration of job subcomponents cannot
        # modify them silently/unpredictably.
        ConfigFlags.lock()

        checkArgs(ConfigFlags, args, parser)

        # Get a ComponentAccumulator setting up the standard components
        # needed to run an Athena job.
        from AthenaConfiguration.MainServicesConfig import MainServicesCfg

        cfg = MainServicesCfg(ConfigFlags)
        # Adjust the loop manager to announce the event number less frequently.
        # Makes a big difference if running over many events
        if ConfigFlags.Concurrency.NumThreads > 0:
            cfg.addService(
                CompFactory.AthenaHiveEventLoopMgr(EventPrintoutInterval=500)
            )
        else:
            cfg.addService(CompFactory.AthenaEventLoopMgr(EventPrintoutInterval=500))

        # Add the components for reading in POOL files -- this is a specialised
        # ROOT format storing structured objects like the
        # ATLAS physics objects (jets etc)
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

        cfg.merge(PoolReadCfg(ConfigFlags))

        # Add our VariableDumper CA, calling the function defined above.
        cfg.merge(
            VariableDumperCfg(
                ConfigFlags, daodphyslite=args.daod_physlite, outfname=args.outFile
            )
        )

        # Print the full job configuration
        cfg.printConfig()

    # Execute the job defined in the ComponentAccumulator.
    # The number of events is specified by `args.evtMax`
    cfg.run(args.evtMax)


# Execute the main function if this file was executed as a script
if __name__ == "__main__":
    main()
