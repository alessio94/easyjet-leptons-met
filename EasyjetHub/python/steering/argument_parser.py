from argparse import ArgumentParser
import pathlib

from EasyjetHub.steering.utils.log_helper import log
from .config_files import run_config_arg


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

    flags.Output.AODFileName = args.output_xaod

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


def validate_args(parser, overwrites={}):
    args, _ = parser.parse_known_args()
    runconfig = args.run_config
    # check that values belonging in runcofig exist, and vice-versa
    for key, value in overwrites.items():
        if value and key not in runconfig:
            raise ValueError(f"{key} must be set in the config file")
        elif key in runconfig and not value:
            raise ValueError(f"{key} must not exist in the config file")


def fill_flags_from_runconfig(args, flags, overwrites={}):
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


# type=bool is not recommended because it sets non-empty strings to True,
# but we want to allow the user to set the flag to "False" to disable
def _bool_opt(opt):
    return opt.lower() == "true" if opt.lower() in ("true", "false") else opt


class AnalysisArgumentParser(ArgumentParser):

    oropt = dict(
        type=_bool_opt,
        choices=[True, False],
        metavar="BOOL",
        overwrite=True,
    )

    def __init__(self):
        super().__init__()
        # Generate a parser and add an output file argument, then retrieve the args
        add_standard_athena_args(self)

        self.add_argument(
            "-c",
            "--run-config",
            metavar="CFG",
            type=run_config_arg,
            default="EasyjetHub/RunConfig.yaml",
            help="Run config file path, default: %(default)s",
        )
        self.add_argument(
            "-t",
            "--timeout",
            type=float,
            help="Maximum runtime (in seconds). Longer processes finish with an error.",
        )
        self.add_argument(
            "--dry-run",
            action="store_true",
            help="Print the configuration and exit.",
        )
        self.add_argument(
            "-p",
            "--output-xaod",
            nargs='?',
            help="name for output xAOD file",
            default="",
            const="output.pool.root",
        )

        # add analysis-specific flags
        self.an_opts = self.add_argument_group(
            "analysis", "These flags are added to `ConfigFlags.Analysis`."
        )

        # slightly ugly, keep track of the options we can write to here
        self.overwrites = {}

        self.add_analysis_arg(
            "-o",
            "--out-file",
            nargs="?",
            default=False,
            const="analysis-variables.root",
            help="Output ROOT file name",
        )
        self.add_analysis_arg(
            "-O",
            "--dump-output-branchlist",
            action="store_true",
            help=(
                "enable output branch list dump."
                "Will be written to output-branches-[out-file].txt"
            )
        )
        self.add_analysis_arg(
            "-m",
            "--cache-metadata",
            action="store_true",
            help="use metadata cache file",
        )
        self.add_analysis_arg(
            "--fast-test",
            action="store_true",
            help="disables certain sequences to speed up testing",
        )
        self.add_analysis_arg(
            "-5",
            "--h5-output",
            metavar="H5OUT",
            type=pathlib.Path,
            default=False,
            help="save output HDF5 file",
        )
        self.add_analysis_arg(
            "-z",
            "--n-h5-jets",
            type=int,
            default=6,
            help="number of jets in array, or 0 for awkward array",
        )
        self.add_analysis_arg(
            "-x",
            "--systematics-regex",
            type=str,
            nargs='+',
            help=(
                "List of regexes for matching systematic variations,"
                " each value autocompleted to (^[value].*)"
            ),
            overwrite=True,
        )

        # Overwrite options

        self.add_analysis_arg(
            "-a",
            "--disable-calib",
            help=(
                "disable CP Algs for calibration "
                "(can be set to True for plain PHYSLITE processing)"
            ),
            **AnalysisArgumentParser.oropt,
        )
        self.add_analysis_arg(
            "-f",
            "--do-trigger-filtering",
            help=(
                "enable trigger filtering"
                " (set flag to False to get all events to pass)."
                " Has no effect on data."
            ),
            **AnalysisArgumentParser.oropt,
        )
        self.add_analysis_arg(
            "-j", "--loose-jet-cleaning",
            **AnalysisArgumentParser.oropt
        )
        self.add_analysis_arg(
            "-y", "--do-CP-systematics",
            **AnalysisArgumentParser.oropt
        )
        validate_args(self, self.overwrites)

    # overwrite means that the option should be set to the value of the flag,
    # overwriting the value in the config file
    def add_analysis_arg(self, *pos, overwrite=False, **args):
        if overwrite and args.get("action") == "store_true":
            raise ValueError("bool options must be specified explicitly")
        argname = self.an_opts.add_argument(*pos, **args).dest
        self.overwrites[argname] = overwrite
