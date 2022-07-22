#!/bin/env python

#
# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
#

#
# VariableDumperConfig.py
# A simple CA file to create a tree of variables
#
# Author: Victor Ruelas

import sys

from AthenaCommon import Logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from Config.Base import pileupConfigFiles
from Algs.Event import makeAndAddTriggerAnalysisAlgs, makeAndAddPileupAnalysisSequence
from Algs.Electrons import makeAndAddElectronAnalysisSequence
from Algs.Photons import makeAndAddPhotonAnalysisSequence
from Algs.Muons import makeAndAddMuonAnalysisSequence
from Algs.Jets import makeAndAddJetAnalysisSequence, makeAndAddFatJetAnalysisSequence
from Algs.Postprocessing import makeAndAddOverlapAnalysisSequence
from Algs.Tree import makeAndAddAnalysisTreeAlg
from utils.argsHelper import checkArgs
from utils.containerNameHelper import getContainerName

variabledumperlog = Logging.logging.getLogger("VariableDumperConfig")


# Generate the algorithm to do the dumping.
# AthAlgSequence does not respect filter decisions,
# so we will need to add a new sequence to the CA
def VariableDumperCfg(flags, outfname, is_daod_physlite, btag_wps, trigger_chains):
    dataType = "mc" if flags.Input.isMC else "data"

    reco4JetInputContainerName = getContainerName("Reco4PFlowJets", is_daod_physlite)
    reco10JetInputContainerName = getContainerName("Reco10PFlowJets", is_daod_physlite)
    muonsInputContainerName = getContainerName("Muons", is_daod_physlite)
    electronsInputContainerName = getContainerName("Electrons", is_daod_physlite)
    photonsInputContainerName = getContainerName("Photons", is_daod_physlite)

    reco4JetOutputContainerName = f"Analysis{reco4JetInputContainerName}_%SYS%"
    reco10JetOutputContainerName = f"Analysis{reco10JetInputContainerName}_%SYS%"
    muonsOutputContainerName = f"Analysis{muonsInputContainerName}_%SYS%"
    electronsOutputContainerName = f"Analysis{electronsInputContainerName}_%SYS%"
    photonsOutputContainerName = f"Analysis{photonsInputContainerName}_%SYS%"

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

    try:
        # Include, and then set up the pileup analysis sequence:
        prwFiles, lumicalcFiles = pileupConfigFiles(*flags.Input.Files)
    except LookupError as err:
        variabledumperlog.error(err)

    # Adds variable to EventInfo if trigger passed or not, for example:
    # EventInfo.HLT_j420 -> 1 # or 0
    makeAndAddTriggerAnalysisAlgs(cfg, flags, trigger_chains)

    # Adds variable to EventInfo if for pileup weight, for example:
    # EventInfo.PileWeight_%SYS$ -> ?
    makeAndAddPileupAnalysisSequence(
        cfg,
        dataType=dataType,
        prwFiles=prwFiles,
        lumicalcFiles=lumicalcFiles,
    )

    makeAndAddElectronAnalysisSequence(
        cfg,
        dataType=dataType,
        inputContainerName=electronsInputContainerName,
        outputContainerName=electronsOutputContainerName,
    )

    makeAndAddPhotonAnalysisSequence(
        cfg,
        dataType=dataType,
        inputContainerName=photonsInputContainerName,
        outputContainerName=photonsOutputContainerName,
    )

    makeAndAddMuonAnalysisSequence(
        cfg,
        dataType=dataType,
        inputContainerName=muonsInputContainerName,
        outputContainerName=muonsOutputContainerName,
    )

    makeAndAddJetAnalysisSequence(
        cfg,
        dataType=dataType,
        inputContainerName=reco4JetInputContainerName,
        outputContainerName=reco4JetOutputContainerName,
        workingPoints=btag_wps,
    )

    makeAndAddFatJetAnalysisSequence(
        cfg,
        dataType=dataType,
        inputContainerName=reco10JetInputContainerName,
        outputContainerName=reco10JetOutputContainerName,
    )

    # Add the custom alg for information not avaiable through CP algs
    cfg.addEventAlgo(
        CompFactory.HH4B.VariableDumperAlg(
            "VariableDumper",
            EventInfoKey="EventInfo",
            ElectronsKey=electronsOutputContainerName,
            PhotosKey=photonsInputContainerName,
            MuonsKey=muonsInputContainerName,
            SmallJetKey=reco4JetInputContainerName,
            LargeJetKey=reco10JetOutputContainerName,
            RootStreamName="ANALYSIS",
            applyJetCleaning=True,
        )
    )

    ###
    # Beging postprocessing
    ###

    # Include, and then set up the overlap analysis algorithm sequence:
    overlapInputNames = {
        "electrons": electronsOutputContainerName,
        "photons": photonsOutputContainerName,
        "muons": muonsOutputContainerName,
        "jets": reco4JetOutputContainerName,
        "fatJets": reco10JetOutputContainerName,
        # 'taus'      : 'AnalysisTauJets_%SYS%'
    }
    overlapOutputNames = {
        "electrons": f"{electronsOutputContainerName}_OR",
        "photons": f"{photonsOutputContainerName}_OR",
        "muons": f"{muonsOutputContainerName}_OR",
        "jets": f"{reco4JetOutputContainerName}_OR",
        "fatJets": f"{reco10JetOutputContainerName}_OR",
        # 'taus'      : 'AnalysisTauJetsOR_%SYS%'
    }
    makeAndAddOverlapAnalysisSequence(
        cfg,
        dataType=dataType,
        inputNames=overlapInputNames,
        outputNames=overlapOutputNames,
    )

    # Create analysis mini-ntuple
    analysisTreeBranches = [
        "EventInfo.runNumber     -> runNumber",
        "EventInfo.eventNumber   -> eventNumber",
        "EventInfo.mcEventWeights   -> mcEventWeights",
        "EventInfo.PileupWeight_%SYS% -> pileupWeight_%SYS%",
        f"{electronsOutputContainerName}.pt  -> el_%SYS%_pt",
        f"{electronsOutputContainerName}.eta -> el_%SYS%_eta",
        f"{electronsOutputContainerName}.phi -> el_%SYS%_phi",
        f"{electronsOutputContainerName}_OR.eta -> el_OR_%SYS%_eta",
        f"{electronsOutputContainerName}_OR.phi -> el_OR_%SYS%_phi",
        f"{electronsOutputContainerName}_OR.pt  -> el_OR_%SYS%_pt",
        f"{photonsOutputContainerName}.pt  -> ph_%SYS%_pt",
        f"{photonsOutputContainerName}.eta -> ph_%SYS%_eta",
        f"{photonsOutputContainerName}.phi -> ph_%SYS%_phi",
        f"{photonsOutputContainerName}_OR.eta -> ph_OR_%SYS%_eta",
        f"{photonsOutputContainerName}_OR.phi -> ph_OR_%SYS%_phi",
        f"{photonsOutputContainerName}_OR.pt  -> ph_OR_%SYS%_pt",
        f"{muonsOutputContainerName}.pt  -> mu_%SYS%_pt",
        f"{muonsOutputContainerName}.eta -> mu_%SYS%_eta",
        f"{muonsOutputContainerName}.phi -> mu_%SYS%_phi",
        f"{muonsOutputContainerName}_OR.eta -> mu_OR_%SYS%_eta",
        f"{muonsOutputContainerName}_OR.phi -> mu_OR_%SYS%_phi",
        f"{muonsOutputContainerName}_OR.pt  -> mu_OR_%SYS%_pt",
        f"{reco4JetOutputContainerName}.m  -> recojet_antikt4_%SYS%_m",
        f"{reco4JetOutputContainerName}.pt  -> recojet_antikt4_%SYS%_pt",
        f"{reco4JetOutputContainerName}.eta -> recojet_antikt4_%SYS%_eta",
        f"{reco4JetOutputContainerName}.phi -> recojet_antikt4_%SYS%_phi",
        f"{reco4JetOutputContainerName}_OR.m  -> recojet_antikt4_OR_%SYS%_m",
        f"{reco4JetOutputContainerName}_OR.pt  -> recojet_antikt4_OR_%SYS%_pt",
        f"{reco4JetOutputContainerName}_OR.eta -> recojet_antikt4_OR_%SYS%_eta",
        f"{reco4JetOutputContainerName}_OR.phi -> recojet_antikt4_OR_%SYS%_phi",
        f"{reco10JetOutputContainerName}.m  -> recojet_antikt10_%SYS%_m",
        f"{reco10JetOutputContainerName}.pt  -> recojet_antikt10_%SYS%_pt",
        f"{reco10JetOutputContainerName}.eta -> recojet_antikt10_%SYS%_eta",
        f"{reco10JetOutputContainerName}.phi -> recojet_antikt10_%SYS%_phi",
        f"{reco10JetOutputContainerName}_OR.m  -> recojet_antikt10_OR_%SYS%_m",
        f"{reco10JetOutputContainerName}_OR.pt  -> recojet_antikt10_OR_%SYS%_pt",
        f"{reco10JetOutputContainerName}_OR.eta -> recojet_antikt10_OR_%SYS%_eta",
        f"{reco10JetOutputContainerName}_OR.phi -> recojet_antikt10_OR_%SYS%_phi",
    ]
    analysisTreeBranches += [
        f"EventInfo.trigPassed_{trig_chain} -> trigPassed_{trig_chain}"
        for trig_chain in trigger_chains
    ]
    analysisTreeBranches += [
        f"{reco4JetOutputContainerName}.ftag_select_{btag_wp}"
        f" -> recojet_antikt4_%SYS%_{btag_wp}"
        for btag_wp in btag_wps
    ]
    makeAndAddAnalysisTreeAlg(cfg, branches=analysisTreeBranches)

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
        parser.add_argument(
            "--trigger-chains",
            type=str,
            nargs="+",
            default=[
                "HLT_j420",
                "HLT_j460",
            ],
            help="trigger chains default %(default)s",
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
                trigger_chains=args.trigger_chains,
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
