#!/usr/bin/env python

#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

# Basic postProcess jobOptions, to be adapted for each analysis

import sys
import yaml
import ROOT

from argparse import ArgumentParser
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg


def RunEasyjetPlus(args):
    flags = initConfigFlags()

    # Dummy event number because we run some execute method not in xAOD inputs
    flags.Exec.MaxEvents = 1

    flags.lock()

    acc = MainServicesCfg(flags)

    # Dummy tools for demonstration; replace with your actual tools and configuration
    SOWTool = CompFactory.SumOfWeightsTool(inFile=args.inFile)

    # Get XSection Path
    with open(args.xSectionsConfig, 'r') as file:
        XSectionData = yaml.safe_load(file)
    # Get XSection from either custom file (which is in PMG format)
    # or from an official PMG file
    XSectionTool = CompFactory.GetXSectionTool(
        pathsToPMGFiles=XSectionData['XSection_paths'])

    TotalWeightsTool_bbyy = CompFactory.TotalWeightsTool(
        analysis="bbyy", nPhotons=2, bTagWP="GN2v00LegacyWP_FixedCutBEff_77")

    acc.addEventAlgo(CompFactory.PostProcessor(
        inFile=args.inFile,
        outFile=args.outFile,
        maxEvents=args.maxEvents,
        copyInputs=args.copyInputs,
        postProcessTools=[SOWTool, XSectionTool, TotalWeightsTool_bbyy]
    ))

    # Execute and finish
    sc = acc.run()

    # Success should be 0
    if not sc.isSuccess():
        print("Athena job failed.")
        sys.exit(1)


def mergeFiles(inFileName, outFileName):
    inFile = ROOT.TFile.Open(inFileName, "UPDATE")
    outFile = ROOT.TFile.Open(outFileName, "READ")

    t1 = inFile.Get("AnalysisMiniTree")
    t2 = outFile.Get("AnalysisMiniTree")

    if not t1:
        print("Error: Could not retrieve the TTree from inFile.")
        sys.exit(1)

    if not t2:
        print("Error: Could not retrieve the TTree from outFile.")
        sys.exit(1)

    # Save all branches of outFile to the inFile
    t1.AddFriend(t2, "friendTree")
    df = ROOT.RDataFrame(t1)

    opts = ROOT.RDF.RSnapshotOptions()
    opts.fMode = "RECREATE"
    df.Snapshot("AnalysisMiniTree", inFileName, "", opts)


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("--inFile", required=True)
    parser.add_argument("--outFile", default="SumOfWeights_test.root")
    parser.add_argument("--maxEvents", default=-1, type=int)
    parser.add_argument("--xSectionsConfig", required=True)
    parser.add_argument("--copyInputs", action='store_true',
                        help="Copy pre-processed branches to outFile.")
    parser.add_argument("--mergeMyFiles", action='store_true',
                        help="Merge branches of outFile into the inFile.")

    args = parser.parse_args()

    RunEasyjetPlus(args)

    if (args.mergeMyFiles):
        mergeFiles(args.inFile, args.outFile)
