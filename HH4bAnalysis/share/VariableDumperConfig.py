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
from utils.containerNameHelper import getContainerName
from utils.convertOldConfigHelper import convertSequenceAndGetAlgs

variabledumperlog = Logging.logging.getLogger("VariableDumperConfig")


def pileupConfigFiles(dataType):
    """Return the PRW config files and lumicalc files"""
    if dataType == "data":
        prwfiles = []
        lumicalcfiles = []
    else:
        lumicalcfiles = [
            # These need to be updated for release 22 data?
            "GoodRunsLists/data15_13TeV/20170619/PHYS_StandardGRL_All_Good_25ns_276262-284484_OflLumi-13TeV-008.root",
            "GoodRunsLists/data16_13TeV/20180129/PHYS_StandardGRL_All_Good_25ns_297730-311481_OflLumi-13TeV-009.root",
            "GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.lumicalc.OflLumi-13TeV-010.root",
            "GoodRunsLists/data18_13TeV/20190318/ilumicalc_histograms_None_348885-364292_OflLumi-13TeV-010.root",
        ]
        if dataType == "mc":
            prwfiles = [
                # These need to be updated for the specific sample that is given to the job
                # be taken from cvmfs
                # "/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/PileupReweighting/share/DSID364xxx/pileup_mc20d_dsid364701_FS.root"
            ]
        else:
            # We don't have a PRW file that works properly for the AFII file so we don't apply it in
            # this case
            prwfiles = []
    return prwfiles, lumicalcfiles


