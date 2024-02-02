#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

# Basic postProcess jobOptions, to be adapted for each analysis
from AthenaCommon.AthArgumentParser import AthArgumentParser
from AthenaCommon.AlgSequence import AlgSequence
from AthenaCommon.AppMgr import theApp

from EasyjetPlus.EasyjetPlusConf import PostProcessor, SumOfWeightsTool, GetXSectionTool

import yaml

# Argument parser
parser = AthArgumentParser()
parser.add_argument("--inFile", required=True)
parser.add_argument("--outFile", help="Output file", default="mcNormalisationVars.root")
parser.add_argument("--maxEvents", help="Number of events to process",
                    default=-1, type=int)
parser.add_argument("--xSectionsConfig", required=True)

args = parser.parse_args()

# Create top sequence
topSequence = AlgSequence()

# Create an instance of your algorithm
postProcessor = PostProcessor(
    inFile=args.inFile,
    outFile=args.outFile,
    maxEvents=args.maxEvents,
)

# Get XSection Path
with open(args.xSectionsConfig, 'r') as file:
    XSectionData = yaml.safe_load(file)

# Get XSection from either custom file (which is in PMG format)
# or from an official PMG file
getXSection = GetXSectionTool(pathsToPMGFiles=XSectionData['XSection_paths'])

# Add postProcessing tools
postProcessor.postProcessTools = [SumOfWeightsTool(inFile=args.inFile),
                                  getXSection]

topSequence += postProcessor

# Dummy event number because we run some execute method not in xAOD inputs
theApp.EvtMax = 1
