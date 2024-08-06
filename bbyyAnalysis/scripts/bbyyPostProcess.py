#!/usr/bin/env python

#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

# Basic postProcess jobOptions, to be adapted for each analysis

import sys
import yaml
import os
import re

from argparse import ArgumentParser
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

from bbyyAnalysis.bbyy_config import FullPath, get_sys_weight_name, contain_dalitz
from EasyjetPlus.PostProcessTools import mergeFiles

import ROOT


def get_DSID(file_path):
    file = ROOT.TFile.Open(file_path)
    if not file or file.IsZombie():
        raise IOError(f"Could not open file: {file_path}")

    metadata = file.Get("metadata")
    if not metadata:
        raise KeyError("The 'metadata' histo is not found in the file.")

    bin_label = metadata.GetXaxis().GetBinLabel(3)

    DSID = int(bin_label)
    file.Close()
    return DSID


def get_sys_histogram_name(file_path, sys_suffix):
    file = ROOT.TFile.Open(file_path)
    if not file or file.IsZombie():
        raise IOError(f"Could not open file: {file_path}")

    histogram_name = f"SumOfWeights_{sys_suffix}"
    if file.GetListOfKeys().Contains(histogram_name):
        file.Close()
        return histogram_name
    else:
        pattern = re.compile(f"^CutBookkeeper_.*_{sys_suffix}$")
        matching_histograms = [
            key.GetName() for key in file.GetListOfKeys()
            if pattern.match(key.GetName().split(';')[0])
        ]
        if matching_histograms:
            file.Close()
            return matching_histograms[0].split(';')[0]
        else:
            file.Close()
            raise RuntimeError(
                f"Neither SumOfWeights_{sys_suffix} nor"
                f"CutBookkeeper_*_{sys_suffix} found in the file."
            )


def RunEasyjetPlus(args):
    flags = initConfigFlags()

    # Dummy event number because we run some execute method not in xAOD inputs
    flags.Exec.MaxEvents = 1

    flags.lock()

    acc = MainServicesCfg(flags)

    dsid = int(get_DSID(args.inFile))
    sys_weight_prefix = get_sys_weight_name(dsid)
    if sys_weight_prefix == "":
        sys_weight_prefix = "NOSYS"

    SOWTool = CompFactory.SumOfWeightsTool(inFile=args.inFile)
    if bool(sys_weight_prefix) or contain_dalitz(dsid):
        SOWTool.inHisto = get_sys_histogram_name(args.inFile, sys_weight_prefix)

    # Get XSection Path
    with open(FullPath(args.xSectionsConfig), 'r') as file:
        XSectionData = yaml.safe_load(file)
    # Get XSection from either custom file (which is in PMG format)
    # or from an official PMG file
    XSectionTool = CompFactory.GetXSectionTool(
        pathsToPMGFiles=XSectionData['XSection_paths'])

    TotalWeightsTool_bbyy = CompFactory.TotalWeightsTool(
        analysis="bbyy", nPhotons=2, bTagWP=args.bTagWP,
        MCWeightName=f"generatorWeight_{sys_weight_prefix}")

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
        print("Post-Processing job failed.")
        sys.exit(1)


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("--inFile", required=True)
    parser.add_argument("--outFile", required=True)
    parser.add_argument("--maxEvents", default=-1, type=int)
    parser.add_argument("--xSectionsConfig", required=True)
    parser.add_argument("--copyInputs", action='store_true',
                        help="Copy pre-processed branches to outFile.")
    parser.add_argument("--mergeMyFiles", action='store_true',
                        help="Merge branches. Default is merge output to input")
    parser.add_argument("--mergeToOutput", action='store_true',
                        help="Can apply with mergeMyFiles. Merge input to output")
    parser.add_argument("--bTagWP", default="GN2v01_FixedCutBEff_77")

    args = parser.parse_args()

    outputFile = args.outFile
    if os.path.exists(outputFile):
        raise RuntimeError("Output file already exists, provide another name")

    RunEasyjetPlus(args)

    if args.mergeMyFiles:
        mergeFiles(args.inFile, args.outFile, args.mergeToOutput)
