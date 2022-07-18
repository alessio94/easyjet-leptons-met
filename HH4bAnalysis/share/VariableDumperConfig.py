#!/bin/env python
################################################################################
# VariableDumperConfig.py
# A simple CA file to create a tree of variables
#
# Author: Victor Ruelas

# Basic setup
import sys

from AthenaCommon import Logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from utils.argsHelper import checkArgs
from utils.containerNameHelper import getContainerName
from utils.convertOldConfigHelper import convertSequenceAndGetAlgs

variabledumperlog = Logging.logging.getLogger("VariableDumperConfig")


def pileupConfigFiles(filename):
    """Return the PRW (Pileup ReWeighting) config files and lumicalc files"""
    filename = filename.split("/")
    dataset = filename[-2]
    project, dsid, physics_short, prod_step, dtype, tags = dataset.split(".")
    if "mc" in project:
        split_tags = tags.split("_")
        print(split_tags)
        # Figure out which MC we are using
        if "r13167" in split_tags:
            subcampaign = "mc20a"
            lumicalcFiles = [
                "GoodRunsLists/data15_13TeV/20170619/PHYS_StandardGRL_All_Good_25ns_276262-284484_OflLumi-13TeV-008.root",  # noqa
                "GoodRunsLists/data16_13TeV/20180129/PHYS_StandardGRL_All_Good_25ns_297730-311481_OflLumi-13TeV-009.root",  # noqa
            ]
            actual_mu = []
        elif "r13144" in split_tags:
            subcampaign = "mc20d"
            lumicalcFiles = [
                "GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.lumicalc.OflLumi-13TeV-010.root",  # noqa
            ]
            actual_mu = [
                "GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.actualMu.OflLumi-13TeV-010.root"  # noqa
            ]
        elif "r13145" in split_tags:
            subcampaign = "mc20e"
            lumicalcFiles = [
                "GoodRunsLists/data18_13TeV/20190318/ilumicalc_histograms_None_348885-364292_OflLumi-13TeV-010.root"  # noqa
            ]
            actual_mu = [
                "GoodRunsLists/data18_13TeV/20190318/physics_25ns_Triggerno17e33prim.actualMu.OflLumi-13TeV-010.root"  # noqa
            ]
        else:
            variabledumperlog.error("Cannot determine subcampaign for " + dataset)
            sys.exit(1)

        prwFiles = [
            "dev/PileupReweighting/share/DSID{0}xxx/pileup_{1}_dsid{2}_{3}.root".format(  # noqa
                dsid[:3], subcampaign, dsid, "AFII" if "a" in tags else "FS"
            )
        ]
        prwFiles += actual_mu

    return prwFiles, lumicalcFiles


