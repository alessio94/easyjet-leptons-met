from pathlib import Path
import sys

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.AutoConfigFlags import GetFileMD
from AthenaCommon.Constants import INFO

from EasyjetHub.steering.sample_metadata import (
    get_data_type,
    get_run_years,
    get_pileup_config_files,
    update_metadata,
    has_metadata,
    cache_metadata,
)
from EasyjetHub.steering.argument_parser import (
    AnalysisArgumentParser,
    fill_from_args,
    fill_flags_from_runconfig,
)
from EasyjetHub.steering.utils.option_validation import validate_flags
from EasyjetHub.steering.utils.log_helper import log, setRogueLoggers
from EasyjetHub.steering.container_names import define_output_container_name_flags


def analysis_configuration(parser="default"):

    if parser == "default":
        parser = AnalysisArgumentParser()

    # Initialise the job configuration flags, some of which will be autoconfigured.
    # These are used for steering the job, and include e.g. the input file (list).
    flags = initConfigFlags()

    args = fill_from_args(flags, parser=parser)
    log.setLevel(flags.Exec.OutputLevel)
    if args.config_only and has_metadata(flags):
        log.info("found metadata for input files, skipping config run")
        sys.exit(0)
    setRogueLoggers(flags.Exec.OutputLevel)
    # Write user options to flags.Analysis
    fill_flags_from_runconfig(args, flags, parser.overwrites)

    # Determined from input file without explicitly reading metadata
    def is_physlite(flags):
        return flags.Input.ProcessingTags == ["StreamDAOD_PHYSLITE"]

    flags.addFlag("Input.isPHYSLITE", lambda prevFlags: is_physlite(prevFlags))

    # disable some sequences for fast-run
    if flags.Analysis.fast_test:
        flags.Analysis.do_overlap_removal = False
        flags.Analysis.do_muons = False
        flags.Analysis.write_muons = False

    log.info(f"Operating on input files {flags.Input.Files}")

    fileMD = GetFileMD(flags.Input.Files)
    if flags.Analysis.cache_metadata:
        update_metadata(Path("metadata.json"))
    flags.addFlag("Input.AMITag", fileMD.get("AMITag", ""))
    flags.addFlag("Input.SimulationFlavour",
                  fileMD.get("SimulationFlavour", ""))
    flags.addFlag("Analysis.DataType", get_data_type)
    flags.addFlag("Analysis.Years", get_run_years)
    log.info(f"Configuring to match dataset from {flags.Analysis.Years}")
    if max(flags.Analysis.Years) <= 2018:
        flags.addFlag("Analysis.Run", 2)
    elif min(flags.Analysis.Years) >= 2022:
        flags.addFlag("Analysis.Run", 3)
    else:
        raise RuntimeError("Invalid list of years, cannot combine runs")
    log.info(f"Configured years match Run {flags.Analysis.Run}")

    flags.PerfMon.doFullMonMT = flags.Exec.OutputLevel <= INFO

    flags.addFlag(
        "Analysis.TriggerChains",
        lambda prevFlags: get_trigger_chains(prevFlags)
    )

    def is_mc_phys(flags):
        return flags.Input.isMC and not flags.Input.isPHYSLITE
    do_PRW = is_mc_phys(flags)
    prw_files, lumicalc_files = [], []
    if do_PRW:
        try:
            prw_files, lumicalc_files = get_pileup_config_files(flags)
        except LookupError as err:
            log.error(err)
            do_PRW = False
    flags.addFlag("Analysis.doPRW", do_PRW)
    flags.addFlag("Analysis.PRWFiles", prw_files)
    flags.addFlag("Analysis.LumiCalcFiles", lumicalc_files)

    flags.addFlagsCategory(
        "Analysis.container_names.output",
        lambda flags=flags: define_output_container_name_flags(flags),
        prefix=True,
    )

    validate_flags(flags)

    log.info(
        f"Self-configured: datatype: '{flags.Analysis.DataType}', "
        f"is PHYSLITE? {flags.Input.isPHYSLITE}"
    )

    if flags.Analysis.cache_metadata:
        cache_metadata(Path("metadata.json"))

    return flags, args


def get_trigger_chains(flags):

    from EasyjetHub.steering.trigger_lists import TRIGGER_LISTS

    trigger_year_list = flags.Analysis.trigger_year
    if trigger_year_list == "Auto":
        trigger_year_list = flags.Analysis.Years
        log.info(
            "Self-configured trigger list for years: "
            f"{', '.join(str(year) for year in trigger_year_list) or None}"
        )

    trigger_chains = set()
    # Empty: set the bbbb analysis triggers
    trigger_groups = flags.Analysis.trigger_list
    if trigger_groups == "Auto":
        log.info("No triggers specified, adding bbbb analysis triggers")
        trigger_groups = ["bbbbResolved", "bbbbBoosted"]
    try:
        for trigger_group in trigger_groups:
            for year in trigger_year_list:
                trigger_chains |= set(TRIGGER_LISTS[trigger_group][year])
    except KeyError as err:
        log.error(f"Trigger list for {trigger_group}, {year} not defined.")
        raise err

    return list(trigger_chains)