# Generate the algorithm to do the histogramming.
# AthAlgSequence does not respect filter decisions,
# so we will need to add a new sequence to the CA
def VariableDumperCfg(flags, daodphyslite, outfname):
    dataType = "mc" if flags.Input.isMC else "data"

    reco4JetContainerName = getContainerName("Reco4PFlowJets", daodphyslite)
    reco10JetContainerName = getContainerName("Reco10PFlowJets", daodphyslite)
    # truth4JetContainerName = getContainerName("Truth4Jets", daodphyslite)
    # truth10JetContainerName = getContainerName("Truth10Jets", daodphyslite)
    muonsContainerName = getContainerName("Muons", daodphyslite)
    electronsContainerName = getContainerName("Electrons", daodphyslite)
    photonsContainerName = getContainerName("Photons", daodphyslite)

    cfg = ComponentAccumulator()

    # # Skip events with no primary vertex:
    # vertexSelectionAlg = CompFactory.getComp("CP::VertexSelectionAlg")(
    #     "PrimaryVertexSelectorAlg"
    # )
    # vertexSelectionAlg.VertexContainer = "PrimaryVertices"
    # vertexSelectionAlg.MinVertices = 1
    # cfg.addEventAlgo(vertexSelectionAlg)

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

    # Include, and then set up the pileup analysis sequence:
    prwfiles, lumicalcfiles = pileupConfigFiles(dataType)

    # Create a pile-up analysis sequence
    from AsgAnalysisAlgorithms.PileupAnalysisSequence import makePileupAnalysisSequence

    pileupSequence = makePileupAnalysisSequence(
        dataType,
        userPileupConfigs=prwfiles,
        userLumicalcFiles=lumicalcfiles,
    )
    pileupSequence.configure(inputName={}, outputName={})
    # print(pileupSequence)  # For debugging
    # Convert to new configurables
    pileupSequenceCnv, algsCnv = convertSequenceAndGetAlgs(CompFactory, pileupSequence)
    cfg.addSequence(pileupSequenceCnv)
    for alg in algsCnv:
        cfg.addEventAlgo(alg, pileupSequenceCnv.getName())

    # Include, and then set up the electron analysis sequence:
    from EgammaAnalysisAlgorithms.ElectronAnalysisSequence import (
        makeElectronAnalysisSequence,
    )

    electronSequence = makeElectronAnalysisSequence(
        dataType, workingPoint="LooseLHElectron.NonIso", postfix="loose"
    )
    electronSequence.configure(
        inputName=electronsContainerName, outputName="AnalysisElectrons_%SYS%"
    )
    # print(electronSequence)  # For debugging
    # Convert to new configurables
    electronSequenceCnv, electronAlgsCnv = convertSequenceAndGetAlgs(
        CompFactory, electronSequence
    )
    cfg.addSequence(electronSequenceCnv)
    for electronAlg in electronAlgsCnv:
        cfg.addEventAlgo(electronAlg, electronSequenceCnv.getName())

    # Include, and then set up the photon analysis sequence:
    from EgammaAnalysisAlgorithms.PhotonAnalysisSequence import (
        makePhotonAnalysisSequence,
    )

    photonSequence = makePhotonAnalysisSequence(
        dataType, workingPoint="Loose.Undefined", postfix="loose"
    )
    photonSequence.configure(
        inputName=photonsContainerName, outputName="AnalysisPhotons_%SYS%"
    )
    # print(photonSequence)  # For debugging
    # Convert to new configurables
    photonSequenceCnv, photonAlgsCnv = convertSequenceAndGetAlgs(
        CompFactory, photonSequence
    )
    cfg.addSequence(photonSequenceCnv)
    for photonAlg in photonAlgsCnv:
        cfg.addEventAlgo(photonAlg, photonSequenceCnv.getName())

    # Include, and then set up the muon analysis algorithm sequence:
    from MuonAnalysisAlgorithms.MuonAnalysisSequence import makeMuonAnalysisSequence

    muonSequence = makeMuonAnalysisSequence(
        dataType,
        workingPoint="Loose.NonIso",
        postfix="loose",
    )
    muonSequence.configure(
        inputName=muonsContainerName, outputName="AnalysisMuons_%SYS%"
    )
    # print(muonLooseSequence)  # For debugging
    # Convert to new configurables
    muonSequenceCnv, muonAlgsCnv = convertSequenceAndGetAlgs(CompFactory, muonSequence)
    cfg.addSequence(muonSequenceCnv)
    for muonAlg in muonAlgsCnv:
        cfg.addEventAlgo(muonAlg, muonSequenceCnv.getName())

    from JetAnalysisAlgorithms.JetAnalysisSequence import makeJetAnalysisSequence

    jetSequence = makeJetAnalysisSequence(
        dataType,
        reco4JetContainerName,
        postfix="smallR",
        deepCopyOutput=True,
        shallowViewOutput=False,
        runGhostMuonAssociation=False,
        runFJvtUpdate=False,
        runFJvtSelection=False,
        runJvtSelection=False,
    )

    from FTagAnalysisAlgorithms.FTagAnalysisSequence import makeFTagAnalysisSequence

    makeFTagAnalysisSequence(
        jetSequence,
        dataType,
        reco4JetContainerName,
        btagWP="FixedCutBEff_77",
        btagger="DL1r",  # DL1dv00 not available in makeFTagAnalysisSequence CDI
        generator="default",  # Pythia8 not available in makeFTagAnalysisSequence CDI
        postfix="",
        preselection=None,
        kinematicSelection=False,
        noEfficiency=False,
        legacyRecommendations=True,
        enableCutflow=False,
        minPt=20000,
    )

    jetSequence.configure(
        inputName=reco4JetContainerName, outputName="AnalysisJetsBTAG_%SYS%"
    )

    # print(jetSequence)  # For debugging
    # Convert to new configurables
    jetSequenceCnv, jetAlgsCnv = convertSequenceAndGetAlgs(CompFactory, jetSequence)
    cfg.addSequence(jetSequenceCnv)
    for jetAlg in jetAlgsCnv:
        cfg.addEventAlgo(jetAlg, jetSequenceCnv.getName())

    # Define and configure a tool instance
    # Properties can be set as keyword arguments to the tool constructor
    # bTagSelectionTool = CompFactory.BTaggingSelectionTool(
    #     "bTagSelectionTool",
    #     FlvTagCutDefinitionsFileName=(
    #         "xAODBTaggingEfficiency/13TeV/2021-22-13TeV-MC16-CDI-2021-12-02_v2.root"
    #     ),
    #     TaggerName="DL1dv00",
    #     OperatingPoint="FixedCutBEff_77",
    #     JetAuthor=reco4JetContainerName,
    #     MinPt=20e3,
    #     MaxEta=2.5,
    # )

    from JetAnalysisAlgorithms.JetAnalysisSequence import makeJetAnalysisSequence

    largeRrecojetSequence = makeJetAnalysisSequence(
        dataType,
        reco10JetContainerName,
        postfix="largeR",
        deepCopyOutput=True,
        shallowViewOutput=False,
        runGhostMuonAssociation=False,
        largeRMass="Comb",
    )

    largeRrecojetSequence.configure(
        inputName=reco10JetContainerName, outputName="AnalysisLargeRRecoJets_%SYS%"
    )
    # print(largeRrecojetSequence)  # For debugging
    # Convert to new configurables
    largeRrecojetSequenceCnv, largeJetAlgsCnv = convertSequenceAndGetAlgs(
        CompFactory, largeRrecojetSequence
    )
    cfg.addSequence(largeRrecojetSequenceCnv)
    for largeJetAlg in largeJetAlgsCnv:
        cfg.addEventAlgo(largeJetAlg, largeRrecojetSequenceCnv.getName())

    cfg.addEventAlgo(
        CompFactory.HH4B.VariableDumperAlg(
            "VariableDumper",
            EventInfoKey="EventInfo",
            RootStreamName="ANALYSIS",
            # RootDirName="Reco",
            # BTaggingSelectionTool=bTagSelectionTool,
        )
    )

    # Create analysis mini-ntuple
    treeMaker = CompFactory.getComp("CP::TreeMakerAlg")("TreeMaker")
    treeMaker.TreeName = "AnalysisMiniTree_NOSYS"

    # Add event info
    cfg.addEventAlgo(treeMaker)
    ntupleMaker = CompFactory.getComp("CP::AsgxAODNTupleMakerAlg")(
        "NTupleMakerEventInfo"
    )
    ntupleMaker.TreeName = "AnalysisMiniTree_NOSYS"
    ntupleMaker.Branches = [
        "EventInfo.runNumber     -> runNumber",
        "EventInfo.eventNumber   -> eventNumber",
        # Having some issues retrieving this value
        # "EventInfo.mcEventWeight   -> mcEventWeight",
    ]
    cfg.addEventAlgo(ntupleMaker)

    # Add electrons info
    ntupleMaker = CompFactory.getComp("CP::AsgxAODNTupleMakerAlg")(
        "NTupleMakerElectrons"
    )
    ntupleMaker.TreeName = "AnalysisMiniTree_NOSYS"
    ntupleMaker.Branches = [
        "AnalysisElectrons_NOSYS.m  -> el_m",
        "AnalysisElectrons_NOSYS.pt  -> el_pt",
        "AnalysisElectrons_NOSYS.eta -> el_eta",
        "AnalysisElectrons_NOSYS.phi -> el_phi",
        # "AnalysisElectrons_%SYS%.pt  -> el_%SYS%_pt",
    ]
    cfg.addEventAlgo(ntupleMaker)

    # Add photons info
    ntupleMaker = CompFactory.getComp("CP::AsgxAODNTupleMakerAlg")("NTupleMakerPhotons")
    ntupleMaker.TreeName = "AnalysisMiniTree_NOSYS"
    ntupleMaker.Branches = [
        "AnalysisPhotons_NOSYS.m  -> ph_m",
        "AnalysisPhotons_NOSYS.pt  -> ph_pt",
        "AnalysisPhotons_NOSYS.eta -> ph_eta",
        "AnalysisPhotons_NOSYS.phi -> ph_phi",
        # "AnalysisPhotons_%SYS%.pt  -> ph_%SYS%_pt",
    ]
    cfg.addEventAlgo(ntupleMaker)

    # Add muons info
    # Having some issues wit the muons
    # ntupleMaker = CompFactory.getComp("CP::AsgxAODNTupleMakerAlg")("NTupleMakerMuons")
    # ntupleMaker.TreeName = "AnalysisMiniTree_NOSYS"
    # ntupleMaker.Branches = [
    #     "AnalysisMuons_NOSYS.m  -> mu_m",
    #     "AnalysisMuons_NOSYS.pt  -> mu_pt",
    #     "AnalysisMuons_NOSYS.eta -> mu_eta",
    #     "AnalysisMuons_NOSYS.phi -> mu_phi",
    #     # "AnalysisMuons_%SYS%.pt  -> mu_%SYS%_pt",
    # ]
    # cfg.addEventAlgo(ntupleMaker)

    # Add small R jet info
    ntupleMaker = CompFactory.getComp("CP::AsgxAODNTupleMakerAlg")(
        "NTupleMakerSmallRJets"
    )
    ntupleMaker.TreeName = "AnalysisMiniTree_NOSYS"
    ntupleMaker.Branches = [
        "AnalysisJetsBTAG_NOSYS.m  -> recojet_antikt4_m",
        "AnalysisJetsBTAG_NOSYS.pt  -> recojet_antikt4_pt",
        "AnalysisJetsBTAG_NOSYS.eta -> recojet_antikt4_eta",
        "AnalysisJetsBTAG_NOSYS.phi -> recojet_antikt4_phi",
        # "AnalysisJetsBTAG_%SYS%.pt  -> recojet_antikt4_%SYS%_pt",
    ]
    cfg.addEventAlgo(ntupleMaker)

    # Add large R jet info
    ntupleMaker = CompFactory.getComp("CP::AsgxAODNTupleMakerAlg")(
        "NTupleMakerLargeRJets"
    )
    ntupleMaker.TreeName = "AnalysisMiniTree_NOSYS"
    ntupleMaker.Branches = [
        "AnalysisLargeRRecoJets_NOSYS.m  -> recojet_antikt10_m",
        "AnalysisLargeRRecoJets_NOSYS.pt  -> recojet_antikt10_pt",
        "AnalysisLargeRRecoJets_NOSYS.eta -> recojet_antikt10_eta",
        "AnalysisLargeRRecoJets_NOSYS.phi -> recojet_antikt10_phi",
        # "AnalysisLargeRRecoJets_%SYS%.pt  -> recojet_antikt10_%SYS%_pt",
    ]
    cfg.addEventAlgo(ntupleMaker)

    # Fill tree
    treeFiller = CompFactory.getComp("CP::TreeFillerAlg")("TreeFiller")
    treeFiller.TreeName = "AnalysisMiniTree_NOSYS"
    cfg.addEventAlgo(treeFiller)

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
