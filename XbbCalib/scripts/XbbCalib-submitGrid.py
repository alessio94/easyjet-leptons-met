#!/usr/bin/env python3
# Yassine El Ghazali
# Modified for XbbClaib use by Iza Veliscek and Nikita Pond
# script helps with grid submission
# to run:
# all samples: XbbCalib-submitGrid.py --tag {}  --analysis Zbbj --ptag p7018 \
# --mc mc20 mc23  --data run2 run3
# A specific samples:
# python3 XbbCalib-submitGrid.py --tag {}  -s "Wjets --samplePath Zbby" --ptag p7018 \
#  --mc mc20 mc23
# By default will use:
# All samples for a specific analysis, as defined in MC_PROCESSES_BY_ANALYSIS
# Default config for a specific analysis, as defined in CONFIG_BY_ANALYSIS
# Any additional arguments not covered here will be passed to easyjet-gridsubmit,
# e.g. --nFilesPerJob, --maxFiles, --mergeOutput, --noSubmit, etc.

import os
import tempfile
from argparse import ArgumentParser, ArgumentDefaultsHelpFormatter
from pathlib import Path

MC_PROCESSES_BY_ANALYSIS = {
    "Zbbj": [
        "dijets",
        # "dijet_bfilt",
        "Zbb_ptZ_200_ECMS",
        "Zqq_ptZ_200_ECMS",
        "Zqq_ptZ_200_ECMS",
        "Wqq_ptW_200_ECMS",
        "ttbar_allhad"
    ],
    "Zbby": [
        "dijets",
        # "dijet_bfilt",
        "Zbb_ptZ_200_ECMS",
        "Zbbgamma_pTZ100",
        "Zqq_ptZ_200_ECMS",
        "Zqqgamma_pTZ100",
        "SinglePhoton",
        "Vgamma_Vgammagamma",
        "Wqq_ptW_200_ECMS",
        "Wqqgamma_pTW140",
        "ttbar_allhad",
        "tty"
    ],
    "Zll": [
        "Diboson",
        "Zll"
    ],
    "tt": [
        "Diboson",
        "Wjet",
        "singletop",
        "ttbar_nonallhad",
    ],
}

# Define the default config to use for a given analysis
CONFIG_BY_ANALYSIS = {
    "Zbbj": "../easyjet/XbbCalib/share/RunConfig_ZbbjCalib.yaml",
    "Zbby": "../easyjet/XbbCalib/share/RunConfig-ZbbyCalib.yaml",
    "Zll": "../easyjet/XbbCalib/share/RunConfig-Zll.yaml",
    "tt": "../easyjet/XbbCalib/share/RunConfig-ttCalib.yaml"
}

SAMPLES_BASE_PATH = Path(
    "../easyjet/XbbCalib/datasets/"
).resolve()


def get_args():
    parser = ArgumentParser(description="",
                            formatter_class=ArgumentDefaultsHelpFormatter)
    parser.add_argument(
        "--tag",
        required=True,
        help="A tag to identify the submission - will be included in the output names"
    )
    parser.add_argument(
        "--ptag",
        type=str,
        required=True,
        help="Production tag to use, e.g. p7018, p6490"
    )
    parser.add_argument(
        "--data",
        nargs="+",
        choices=["run2", "run3"],
        help="Space seperated list of data periods to process"
    )
    parser.add_argument(
        "--mc",
        nargs="+",
        choices=["mc20", "mc23"],
        help="Space seperated list of mc campaigns to process"
    )
    parser.add_argument(
        "-c",
        "--config",
        help="Use this config instead of the default one")
    parser.add_argument(
        "-s",
        "--samples",
        nargs="+",
        required=False,
        help="Space-delimited list of samples, \"MG_Zqqgamma SinglePhoton\" If "
        "none are given, all samples for a given analysis are run",)
    parser.add_argument(
        "--analysis",
        type=str,
        required=True,
        choices=["Zbby", "Zbbj", "Zll", "tt"],
        help="Which analysis to run - defines the default config and samples"
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="If set, will only print the commands that would be run, without executing"
    )
    return parser.parse_known_args()


def make_temp_file_with_containers(mc_list):
    '''Creates a temporary file containing all of the unique containers found by
    reading all the text files in mc_list'''
    containers = set()
    for mc_file in mc_list:
        with open(mc_file, 'r') as f:
            for line in f:
                line = line.strip()
                if line and not line.startswith("#"):
                    containers.add(line)
    containers = list(containers)
    with tempfile.NamedTemporaryFile(mode='w+', delete=False) as temp_file:
        for container in containers:
            temp_file.write(f"{container}\n")

    return temp_file.name


def get_all_mc_samples(
    analysis,
    campaign,
    ptag,
    samples=None,
):
    '''
    Get all the relevent samples.txt files for a given analysis, mc campaign, and ptag

    Parameters
    ----------
    analysis : str
        Analysis name, e.g. Zbbj, Zbby
    campaign : str
        MC campaign, e.g. mc20, mc23
    ptag : str
        Production tag, e.g. p7018, p6490, etc
    '''
    assert analysis in MC_PROCESSES_BY_ANALYSIS, \
        f"Analysis {analysis} not found in MC_PROCESSES_BY_ANALYSIS"
    assert campaign in ["mc20", "mc23"], f"Campaign {campaign} not supported"
    if samples is None:
        samples = MC_PROCESSES_BY_ANALYSIS[analysis]
    mc_list = []
    missing_files = []

    for process in samples:
        sample_file = SAMPLES_BASE_PATH / process / ptag / f"{campaign}.txt"
        if sample_file.exists():
            mc_list.append(str(sample_file))
        else:
            missing_files.append(str(sample_file))

    if missing_files:
        raise ValueError(
            f"For analysis {analysis}, campaign {campaign}, ptag {ptag}, the following "
            f"sample files are missing: {missing_files}"
        )
    return mc_list


def submit(command, dry_run):
    command_str = " ".join(command)
    if dry_run:
        print(f"Dry run: {command_str}")
    else:
        os.system(command_str)


def main(args, ej_grid_submit_args):

    executable = "xbbcalib-ntupler"
    analysis = args.analysis
    runConfig = args.config or CONFIG_BY_ANALYSIS[analysis]
    if not args.mc and not args.data:
        raise ValueError("At least one of --mc or --data must be provided")

    # All the defaults
    ej_command = [
        "easyjet-gridsubmit",
        "--run-config", runConfig,
        "--exec", executable,
        "--campaign", args.tag,
        "--mergeOutput",
    ] + ej_grid_submit_args

    # Run all mc campaigns
    for campaign in args.mc or []:
        mc_list = get_all_mc_samples(
            analysis,
            campaign,
            args.ptag,
            args.samples
        )

        submit(
            ej_command + ['--mc-list', make_temp_file_with_containers(mc_list)],
            args.dry_run
        )

    # Run all data periods
    for data_period in args.data or []:
        data_file = SAMPLES_BASE_PATH / "data" / args.ptag / f"{data_period}.txt"
        assert data_file.exists(), f"Data file {data_file} does not exist"
        submit(
            ej_command + ['--data-list', str(data_file)],
            args.dry_run
        )


if __name__ == "__main__":
    current_dir = os.getcwd()
    if os.path.basename(current_dir) != "run":
        raise ValueError("you need to submit from run directory")

    this_args, ej_grid_submit_args = get_args()
    main(this_args, ej_grid_submit_args)
