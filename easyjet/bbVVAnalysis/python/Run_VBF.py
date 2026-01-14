#!/usr/bin/python3

import subprocess
import os
import sys
import argparse

dbg_level = "INFO"

easyjet_build_dir = os.environ['easyjet_DIR']
easyjet_run_dir = easyjet_build_dir + "/../../run/"

# 1lep SplitBoosted under development
sample_DSID_Dict = {
    "cvv0": "546872", "cvv0p5": "546873", "cvv1": "546874",
    "cvv1p5": "546875", "cvv2": "546876", "cvv3": "546877"}


def get_sample_dir(path, dsid_str):
    tmp = "999999"
    with os.scandir(path) as entries:  # Get a list of file/subdir in path as entries

        for entry in entries:
            # Assume the subdir's name has substr
            if (entry.is_dir()) and (dsid_str in entry.name):
                tmp = os.path.realpath(entry) + '/'

    if tmp == "999999":
        sys.exit("!!!!!SAMPLE NOT FOUND IN SAMPLEDIR!!!!!")

    return tmp


def io_manager(
        process="Signal", Sample_dir="", k_vv=""):

    outFile = "ejOutput_PHYS_bbVV_"

    configFile = "bbVVAnalysis/RunConfig-PHYS-bbVV-1lep-boosted-VBF.yaml"
    outFile += "1lep_boosted_"

    if process == "signal":

        sample = get_sample_dir(
            Sample_dir, sample_DSID_Dict[k_vv])
        outFile += k_vv + "_VBF.root"

    return sample, outFile, configFile


def run_local(
        process="vbfHHbbWW1lep", sample_dir="",):
    k_vv = ["cvv0", "cvv0p5", "cvv1", "cvv1p5", "cvv2", "cvv3"]
    for entry in k_vv:
        inputFile, outputFile, configFile = io_manager(
            process, sample_dir, entry)
        input = '"' + inputFile + '*"'
        command = ("bbVV-ntupler " + input + " --run-config " + configFile
                   + " --out-file " + outputFile)

        os.chdir(easyjet_run_dir)  # Move to the easyjet/run directory

        print(command)
        subprocess.run(command, shell=True)
        print(
            "End of Run_bbVV.py, check your output\nOutFile:",
            os.path.realpath(outputFile))


def run_grid(process="vbfHHbbWW1lep", campaign="mc20a"):
    executable = "bbVV-ntupler"
    ListDir = easyjet_build_dir + "/data/bbVVAnalysis/PHYS/nominal/" + campaign + "/"

    List = ListDir + campaign + "_" + process + "_p*.txt"

    configFile = "bbVVAnalysis/RunConfig-bbVV-bypass.yaml"

    command = ("easyjet-gridsubmit --mc-list "
               + List + " --run-config " + configFile + " --exec " + executable
               + " --nGBperJob 10 " + " --campaign " + campaign + "_%Y_%m_%d")
    print(command)
    subprocess.run(command, shell=True)
    print("End of Run_bbVV.py, check your BigPanda!")


if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--Process",
        help="""Define which process: vbfHHbbWW1lep, vbfHHbbWW1lepFASTSIM, /
        "diboson, dijet, singleTop, singleHiggs, ttbar, Wjets, Zjets""",
        default="vbfHHbbWW1lep")
    parser.add_argument("--GridRun", help="Run on Grid", action='store_true')
    parser.add_argument(
        "--SampleDir", help="Type your PHYS directory",
        default="/eos/atlas/atlascerngroupdisk/phys-higp/higgs-pairs/bbVV/")
    parser.add_argument(
        "--campaign", help="mc20a, mc20d, mc20e, mc23a, mc23d", default="mc20a")
    args = parser.parse_args()
    process = args.Process
    grid = args.GridRun
    sample_dir = args.SampleDir
    campaign = args.campaign
    print(
        "Process:", process,
        "\tGrid:", grid, "\tSampleDir:", sample_dir, "\tCampaign", campaign)

    # Need to synchronize with io_manager / run_grid
    if grid:
        print("Running on Grid.")
        run_grid(process, campaign)
    else:
        print("Running locally.")
        run_local(process, sample_dir)
