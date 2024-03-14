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

# Argument parser
parser = ArgumentParser()
parser.add_argument("--inFile", required=True)
parser.add_argument("--outFile", help="Output file", default="SumOfWeights_test.root")
parser.add_argument("--maxEvents", help="Number of events to process",
                    default=-1, type=int)
parser.add_argument("--xSectionsConfig", required=True)
args = parser.parse_args()

flags = initConfigFlags()

# Dummy event number because we run some execute method not in xAOD inputs
flags.Exec.MaxEvents = 1

flags.lock()

acc = MainServicesCfg(flags)

SOWTool = CompFactory.SumOfWeightsTool(inFile=args.inFile)

# Get XSection Path
with open(args.xSectionsConfig, 'r') as file:
    XSectionData = yaml.safe_load(file)
# Get XSection from either custom file (which is in PMG format)
# or from an official PMG file
XSectionTool = CompFactory.GetXSectionTool(
    pathsToPMGFiles=XSectionData['XSection_paths'])

TotalWeightsTool_ttHH = CompFactory.TotalWeightsTool(
    analysis="ttHH", bTagWP="GN2v00LegacyWP_FixedCutBEff_77")

acc.addEventAlgo(CompFactory.PostProcessor(
    inFile=args.inFile,
    outFile=args.outFile,
    maxEvents=args.maxEvents,
    postProcessTools=[SOWTool,
                      XSectionTool,
                      TotalWeightsTool_ttHH]
))

# Execute and finish
sc = acc.run()

# Success should be 0
sys.exit(not sc.isSuccess())
