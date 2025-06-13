# Yassine El Ghazali
# script helps with grid submission
# to run:
# all samples: python3 submitGrid.py --tag {}  -c {config} -s all
# A specific samples: python3 submitGrid.py --tag {}  -c {config} -s "Wjets"


import os
from argparse import ArgumentParser, ArgumentDefaultsHelpFormatter


def get_args():
    parser = ArgumentParser(description="",
                            formatter_class=ArgumentDefaultsHelpFormatter)
    parser.add_argument("-s", "--samples", required=True,
                        help="Space-delimited list of samples, \"EWVVjj VH\"")
    parser.add_argument("--tag", required=True)
    parser.add_argument("-c", "--config")
    parser.add_argument("--HMBS", default=None, const=True, action='store',
                        nargs='?', help="HMBS production role")
    parser.add_argument("--nGBPerJob", default=-1, type=int)
    parser.add_argument("--memory", default=-1, type=int)

    return parser.parse_args()


def get_list_files(processes):
    mc_list = []

    for process in processes:
        file_name = ""
        if process == "data":
            file_name = "data_Run2_p6490.txt"
        else:
            file_name = f"mc20_{process}_DAOD_PHYS_p6490.txt"
        f_base_path = os.path.abspath(
            "../easyjet/vbshiggsAnalysis/datasets/PHYS/p6490/"
        )
        path_to_file = os.path.join(f_base_path, file_name)
        if os.path.exists(path_to_file):
            mc_list.append(path_to_file)
        else:
            print(f"\tfile {path_to_file} does not exist")

    return mc_list


def main(args):

    executable = "vbshiggs-ntupler"
    runConfig = "vbshiggsAnalysis/RunConfig-fullLep.yaml"

    if args.config:
        runConfig = args.config

    mc_list = []
    # all submit all signals + bkgs
    if args.samples == "all":
        processes = ["EWVVjj", "VH", "Wjets", "Zjets", "stop", "ttH", "ttV", "ttW",
                     "ttbar", "VVV", "Vgamma", "FullLep_signal", "tty"]
        mc_list = get_list_files(processes)
    else:
        mc_list = get_list_files(args.samples.split())

    for mc_file in mc_list:
        base_command = (
            f"easyjet-gridsubmit --mc-list {mc_file} "
            f"--run-config {runConfig} "
            f"--exec {executable} "
            f"--campaign {args.tag} "
            f"--noTag --mergeOutput --noEmail"
        )

        if args.HMBS:
            base_command += " --ProductionRole HMBS"

        if args.nGBPerJob != -1:
            base_command += f" --nGBperJob {args.nGBPerJob}"
        if args.memory != -1:
            base_command += f" --memory {args.memory}"

        print(f'Executing {base_command}')
        os.system(base_command)


if __name__ == "__main__":
    current_dir = os.getcwd()
    if os.path.basename(current_dir) != "run":
        raise ValueError("you need to submit from run directory")

    args = get_args()

    main(args)
