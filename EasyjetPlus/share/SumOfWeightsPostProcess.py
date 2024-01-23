#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

# Basic postProcess jobOptions, to be adapted for each analysis
from AthenaCommon.AthArgumentParser import AthArgumentParser
from AthenaCommon.AlgSequence import AlgSequence
from AthenaCommon.AppMgr import theApp

from EasyjetPlus.EasyjetPlusConf import PostProcessor, SumOfWeightsTool

# Argument parser
parser = AthArgumentParser()
parser.add_argument("--inFile", required=True)
parser.add_argument("--outFile", help="Output file", default="SumOfWeights_test.root")
parser.add_argument("--maxEvents", help="Number of events to process",
                    default=-1, type=int)
args = parser.parse_args()

# Create top sequence
topSequence = AlgSequence()

# Create an instance of your algorithm
postProcessor = PostProcessor(
    inFile=args.inFile,
    outFile=args.outFile,
    maxEvents=args.maxEvents
)

# Add postProcessing tools
postProcessor.postProcessTools = [SumOfWeightsTool(inFile=args.inFile)]

topSequence += postProcessor

# Dummy event number because we run some execute method not in xAOD inputs
theApp.EvtMax = 1
