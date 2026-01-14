#!/usr/bin/env python

#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

# Basic postProcess jobOptions, to be adapted for each analysis

import sys

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
args = parser.parse_args()


flags = initConfigFlags()

# Dummy event number because we run some execute method not in xAOD inputs
flags.Exec.MaxEvents = 1

flags.lock()

acc = MainServicesCfg(flags)

acc.addEventAlgo(CompFactory.PostProcessor(
    inFile=args.inFile,
    outFile=args.outFile,
    maxEvents=args.maxEvents,
    postProcessTools=[CompFactory.SumOfWeightsTool(inFile=args.inFile)]
))

# Execute and finish
sc = acc.run()

# Success should be 0
sys.exit(not sc.isSuccess())
