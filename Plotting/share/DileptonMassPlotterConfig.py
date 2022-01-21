#!/bin/env python
################################################################################
# PileupPlotterConfig.py
# A simple starter CA file to illustrate histogramming in Athena
#
# Author: TJ Khoo

# Basic setup, similar to HelloWorldConfig
from AthenaCommon import Logging
pileuplog = Logging.logging.getLogger('PileupPlotterConfig')
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Generate the algorithm to do the histogramming.
# AthAlgSequence does not respect filter decisions,
# so we will need to add a new sequence to the CA
def DileptonMassPlotterCfg(flags,outfname):
    cfg = ComponentAccumulator()
    # The default sequence created by providing `sequence`
    # to the CA constructor also ignores filters
    # Need to make sure the algs act in sequence, else the
    # filter result may be skipped
    cfg.addSequence(CompFactory.AthSequencer("MuMuSequence",Sequential=True))

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
    cfg.addService(CompFactory.THistSvc(Output = [f"ANALYSIS DATAFILE='{outfname}', OPT='RECREATE'"]))

    # Add the algs to the MuMuSequence to allow filtering
    cfg.addEventAlgo(CompFactory.MSA.DileptonFinderAlg(
        "DimuonFinder",
        LeptonsInKey="Muons",
        LeptonsOutKey="MuonPair"
        ),
        "MuMuSequence"
    )
    cfg.addEventAlgo(CompFactory.MSA.MllPlotterAlg(
        "MmumuPlotter",
        LeptonPairKey="MuonPair",
        RootStreamName = "ANALYSIS",
        RootDirName = "Zmumu"
        ),
        "MuMuSequence"
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
            '--outFile', type=str,
            default="mll-hists.root",
            help='Output file name')
        args = ConfigFlags.fillFromArgs([],parser)
        # Lock the flags so that the configuration of job subcomponents cannot
        # modify them silently/unpredictably.
        ConfigFlags.lock()

        # Get a ComponentAccumulator setting up the standard components
        # needed to run an Athena job.
        from AthenaConfiguration.MainServicesConfig import MainServicesCfg
        cfg=MainServicesCfg(ConfigFlags)
        # Adjust the loop manager to announce the event number less frequently.
        # Makes a big difference if running over many events
        if ConfigFlags.Concurrency.NumThreads>0:
            cfg.addService(CompFactory.AthenaHiveEventLoopMgr(EventPrintoutInterval=500))
        else:
            cfg.addService(CompFactory.AthenaEventLoopMgr(EventPrintoutInterval=500))

        # Add the components for reading in POOL files -- this is a specialised ROOT format
        # storing structured objects like the ATLAS physics objects (jets etc)
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        cfg.merge(PoolReadCfg(ConfigFlags))
        
        # Add our HelloWorld CA, calling the function defined above.
        # Merging this into the top-level sequence inserts the
        # HelloWorldSeq into the specified algorithm sequence.
        cfg.merge(DileptonMassPlotterCfg(ConfigFlags,outfname=args.outFile),'AthAlgSeq')

        # Print the full job configuration
        cfg.printConfig()

    # Execute the job defined in the ComponentAccumulator.
    # The number of events is specified by `args.evtMax`
    cfg.run(args.evtMax)

# Execute the main function if this file was executed as a script
if __name__=="__main__":
    main()