# Generate the algorithm to do the dumping.
# AthAlgSequence does not respect filter decisions,
# so we will need to add a new sequence to the CA
def VariableDumperCfg(flags, outfname, is_daod_physlite, btag_wps):
    dataType = "mc" if flags.Input.isMC else "data"

    reco4JetContainerName = getContainerName("Reco4PFlowJets", is_daod_physlite)
    reco10JetContainerName = getContainerName("Reco10PFlowJets", is_daod_physlite)
    muonsContainerName = getContainerName("Muons", is_daod_physlite)
    electronsContainerName = getContainerName("Electrons", is_daod_physlite)
    photonsContainerName = getContainerName("Photons", is_daod_physlite)

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
    # Create SystematicsSvc explicitly:
    cfg.addService(CompFactory.getComp("CP::SystematicsSvc")("SystematicsSvc"))

    # Include, and then set up the pileup analysis sequence:
    prwFiles, lumicalcFiles = pileupConfigFiles(*flags.Input.Files)

    # Create a pile-up analysis sequence
    from AsgAnalysisAlgorithms.PileupAnalysisSequence import makePileupAnalysisSequence

    pileupSequence = makePileupAnalysisSequence(
        dataType,
        userPileupConfigs=prwFiles,
        userLumicalcFiles=lumicalcFiles,
        autoConfig=False,
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
        dataType,
        workingPoint="LooseLHElectron.NonIso",
        postfix="loose",
        deepCopyOutput=False,
        shallowViewOutput=True,
        recomputeLikelihood=False,
        chargeIDSelection=False,
        isolationCorrection=False,
        crackVeto=False,
        ptSelectionOutput=False,
        enableCutflow=False,
        enableKinematicHistograms=False,
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
        dataType,
        workingPoint="Loose.Undefined",
        postfix="loose",
        deepCopyOutput=False,
        shallowViewOutput=True,
        crackVeto=False,
        enableCleaning=True,
        cleaningAllowLate=False,
        recomputeIsEM=False,
        ptSelectionOutput=False,
        enableCutflow=False,
        enableKinematicHistograms=False,
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
        deepCopyOutput=False,
        shallowViewOutput=True,
        ptSelectionOutput=False,
        qualitySelectionOutput=True,
        enableCutflow=False,
        enableKinematicHistograms=False,
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
        jetCollection=reco4JetContainerName,
        postfix="smallR",
        deepCopyOutput=False,
        shallowViewOutput=True,
        runGhostMuonAssociation=True,
        enableCutflow=False,
        enableKinematicHistograms=False,
        runFJvtUpdate=False,
        runFJvtSelection=False,
        runJvtSelection=False,
    )

    from FTagAnalysisAlgorithms.FTagAnalysisSequence import makeFTagAnalysisSequence

    bTagCalibFile = (
        "xAODBTaggingEfficiency/13TeV/2021-22-13TeV-MC16-CDI-2021-12-02_v2.root"
    )
    for tagger_wp in btag_wps:
        tagger, btag_wp = tagger_wp.split("_", 1)
        ftagSeq = makeFTagAnalysisSequence(
            jetSequence,
            dataType,
            jetCollection=reco4JetContainerName,
            btagWP=btag_wp,
            btagger=tagger,
            generator="Pythia8",
            minPt=20000,
            postfix="",
            preselection=None,
            kinematicSelection=False,
            noEfficiency=False,
            legacyRecommendations=False,
            enableCutflow=False,
        )
        # Hack until this is merged:
        # https://gitlab.cern.ch/atlas/athena/-/merge_requests/54939]
        for ftagAlg in ftagSeq:
            if "FTagSelectionAlg" in ftagAlg.getName():
                ftagAlg.selectionTool.FlvTagCutDefinitionsFileName = bTagCalibFile
            if "FTagEfficiencyScaleFactorAlg" in ftagAlg.getName():
                ftagAlg.efficiencyTool.ScaleFactorFileName = bTagCalibFile

    jetSequence.configure(
        inputName=reco4JetContainerName,
        outputName="AnalysisJets_%SYS%",
    )
    # print(jetSequence)  # For debugging
    # Convert to new configurables
    jetSequenceCnv, jetAlgsCnv = convertSequenceAndGetAlgs(CompFactory, jetSequence)
    cfg.addSequence(jetSequenceCnv)
    for jetAlg in jetAlgsCnv:
        cfg.addEventAlgo(jetAlg, jetSequenceCnv.getName())

    from JetAnalysisAlgorithms.JetAnalysisSequence import makeJetAnalysisSequence

    largeRrecojetSequence = makeJetAnalysisSequence(
        dataType,
        reco10JetContainerName,
        postfix="largeR",
        deepCopyOutput=False,
        shallowViewOutput=True,
        runGhostMuonAssociation=False,
        enableCutflow=False,
        enableKinematicHistograms=False,
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

    # Include, and then set up the overlap analysis algorithm sequence:
    from AsgAnalysisAlgorithms.OverlapAnalysisSequence import (
        makeOverlapAnalysisSequence,
    )

    overlapSequence = makeOverlapAnalysisSequence(
        dataType,
        inputLabel="",
        outputLabel="passesOR",
        linkOverlapObjects=False,
        doEleEleOR=False,
        doMuPFJetOR=True,
        doTaus=False,
        doElectrons=True,
        doMuons=True,
        doJets=True,
        doPhotons=True,
        doFatJets=True,
        enableUserPriority=False,
        bJetLabel="",
        boostedLeptons=False,
        postfix="",
        shallowViewOutput=True,
        enableCutflow=False,
    )
    overlapSequence.configure(
        inputName={
            "electrons": "AnalysisElectrons_%SYS%",
            "photons": "AnalysisPhotons_%SYS%",
            "muons": "AnalysisMuons_%SYS%",
            "jets": "AnalysisJets_%SYS%",
            "fatJets": "AnalysisLargeRRecoJets_%SYS%",
            # 'taus'      : 'AnalysisTauJets_%SYS%'
        },
        outputName={
            "electrons": "AnalysisElectronsOR_%SYS%",
            "photons": "AnalysisPhotonsOR_%SYS%",
            "muons": "AnalysisMuonsOR_%SYS%",
            "jets": "AnalysisJetsOR_%SYS%",
            "fatJets": "AnalysisLargeRRecoJetsOR_%SYS%",
            # 'taus'      : 'AnalysisTauJetsOR_%SYS%'
        },
    )
    # print(overlapSequence)  # For debugging
    # Convert to new configurables
    overlapSequenceCnv, overlapAlgsCnv = convertSequenceAndGetAlgs(
        CompFactory, overlapSequence
    )
    cfg.addSequence(overlapSequenceCnv)
    for overlapAlg in overlapAlgsCnv:
        cfg.addEventAlgo(overlapAlg, overlapSequenceCnv.getName())

    cfg.addEventAlgo(
        CompFactory.HH4B.VariableDumperAlg(
            "VariableDumper",
            EventInfoKey="EventInfo",
            RootStreamName="ANALYSIS",
            applyJetCleaning=True,
        )
    )

    # Create analysis mini-ntuple
    treeMaker = CompFactory.getComp("CP::TreeMakerAlg")("TreeMaker")
    treeMaker.TreeName = "AnalysisMiniTree"

    # Add event info
    cfg.addEventAlgo(treeMaker)
    ntupleMaker = CompFactory.getComp("CP::AsgxAODNTupleMakerAlg")("NTupleMaker")
    ntupleMaker.TreeName = "AnalysisMiniTree"
    ntupleMaker.Branches = [
        "EventInfo.runNumber     -> runNumber",
        "EventInfo.eventNumber   -> eventNumber",
        "EventInfo.PileupWeight_%SYS% -> pileupWeight_%SYS%",
        "EventInfo.mcEventWeights   -> mcEventWeights",
        "AnalysisElectrons_%SYS%.pt  -> el_%SYS%_pt",
        "AnalysisElectrons_%SYS%.eta -> el_%SYS%_eta",
        "AnalysisElectrons_%SYS%.phi -> el_%SYS%_phi",
        "AnalysisElectronsOR_%SYS%.eta -> el_OR_%SYS%_eta",
        "AnalysisElectronsOR_%SYS%.phi -> el_OR_%SYS%_phi",
        "AnalysisElectronsOR_%SYS%.pt  -> el_OR_%SYS%_pt",
        "AnalysisPhotons_%SYS%.pt  -> ph_%SYS%_pt",
        "AnalysisPhotons_%SYS%.eta -> ph_%SYS%_eta",
        "AnalysisPhotons_%SYS%.phi -> ph_%SYS%_phi",
        "AnalysisPhotonsOR_%SYS%.eta -> ph_OR_%SYS%_eta",
        "AnalysisPhotonsOR_%SYS%.phi -> ph_OR_%SYS%_phi",
        "AnalysisPhotonsOR_%SYS%.pt  -> ph_OR_%SYS%_pt",
        "AnalysisMuons_%SYS%.pt  -> mu_%SYS%_pt",
        "AnalysisMuons_%SYS%.eta -> mu_%SYS%_eta",
        "AnalysisMuons_%SYS%.phi -> mu_%SYS%_phi",
        "AnalysisMuonsOR_%SYS%.eta -> mu_OR_%SYS%_eta",
        "AnalysisMuonsOR_%SYS%.phi -> mu_OR_%SYS%_phi",
        "AnalysisMuonsOR_%SYS%.pt  -> mu_OR_%SYS%_pt",
        "AnalysisJets_%SYS%.m  -> recojet_antikt4_%SYS%_m",
        "AnalysisJets_%SYS%.pt  -> recojet_antikt4_%SYS%_pt",
        "AnalysisJets_%SYS%.eta -> recojet_antikt4_%SYS%_eta",
        "AnalysisJets_%SYS%.phi -> recojet_antikt4_%SYS%_phi",
        "AnalysisJetsOR_%SYS%.m  -> recojet_antikt4_OR_%SYS%_m",
        "AnalysisJetsOR_%SYS%.pt  -> recojet_antikt4_OR_%SYS%_pt",
        "AnalysisJetsOR_%SYS%.eta -> recojet_antikt4_OR_%SYS%_eta",
        "AnalysisJetsOR_%SYS%.phi -> recojet_antikt4_OR_%SYS%_phi",
        "AnalysisLargeRRecoJets_%SYS%.m  -> recojet_antikt10_%SYS%_m",
        "AnalysisLargeRRecoJets_%SYS%.pt  -> recojet_antikt10_%SYS%_pt",
        "AnalysisLargeRRecoJets_%SYS%.eta -> recojet_antikt10_%SYS%_eta",
        "AnalysisLargeRRecoJets_%SYS%.phi -> recojet_antikt10_%SYS%_phi",
        "AnalysisLargeRRecoJetsOR_%SYS%.m  -> recojet_antikt10_OR_%SYS%_m",
        "AnalysisLargeRRecoJetsOR_%SYS%.pt  -> recojet_antikt10_OR_%SYS%_pt",
        "AnalysisLargeRRecoJetsOR_%SYS%.eta -> recojet_antikt10_OR_%SYS%_eta",
        "AnalysisLargeRRecoJetsOR_%SYS%.phi -> recojet_antikt10_OR_%SYS%_phi",
    ]
    ntupleMaker.Branches += [
        f"AnalysisJets_%SYS%.ftag_select_{btag_wp} -> recojet_antikt4_%SYS%_{btag_wp}"
        for btag_wp in btag_wps
    ]
    cfg.addEventAlgo(ntupleMaker)

    # Fill tree
    treeFiller = CompFactory.getComp("CP::TreeFillerAlg")("TreeFiller")
    treeFiller.TreeName = "AnalysisMiniTree"
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
        parser.add_argument(
            "--btag-wps",
            type=str,
            nargs="+",
            default=[
                "DL1dv00_FixedCutBEff_77",
                "DL1dv00_FixedCutBEff_85",
            ],
            help="btag working points default %(default)s",
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
                ConfigFlags,
                outfname=args.outFile,
                is_daod_physlite=args.daod_physlite,
                btag_wps=args.btag_wps,
            )
        )

        # Print the full job configuration
        cfg.printConfig()

    # Execute the job defined in the ComponentAccumulator.
    # The number of events is specified by `args.evtMax`
    return cfg.run(args.evtMax)


# Execute the main function if this file was executed as a script
if __name__ == "__main__":
    code = main()
    sys.exit(0 if code.isSuccess() else 1)
