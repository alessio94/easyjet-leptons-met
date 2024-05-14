#!/usr/bin/env python

#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

# Basic postProcess jobOptions, to be adapted for each analysis

import sys
import yaml

from argparse import ArgumentParser
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

from ttHHAnalysis.ttHH_config import FullPath
from EasyjetPlus.PostProcessTools import mergeFiles


def RunEasyjetPlus(args):
    flags = initConfigFlags()

    # Dummy event number because we run some execute method not in xAOD inputs
    flags.Exec.MaxEvents = 1

    flags.lock()

    acc = MainServicesCfg(flags)

    SOWTool = CompFactory.SumOfWeightsTool(inFile=args.inFile)

    # Get XSection Path
    with open(FullPath(args.xSectionsConfig), 'r') as file:
        XSectionData = yaml.safe_load(file)
    # Get XSection from either custom file (which is in PMG format)
    # or from an official PMG file
    XSectionTool = CompFactory.GetXSectionTool(
        pathsToPMGFiles=XSectionData['XSection_paths'])

    TotalWeightsTool = CompFactory.TotalWeightsTool(
        analysis="ttHH", bTagWP="GN2v00LegacyWP_FixedCutBEff_85")

    acc.addEventAlgo(CompFactory.PostProcessor(
        inFile=args.inFile,
        outFile=args.outFile,
        maxEvents=args.maxEvents,
        copyInputs=args.copyInputs,
        postProcessTools=[SOWTool, XSectionTool, TotalWeightsTool]
    ))

    # Execute and finish
    sc = acc.run()

    # Success should be 0
    if not sc.isSuccess():
        print("Athena job failed.")
        sys.exit(1)


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("--inFile", required=True)
    parser.add_argument("--outFile", default="SumOfWeights_test.root")
    parser.add_argument("--maxEvents", default=-1, type=int)
    parser.add_argument("--xSectionsConfig", required=True)
    parser.add_argument("--copyInputs", action='store_true',
                        help="Copy pre-processed branches to outFile.")
    parser.add_argument("--mergeMyFiles", action='store_true',
                        help="Merge branches. Default is merge output to input")
    parser.add_argument("--mergeToOutput", action='store_true',
                        help="Can apply with mergeMyFiles. Merge input to output")

    args = parser.parse_args()

    RunEasyjetPlus(args)

    if (args.mergeMyFiles):
        mergeFiles(args.inFile, args.outFile, args.mergeToOutput)
