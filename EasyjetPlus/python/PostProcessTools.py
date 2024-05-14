#!/usr/bin/env python

#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

# The purpose of this module is to provide common post processing
# related functions to be used by any analysis performing post processing

import ROOT
import sys

# Merge branches among input and output files. The pro of
# this is you have a single file with your variables
# and MC weights, and therefore don't have to deal
# with two files when you analyze.


def mergeFiles(inFileName, outFileName, mergeToOutput):

    # parameters to merge to output file
    # the pro of this is you leave your input
    # file untouched.
    if (mergeToOutput):
        inFile = ROOT.TFile.Open(inFileName, "READ")
        outFile = ROOT.TFile.Open(outFileName, "UPDATE")
        targetFileName = outFileName
        t_target = outFile.Get("AnalysisMiniTree")
        t_source = inFile.Get("AnalysisMiniTree")

    # parameters to merge to input file
    else:
        inFile = ROOT.TFile.Open(inFileName, "UPDATE")
        outFile = ROOT.TFile.Open(outFileName, "READ")
        targetFileName = inFileName
        t_target = inFile.Get("AnalysisMiniTree")
        t_source = outFile.Get("AnalysisMiniTree")

    # ensure trees are readable
    if not t_target:
        print("Error: Could not retrieve the TTree from target.")
        sys.exit(1)

    if not t_source:
        print("Error: Could not retrieve the TTree from source.")
        sys.exit(1)

    # Save all branches of both files into the target file
    # via the friend method
    t_target.AddFriend(t_source, "friendTree")
    df = ROOT.RDataFrame(t_target)
    opts = ROOT.RDF.RSnapshotOptions()
    opts.fMode = "RECREATE"
    df.Snapshot("AnalysisMiniTree", targetFileName, "", opts)
