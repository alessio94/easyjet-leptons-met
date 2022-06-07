# Basic setup, similar to HelloWorldConfig
from AthenaCommon import Logging

catestlog = Logging.logging.getLogger("CPAlgCATestConfig")
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Convert old style configurable to new via CompFactory
# Services and public tools will not be handled and would need to be
# added directly to the top-level CA
def convertComp(comp):
    newcomp = CompFactory.getComp(comp.getType())(comp.getName())
    for p, v in comp.getProperties().items():
        if v != "<no value>":
            # Has a componentType, indicating handle
            # Could use this to flag and store any
            # services or public tools
            if hasattr(v, "componentType"):
                setattr(newcomp, p, v.toStringProperty())
            # Has a name, but not a componentType (indicating component)
            elif hasattr(v, "getName"):
                setattr(newcomp, p, convertComp(v))
            else:
                setattr(newcomp, p, v)
    return newcomp


# Assume flat, recursion to get all sequences is
# probably tedious but might be needed
def convertSequenceAndGetAlgs(seq):
    newseq = CompFactory.AthSequencer(seq.name())
    newalgs = []
    for alg in seq:
        newalgs.append(convertComp(alg))
    return newseq, newalgs


# Could make the service creation + conversion into a separate function, as it is
# probably needed for all CP alg sequences
def CPPileupConfig(ConfigFlags, outfname):
    cfg = ComponentAccumulator()

    # Create SystematicsSvc explicitly:
    cfg.addService(CompFactory.getComp("CP::SystematicsSvc")("SystematicsSvc"))
    cfg.addService(
        CompFactory.THistSvc(Output=[f"ANALYSIS DATAFILE='{outfname}', OPT='RECREATE'"])
    )

    # Create a pile-up analysis sequence
    from AsgAnalysisAlgorithms.PileupAnalysisSequence import makePileupAnalysisSequence

    # dataType = "mc" if ConfigFlags.Input.isMC else "data"
    # pileupSequence = makePileupAnalysisSequence(dataType)
    # pileupSequence.configure(inputName={}, outputName={})
    # print(pileupSequence)  # For debugging
    # Convert to new configurables
    # pileupSequenceCnv, algsCnv = convertSequenceAndGetAlgs(pileupSequence)
    # cfg.addSequence(pileupSequenceCnv)
    # for alg in algsCnv:
    #     cfg.addEventAlgo(alg, pileupSequenceCnv.getName())

    treeMaker = CompFactory.getComp("CP::TreeMakerAlg")("TreeMaker")
    treeMaker.TreeName = "muons"
    cfg.addEventAlgo(treeMaker)
    ntupleMaker = CompFactory.getComp("CP::AsgxAODNTupleMakerAlg")(
        "NTupleMakerEventInfo"
    )
    ntupleMaker.TreeName = "muons"
    ntupleMaker.Branches = [
        "EventInfo.runNumber     -> runNumber",
        "EventInfo.eventNumber   -> eventNumber",
    ]
    # ntupleMaker.systematicsRegex = "(^$)"
    cfg.addEventAlgo(ntupleMaker)
    ntupleMaker = CompFactory.getComp("CP::AsgxAODNTupleMakerAlg")("NTupleMakerMuons")
    ntupleMaker.TreeName = "muons"
    ntupleMaker.Branches = [
        "Muons.eta -> mu_eta",
        "Muons.phi -> mu_phi",
        "Muons.pt  -> mu_pt",
    ]
    # ntupleMaker.systematicsRegex = '(^MUON_.*)'
    cfg.addEventAlgo(ntupleMaker)
    treeFiller = CompFactory.getComp("CP::TreeFillerAlg")("TreeFiller")
    treeFiller.TreeName = "muons"
    cfg.addEventAlgo(treeFiller)
    return cfg


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
        args = ConfigFlags.fillFromArgs([], parser=parser)
        # Lock the flags so that the configuration of job subcomponents cannot
        # modify them silently/unpredictably.
        ConfigFlags.lock()

        # Get a ComponentAccumulator setting up the standard components
        # needed to run an Athena job.
        from AthenaConfiguration.MainServicesConfig import MainServicesCfg

        cfg = MainServicesCfg(ConfigFlags)

        # Add the components for reading in POOL files -- this is a specialised ROOT format
        # storing structured objects like the ATLAS physics objects (jets etc)
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

        cfg.merge(PoolReadCfg(ConfigFlags))

        # Add our Pileup CP Alg CA, calling the function defined above.
        # Merging this into the top-level sequence inserts the
        # pileup algs into the specified algorithm sequence.
        cfg.merge(CPPileupConfig(ConfigFlags, outfname=args.outFile))

        # Print the full job configuration
        cfg.printConfig()

    # Execute the job defined in the ComponentAccumulator.
    # The number of events is specified by `args.evtMax`
    cfg.run(args.evtMax)


# Execute the main function if this file was executed as a script
if __name__ == "__main__":
    main()
