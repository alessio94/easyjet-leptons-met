from argparse import ArgumentTypeError
import pathlib
import yaml
import os
from HH4bAnalysis.utils.log_helper import log


def add_standard_athena_args(parser):
    """Custom version of the arguments flags

    The names of the arguments are stolen from Athena, see here:

    https://gitlab.cern.ch/atlas/athena/-/blob/release/22.2.110/Control/AthenaConfiguration/python/AthConfigFlags.py#L434

    Since this was introduced we also stole the parser function that
    fills the configuration flags from the arguments, see below.

    """
    parser.add_argument(
        "filesInput",
        metavar="input_files",
        help="Input file(s) as a comma separated list, supports * wildcard",
    )
    parser.add_argument(
        "-d",
        "--debug",
        nargs="?",
        const="exec",
        choices=["init", "exec", "fini"],
        metavar="STAGE",
        default=None,
        help="attach debugger at stage, default to '%(const)s'",
    )
    parser.add_argument(
        "-e", "--evtMax", type=int, default=None, help="Max number of events to process"
    )
    parser.add_argument(
        "-k",
        "--skipEvents",
        metavar="N",
        type=int,
        default=None,
        help="Number of events to skip",
    )
    parser.add_argument(
        "-l",
        "--loglevel",
        nargs="?",
        default="WARNING",
        const="INFO",
        choices=["ALL", "VERBOSE", "DEBUG", "INFO", "WARNING", "ERROR", "FATAL"],
        metavar="LEVEL",
        help=("logging level"),
    )
    # The --config-only option is interesting, it takes an argument
    # which is the name of a pickle file, which it pickles the
    # configuration to. But since this isn't readable in CA code (so
    # far as I can tell) it's not terribly useful.
    #
    # So instead we send this file to /dev/null, and abuse the option
    # to make the script quit after running the configuration step.
    parser.add_argument(
        "-s",
        "--stop-after-config",
        action="store_const",
        default=None,
        const="/dev/null",
        dest="config_only",
        help="Stop after configuration phase",
    )


def fill_from_args(flags, parser=None):
    """
    Copied (and simplified) from athena's over-featured version
    """

    args = parser.parse_args()

    if args.debug is not None:
        from AthenaCommon.Debugging import DbgStage

        if args.debug not in DbgStage.allowed_values:
            raise ValueError(
                "Unknown debug stage, allowed values {}".format(DbgStage.allowed_values)
            )
        flags.Exec.DebugStage = args.debug

    if args.evtMax is not None:
        flags.Exec.MaxEvents = args.evtMax

    if args.skipEvents is not None:
        flags.Exec.SkipEvents = args.skipEvents

    flags.Input.Files = []  # remove generic
    for ffile in args.filesInput.split(","):
        if "*" in ffile:  # handle wildcard
            import glob

            flags.Input.Files += glob.glob(ffile)
        else:
            flags.Input.Files += [ffile]

    if args.loglevel is not None:
        from AthenaCommon import Constants

        if hasattr(Constants, args.loglevel):
            flags.Exec.OutputLevel = getattr(Constants, args.loglevel)
        else:
            raise ValueError(
                "Unknown log-level, allowed values are"
                " ALL, VERBOSE, DEBUG,INFO, WARNING, ERROR, FATAL"
            )

    if args.config_only is not None:
        from os import environ

        environ["PICKLECAFILE"] = args.config_only

    return args


def run_config_arg(rawpath):
    fpath = pathlib.Path(rawpath)
    for dirpath in [""] + os.environ["DATAPATH"].split(":"):
        fullpath = dirpath / fpath
        if fullpath.exists():
            try:
                with open(fullpath) as cfgfile:
                    return yaml.safe_load(cfgfile)
            except Exception:
                raise ArgumentTypeError(f"Couldn't load run config: {fullpath}")

    raise ArgumentTypeError(f"Couldn't find config: {fpath}")


def validate_args(parser, overwrites={}):
    args, _ = parser.parse_known_args()
    runconfig = args.run_config
    # check that values belonging in runcofig exist, and vice-versa
    for key, value in overwrites.items():
        if value and key not in runconfig:
            raise ValueError(f"{key} must be set in the config file")
        elif key in runconfig and not value:
            raise ValueError(f"{key} must not exist in the config file")


def fill_config_flags_from_args(args, flags, overwrites={}):
    # Collect all the run config values from config file and flags
    run_config_all = args.run_config

    # args contain the flags, overwrite run_config file values with values from flags
    # and add flags that are not in run_config file
    for key in vars(args):
        # exclude standard athena flags
        if key in overwrites:
            value = getattr(args, key)
            if value is not None or key not in run_config_all:
                run_config_all[key] = value

    # add them to athena's ConfigFlags
    for key, value in run_config_all.items():
        log.info("User configured: " + str(key) + ": " + str(value))
        flags.addFlag("Analysis." + key, value)

    return flags
