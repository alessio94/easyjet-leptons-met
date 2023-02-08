#!/bin/env python

import argparse
import datetime
import os
import subprocess


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--mc-list", help="Text file containing MC datasets")
    parser.add_argument("--data-list", help="Text file containing data datasets")
    parser.add_argument(
        "--runConfig",
        type=str,
        required=True,
        help="file path to the runConfig",
    )
    parser.add_argument(
        "--campaign",
        default="HH4b.%Y_%m_%d",
        help=(
            "The name of the campaign. Will be prepended to the output dataset"
            " name. Can include datetime.strftime replacement fields"
        ),
    )
    parser.add_argument(
        "--asetup",
        help="The asetup command, in case it cannot be read from the environment",
    )
    # parser.add_argument("--dest-se", default="DESY-ZN_LOCALGROUPDISK",
    # help="An RSE to duplicate the results to")
    parser.add_argument("--excluded-site", help="Any sites to exclude when running")
    parser.add_argument(
        "--nGBperJob", default="10", help="How many GB required per job"
    )  # noqa
    # parser.add_argument("--resubmit", action="store_true",
    # help="Run in 'resubmission' mode. This reads in information
    # from an existing task and resubmits the dead tasks.
    # For this mode you don't need the mc and data lists")

    args = parser.parse_args()

    # Prepare some of the prun options here
    submit_opts = {
        "mergeOutput": True,
        "outputs": "TREE:output-hh4b.root",
        "writeInputToTxt": "IN:in.txt",
        "useAthenaPackages": True,
        # "noSubmit": True,  # only creates tarball but doesn't submit
    }

    # if args.dest-se is not None:
    # submit_opts["destSE"] = args.dest-se
    # if args.excluded-site is not None:
    # submit_opts["excludedSite"] = args.excluded-Site
    # if args.nGBperJob is not None:
    # submit_opts["nGBPerJob"] = args.nGBperJob

    if args.asetup is None:
        atlas_project = os.environ["AtlasProject"]

        project_dir = os.environ["{0}_DIR".format(atlas_project)]
        project_version = os.environ["{0}_VERSION".format(atlas_project)]

        if project_dir.startswith("/cvmfs/atlas-nightlies.cern.ch"):
            raise ValueError(
                "Cannot deduce the asetup command for a nightly!"
                + " Use the --asetup option"
            )
        submit_opts["athenaTag"] = ",".join([atlas_project, project_version])
    else:
        submit_opts["athenaTag"] = args.asetup

    nickname = getGridNickname()
    if not nickname:
        raise OSError(
            1,
            "Could not find proxy information."
            + ' Try typing "voms-proxy-init -voms atlas" and try again',
        )

    # copy the runconfig to the submit dir to be available for the exec cmd
    subprocess.run(["cp", args.runConfig, "config.yaml"])

    proc = subprocess.Popen(
        [
            "prun",
            "--outDS",
            "user.{0}.NONE".format(nickname),
            "--exec",
            "NONE",
            "--noSubmit",
            "--useAthenaPackages",
            "--outTarBall",
            "code.tar.gz",
        ]
    )

    if proc.wait() != 0:
        raise OSError(proc.returncode, "Failed to create tarball")

    submit_opts["inTarBall"] = "code.tar.gz"
    # submit_opts["allowTaskDuplication"] = True

    campaign = datetime.datetime.now().strftime(args.campaign)

    io_list = []
    if args.mc_list is not None:
        with open(args.mc_list) as fp:
            mc_list = fp.readlines()
        for line in mc_list:
            line = line.strip()
            if line.startswith("*"):
                continue
            if not line:
                continue
            ds_name = line.rpartition(":")[2]
            project, dsid, physics_short, prod_step, dtype, tags = ds_name.split(".")
            io_list.append(
                {
                    "inDS": ds_name,
                    "outDS": "user.{0}.{1}.{2}.{3}.{4}".format(
                        nickname, campaign, dsid, physics_short, tags
                    ),
                }
            )

    if args.data_list is not None:
        with open(args.data_list) as fp:
            data_list = fp.readlines()
        for line in data_list:
            line = line.strip()
            if line.startswith("*"):
                continue
            if not line:
                continue
            ds_name = line.rpartition(":")[2]
            split = ds_name.split(".")
            io_list.append(
                {
                    "inDS": ds_name,
                    "outDS": "user.{0}.{1}.{2}.{3}".format(
                        nickname, campaign, split[0], split[1]
                    ),
                }
            )

    if not io_list:
        raise ValueError("No inputs defined")

    for io in io_list:
        exec_cmd = (
            "VariableDumperConfig.py --filesInput %IN --outFile {0} --runConfig"
            " config.yaml".format(submit_opts["outputs"].removeprefix("TREE:"))
        )
        if args.mc_list:
            exec_cmd = (
                "VariableDumperConfig.py --filesInput %IN --outFile {0} --runConfig"
                " config.yaml".format(submit_opts["outputs"].removeprefix("TREE:"))
            )
        cmd = ["prun", "--inDS", io["inDS"], "--outDS", io["outDS"], "--exec", exec_cmd]

        for k, v in submit_opts.items():
            if isinstance(v, bool):
                if v:
                    cmd += ["--{0}".format(k)]
            else:
                if isinstance(v, (list, tuple)):
                    v = ",".join(map(str, v))

                cmd += ["--{0}".format(k), v]

        print(" ".join(cmd))
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = proc.communicate()
        print(out.decode("utf-8"), err.decode("utf-8"))
        if proc.returncode != 0:
            raise OSError(proc.returncode, err)


def getGridNickname():
    try:
        voms_proxy_info = subprocess.check_output(["voms-proxy-info", "--all"])
        voms_proxy_info = voms_proxy_info.decode("utf-8")
        nickname_index = voms_proxy_info.find("nickname")
        nickname_parts = voms_proxy_info[nickname_index:].split()
        nickname = nickname_parts[2]
    except Exception:
        nickname = ""
    return nickname


if __name__ == "__main__":
    main()
