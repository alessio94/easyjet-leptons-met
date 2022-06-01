#!/bin/env python
################################################################################
# VariablePlotterConfig.py
# A simple CA file to create histograms of variables
#
# Author: Victor Ruelas

# Basic setup
from AthenaCommon import Logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

pileuplog = Logging.logging.getLogger("VariablePlotterConfig")


# Generate the algorithm to do the histogramming.
# AthAlgSequence does not respect filter decisions,
# so we will need to add a new sequence to the CA
def VariablePlotterCfg(flags, outfname):
    cfg = ComponentAccumulator()

    # Every CA should include all its dependencies, apart from the global ones
    # included in the main function.
    #
    # Add an instance of THistSvc, to create the output file and associated stream.
    # This is needed so that the alg can register its output histograms.
    # The syntax for the output is:
    #   Stream name: "ANALYSIS" (default assumed by AthHistogramAlgorithm)
    #   Output file name: specified by setting "DATAFILE"
    #   File I/O option: specified by setting "OPT" and passed to the TFile constructor
    #      "RECREATE" will (over)write the specified file name with a new file
    cfg.addService(
        CompFactory.THistSvc(
            Output=[f"ANALYSIS DATAFILE='{outfname}', OPT='RECREATE'"]
        )
    )
    # Jet container name for DAOD_PHYS: AntiKt4EMPFlowJets,
    # Jet container name for DAOD_PHYSLITE: AnalysisJets,
    jetContainerName = "AntiKt4EMPFlowJets"
    # Define and configure a tool instance
    # Properties can be set as keyword arguments to the tool constructor
    bTagSelectionTool = CompFactory.BTaggingSelectionTool(
        "bTagSelectionTool",
        FlvTagCutDefinitionsFileName="xAODBTaggingEfficiency/13TeV/2021-22-13TeV-MC16-CDI-2021-12-02_v2.root",
        TaggerName="DL1dv00",
        OperatingPoint="FixedCutBEff_77",
        JetAuthor="AntiKt4EMPFlowJets",
        MinPt=20e3,
        MaxEta=2.5,
    )

    variableplotteralg = CompFactory.HH4B.VariablePlotterAlg(
        "VariablePlotter",
        RootStreamName="ANALYSIS",
        RootDirName="Variables",
        BTaggingSelectionTool=bTagSelectionTool,
        jetContainerName=jetContainerName,
    )

    cfg.addEventAlgo(variableplotteralg)

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
            default="variable-hists.root",
            help="Output file name",
        )
        args = ConfigFlags.fillFromArgs([], parser)
        # Lock the flags so that the configuration of job subcomponents cannot
        # modify them silently/unpredictably.
        ConfigFlags.lock()

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
            cfg.addService(
                CompFactory.AthenaEventLoopMgr(EventPrintoutInterval=500)
            )

        # Add the components for reading in POOL files -- this is a specialised
        # ROOT format storing structured objects like the
        # ATLAS physics objects (jets etc)
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

        cfg.merge(PoolReadCfg(ConfigFlags))

        # Add our VariablePlotter CA, calling the function defined above.
        cfg.merge(VariablePlotterCfg(ConfigFlags, outfname=args.outFile))

        # Print the full job configuration
        cfg.printConfig()

    # Execute the job defined in the ComponentAccumulator.
    # The number of events is specified by `args.evtMax`
    cfg.run(args.evtMax)


# Execute the main function if this file was executed as a script
if __name__ == "__main__":
    main()
