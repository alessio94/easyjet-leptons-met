# Yassine El Ghazali
# Modified for XbbClaib use by Iza Veliscek
# script helps with grid submission
# to run:
# all samples: python3 submitGrid.py --tag {}  -c {config} -s all
# A specific samples: python3 submitGrid.py --tag {}  -c {config} -s "Wjets"


import os
from argparse import ArgumentParser, ArgumentDefaultsHelpFormatter


def get_args():
    parser = ArgumentParser(description="",
                            formatter_class=ArgumentDefaultsHelpFormatter)
    parser.add_argument(
        "-s",
        "--samples",
        required=True,
        help="Space-delimited list of samples, \"MG_Zqqgamma SinglePhoton\"")
    parser.add_argument("--tag", required=True)
    parser.add_argument("-c", "--config")
    parser.add_argument("--nGBPerJob", default=-1, type=int)
    parser.add_argument("--memory", default=-1, type=int)
    parser.add_argument("--samplePath", default="Zbby")

    return parser.parse_args()


def get_list_files(processes, samplePath):
    mc_list = []

    for process in processes:
        file_name = ""
        if process == "data":
            file_name = "data/data_Run2_p6490.txt"
        else:
            file_name = f"{samplePath}/MC/mc20_{process}.txt"
        f_base_path = os.path.abspath(
            "../easyjet/XbbCalib/datasets/"
        )
        path_to_file = os.path.join(f_base_path, file_name)
        if os.path.exists(path_to_file):
            mc_list.append(path_to_file)
        else:
            print(f"\tfile {path_to_file} does not exist")

    return mc_list


def main(args):

    executable = "xbbcalib-ntupler"
    runConfig = "../easyjet/XbbCalib/share/RunConfig-ZbbyCalib.yaml "

    if args.config:
        runConfig = args.config

    mc_list = []
    if args.samples == "all":
        processes = [
            "Zbb_ptZ_200_ECMS",
            "Zbbgamma_pTZ100",
            "Zqq_ptZ_200_ECMS",
            "Zqqgamma_pTZ100",
            "SinglePhoton",
            "Vgamma_Vgammagamma",
            "Wqq_ptW_200_ECMS",
            "Wqqgamma_pTW140",
            "dijet_bfilt",
            "dijets",
            "ttbar_allhad",
            "tty"]
        mc_list = get_list_files(processes, args.samplePath)
    else:
        mc_list = get_list_files(args.samples.split(), args.samplePath)

    for mc_file in mc_list:
        base_command = (
            f"easyjet-gridsubmit --mc-list {mc_file} "
            f"--run-config {runConfig} "
            f"--exec {executable} "
            f"--campaign {args.tag} "
            f"--noTag --mergeOutput --noEmail"
        )

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
