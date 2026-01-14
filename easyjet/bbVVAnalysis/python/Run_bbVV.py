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
    "0lepX2000": "525951", "0lepX4000": "525952", "0lepX6000": "525953",
    "0lepX2000_S1000": "802062", "0lepX3000_S1500": "802064",
    "0lepX4000_S2000": "802066", "1lepX2000": "525948",
    "1lepX3000": "525949", "1lepX4000": "525950", "ttbar": "601237"}


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
        process="bbWW0lep", boosted="", mass=2000, lepton=True, systematics=False,
        sample_dir="/eos/atlas/atlascerngroupdisk/phys-higp/higgs-pairs/bbVV/"):

    outFile = "ejOutput_PHYS_bbVV_"  # Add substrings below

    lep_channel = "1lep" if lepton else "0lep"
    boosted_channel = boosted
    sys = "-sys" if systematics else ""
    configFile = "bbVVAnalysis/RunConfig-PHYS-bbVV-" + lep_channel + \
                 "-" + boosted_channel + sys + ".yaml"
    outFile += lep_channel + "_" + boosted_channel + "_"

    if process == "bbWW0lep":
        mass_key = "X" + str(mass)
        if (not lepton and not boosted):
            mass_key += "_S" + str(int(mass / 2))
        sample = get_sample_dir(
            sample_dir, sample_DSID_Dict[lep_channel + mass_key])
        outFile += mass_key + ".root"

    else:
        sample = get_sample_dir(sample_dir, sample_DSID_Dict["ttbar"])
        outFile += "ttbar.root"

    return sample, outFile, configFile


def run_local(
        process="bbWW0lep", boosted="", mass=2000, lepton=True, systematics=False,
        sample_dir="/eos/atlas/atlascerngroupdisk/phys-higp/higgs-pairs/bbVV/"):

    inputFile, outputFile, configFile = io_manager(
        process, boosted, mass, lepton, systematics, sample_dir)
    input = '"' + inputFile + '*"'
    command = ("bbVV-ntupler --loglevel " + dbg_level + " --evtMax=10000 "
               + input + " --run-config " + configFile + " --out-file " + outputFile)

    os.chdir(easyjet_run_dir)  # Move to the easyjet/run directory

    print(command)
    subprocess.run(command, shell=True)
    print(
        "End of Run_bbVV.py, check your output\nOutFile:",
        os.path.realpath(outputFile))


def run_grid(
        process="top", boosted="", lepton=True, campaign="mc23a",
        tag="NONE", systematics=False, nFile=3):

    executable = "bbVV-ntupler"
    ListDir = easyjet_build_dir + "/data/bbVVAnalysis/PHYS/nominal/" + campaign + "/"
    if process == "data":
        List = ListDir + "data_*.txt"
    else:
        List = ListDir + campaign + "_" + process + "_p6697.txt"

    lep_channel = "1lep" if lepton else "0lep"
    boosted_channel = boosted
    sys = "-sys" if systematics else ""
    nfile = f" --maxNFilesPerJob {nFile}" if systematics else ""
    configFile = "bbVVAnalysis/RunConfig-PHYS-bbVV-" + lep_channel + \
                 "-" + boosted_channel + sys + ".yaml"

    if tag != "NONE":
        ej_gittag = tag
    else:
        ej_gittag = campaign

    if process == "data":
        command = ("easyjet-gridsubmit --data-list " + List
                   + " --run-config " + configFile + " --exec " + executable
                   + " --campaign " + ej_gittag + nfile)
    else:
        command = ("easyjet-gridsubmit --mc-list "
                   + List + " --run-config " + configFile + " --exec " + executable
                   + " --campaign " + ej_gittag + nfile)
    print(command)
    subprocess.run(command, shell=True)
    print("End of Run_bbVV.py, check your BigPanda!")


if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--Process",
        help="Define which process: bbWW0lep / "
        "dijet, singletop, ttbar, diboson, Wjets, Zjets / "
        "data",
        default="bbWW0lep")
    parser.add_argument(
        "--Boost", help="Option: boosted, splitboosted, common (0Lep only).",
        default="boosted")
    parser.add_argument(
        "--Lepton", help="Run bbVV 1lep channel", action='store_true')
    parser.add_argument(
        "--Mass",
        help="1lep Boosted: 2000, 3000, 4000 / "
        "0lep Boosted: 2000, 4000, 6000 / "
        "0lep SplitBoosted: 2000, 3000, 4000",
        type=int, default=2000)
    parser.add_argument("--GridRun", help="Run on Grid", action='store_true')
    parser.add_argument(
        "--SampleDir", help="Type your PHYS directory",
        default="/eos/atlas/atlascerngroupdisk/phys-higp/higgs-pairs/bbVV/")
    parser.add_argument(
        "--campaign", help="mc23a, mc23d, mc23e", default="mc23a")
    parser.add_argument(
        "--tag", help="Custom tag pattern", default="NONE")
    parser.add_argument(
        "--sys", help="Run systematics", action='store_true')
    parser.add_argument(
        "--nFile", help="Set maxNFilesPerJob for systematics production",
        type=int, default=3)
    args = parser.parse_args()
    process = args.Process
    boost = args.Boost
    grid = args.GridRun
    mass = args.Mass
    lepton = args.Lepton
    sample_dir = args.SampleDir
    campaign = args.campaign
    tag = args.tag
    systematics = args.sys
    nFile = args.nFile
    print(
        "Process:", process, "\tLepton:", lepton, "\tBoost:", boost,
        "\tMass: ", mass, "\tGrid:", grid, "\tSampleDir:", sample_dir,
        "\tCampaign:", campaign, "\tTag:", tag, "\tSys:", systematics,
        "\tNFile:", nFile)

    # Need to synchronize with io_manager / run_grid
    if grid:
        print("Running on Grid.")
        run_grid(process, boost, lepton, campaign, tag, systematics, nFile)
    else:
        print("Running locally.")
        run_local(process, boost, mass, lepton, systematics, sample_dir)
