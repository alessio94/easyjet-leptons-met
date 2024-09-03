from argparse import ArgumentParser, Namespace
import pathlib

from AthenaConfiguration.AthConfigFlags import AthConfigFlags


def add_standard_athena_args(parser: ArgumentParser) -> None:
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


def fill_from_args(flags: AthConfigFlags, parser: ArgumentParser) -> Namespace:
    """
    Copied (and simplified) from athena's over-featured version
    """

    args, leftover = parser.parse_known_args()

    # Track the flags handled by custom CLI arguments
    # so we can check for unintentional overrides by
    # explicit flag settings
    flags_from_args = dict(
        debug='Exec.DebugStage',
        evtMax='Exec.MaxEvents',
        output_xaod='Output.AODFileName',
        skipEvents='Exec.SkipEvents',
        filesInput='Input.Files',
        loglevel='Exec.OutputLevel',
    )

    # Custom flag setters
    def set_debug(flags, name, value):
        from AthenaCommon.Debugging import DbgStage
        if value not in DbgStage.allowed_values:
            raise ValueError(
                "Unknown debug stage, allowed values {}".format(DbgStage.allowed_values)
            )
        flags[name] = value

    def set_input_files(flags, name, value):
        input_file_list = []
        for ffile in value.split(","):
            if "*" in ffile:  # handle wildcard
                import glob
                if glob.glob(ffile) != []:
                    input_file_list += glob.glob(ffile)
                else:
                    raise ValueError("Unknown input files " + ffile)
            else:
                if ffile.startswith("root://") or pathlib.Path(ffile).is_file():
                    input_file_list += [ffile]
                else:
                    raise ValueError("Unknown input file " + ffile)
        flags[name] = input_file_list

    def set_log_level(flags, name, value):
        from AthenaCommon import Constants

        if hasattr(Constants, value):
            flags[name] = getattr(Constants, value)
        else:
            raise ValueError(
                "Unknown log-level, allowed values are"
                " ALL, VERBOSE, DEBUG,INFO, WARNING, ERROR, FATAL"
            )

    flag_setters = dict(
        debug=set_debug,
        filesInput=set_input_files,
        loglevel=set_log_level,
    )

    # Set flags with dedicated CLI arguments
    for arg_name, flag_name in flags_from_args.items():
        arg_val = getattr(args, arg_name)
        if arg_val is not None:
            if arg_name in flag_setters:
                flag_setters[arg_name](flags, flag_name, arg_val)
            else:
                flags[flag_name] = arg_val

    if args.config_only is not None:
        from os import environ

        environ["PICKLECAFILE"] = args.config_only

    # Interpret any leftover arguments as ConfigFlags settings
    for flag_arg in leftover:
        # Check that these are not set by custom arguments, as
        # this could lead to inconsistencies
        for k, f in flags_from_args.items():
            if flag_arg.startswith(f):
                raise RuntimeError(
                    f"Flag '{f}' should be set with '{k}', do not override directly."
                )
        try:
            flags.fillFromString(flag_arg)
        except Exception as e:
            raise RuntimeError(f'Failed to parse argument {flag_arg}') from e

    return args


def validate_args(runconfig: dict, overwrites: dict) -> None:
    # check that values belonging in runconfig exist, and vice-versa
    for key, value in overwrites.items():
        if value and key not in runconfig:
            raise ValueError(f"{key} must be set in the config file")
        elif key in runconfig and not value:
            raise ValueError(f"{key} must not exist in the config file")


# type=bool is not recommended because it sets non-empty strings to True,
# but we want to allow the user to set the flag to "False" to disable
def _bool_opt(opt: str) -> bool:
    return opt.lower() == "true" if opt.lower() in ("true", "false") else bool(opt)


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
            type=pathlib.Path,
            default=pathlib.Path("EasyjetHub/RunConfig.yaml"),
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
        self.add_argument(
            "--dump-analysis-config-flags",
            nargs='?',
            default="",
            help="Dump flags.Analysis to a yaml file.",
            const="config-flags.Analysis.yaml",
        )
        self.add_argument(
            "--dump-full-config-flags",
            nargs='?',
            default="",
            help="Dump entire flags container to a yaml file.",
            const="config-flags.full.yaml",
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
        self.add_analysis_arg(
            "-sm",
            "--suppress-metadata-json",
            action="store_true",
            help="Do not create metadata JSON file. For running batch jobs locally"
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

    # overwrite means that the option should be set to the value of the flag,
    # overwriting the value in the config file
    def add_analysis_arg(self, *pos, overwrite=False, **args):
        if overwrite and args.get("action") == "store_true":
            raise ValueError("bool options must be specified explicitly")
        argname = self.an_opts.add_argument(*pos, **args).dest
        self.overwrites[argname] = overwrite
