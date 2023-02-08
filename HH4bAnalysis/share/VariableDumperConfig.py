#!/bin/env python

#
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

#
# VariableDumperConfig.py
# A simple CA file to create a tree of variables
#

import sys
from pathlib import Path
import time

from AthenaConfiguration.AutoConfigFlags import GetFileMD
from AthenaConfiguration.ComponentFactory import CompFactory

from HH4bAnalysis.Config.AnalysisAlgsConfig import AnalysisAlgsCfg
from HH4bAnalysis.Config.Base import (
    updateConfigFlags, getRunYears, pileupConfigFiles, getRunConfig
)
from HH4bAnalysis.Config.MiniTupleConfig import MiniTupleCfg
from HH4bAnalysis.Config.H5Config import getH5Cfg
from HH4bAnalysis.utils.inputsHelper import get_dataType, is_physlite
from HH4bAnalysis.utils.logHelper import log
from HH4bAnalysis.Config.Base import cache_metadata, update_metadata


def defineArgs(ConfigFlags):
    # Generate a parser and add an output file argument, then retrieve the args
    parser = ConfigFlags.getArgumentParser()
    parser.add_argument(
        '-c',
        "--runConfig",
        type=getRunConfig,
        required=True,
        help="Run config file path",
    )
    parser.add_argument(
        '--timeout',
        type=float,
        help=(
            'Maximum runtime (in seconds). Longer processes finish '
            'with an error.'
        )
    )

    # add analysis-specific flags
    an_opts = parser.add_argument_group(
        'analysis',
        'These flags are added to `ConfigFlags.Analysis`.'
    )
    # slightly ugly, keep track of the options we can write to here
    overwrites = {}

    def add_analysis_arg(*pos, overwrite=False, **args):
        if overwrite and args.get('action') == 'store_true':
            raise ValueError('bool options must be specified explicitly')
        argname = an_opts.add_argument(*pos, **args).dest
        overwrites[argname] = overwrite

    add_analysis_arg(
        '-o',
        "--outFile",
        default='analysis-variables.root',
        type=str,
        help="Output file name",
    )
    add_analysis_arg(
        "--disable-trigger-filtering",
        action="store_true",
        help=(
            "Disable trigger filtering (to get all events to pass). "
            "Has no effect on data."
        ),
    )
    add_analysis_arg(
        "-m",
        "--cache-metadata",
        action="store_true",
        help="use metadata cache file, defaults to %(const)s",
    )
    add_analysis_arg(
        "--disable-calib",
        action="store_true",
        help=(
            "disable CP Algs for calibration "
            "(can be used for plain PHYSLITE processing)"
        ),
    )
    add_analysis_arg(
        "--allow-no-ptag",
        action="store_true",
        help=(
            "disable ptag detection for CI tests " "(avoids CBK failure on test files)"
        ),
    )
    add_analysis_arg('--write-h5-event', action='store_true')

    oropt = dict(type=bool, metavar='BOOL', overwrite=True)
    add_analysis_arg('-b','--do-resolved-dihiggs-analysis', **oropt)
    add_analysis_arg('-r','--do-boosted-dihiggs-analysis', **oropt)
    add_analysis_arg('--loose-jet-cleaning', **oropt)
    return parser, overwrites


def _is_mc_phys(flags):
    return flags.Input.isMC and not is_physlite(flags)


# CA modules are intended to be executable, to facilitate easy testing.
# We define a "main function" that will run a test job if the module
# is executed rather than imported.
def main():

    # record the total run time
    starttime = time.process_time()

    # Import the job configuration flags, some of which will be autoconfigured.
    # These are used for steering the job, and include e.g. the input file (list).
    from AthenaConfiguration.AllConfigFlags import ConfigFlags

    parser, overwrites = defineArgs(ConfigFlags)
    args = ConfigFlags.fillFromArgs([], parser)
    # Write user options to flags.Analysis
    updateConfigFlags(args, ConfigFlags, overwrites)

    # Arg checks
    assert not (
        ConfigFlags.Analysis.disable_calib and not is_physlite(ConfigFlags)
    ), "Disabling calibrations is not safe except on PHYSLITE!"

    assert not (
        ConfigFlags.Analysis.disable_trigger_filtering and not ConfigFlags.Input.isMC
    ), "Disabling trigger filtering only allowed for MC!"

    # Workaround for buggy glob, needed prior
    # to https://gitlab.cern.ch/atlas/athena/-/merge_requests/55561
    if ConfigFlags.Input.Files[0] == "_ATHENA_GENERIC_INPUTFILE_NAME_":
        ConfigFlags.Input.Files = ConfigFlags.Input.Files[1:]
    log.info(f"Operating on input files {ConfigFlags.Input.Files}")

    fileMD = GetFileMD(ConfigFlags.Input.Files)
    if ConfigFlags.Analysis.cache_metadata:
        update_metadata(Path("metadata.json"))
    ConfigFlags.addFlag("Input.AMITag", fileMD.get("AMITag", ""))
    ConfigFlags.addFlag("Input.SimulationFlavour", fileMD.get("SimulationFlavour", ""))

    ConfigFlags.addFlag("Analysis.DataType", lambda prevFlags: get_dataType(prevFlags))
    ConfigFlags.addFlag("Analysis.Years", lambda prevFlags: getRunYears(prevFlags))
    log.info(f"Configuring to match dataset from {ConfigFlags.Analysis.Years}")
    if max(ConfigFlags.Analysis.Years) <= 2018:
        ConfigFlags.addFlag("Analysis.Run", 2)
    elif min(ConfigFlags.Analysis.Years) >= 2022:
        ConfigFlags.addFlag("Analysis.Run", 3)
    else:
        raise RuntimeError("Invalid list of years, cannot combine runs")
    log.info(f"Configured years match Run {ConfigFlags.Analysis.Run}")

    # Lock the flags so that the configuration of job subcomponents cannot
    # modify them silently/unpredictably.
    ConfigFlags.lock()

    if ConfigFlags.Analysis.cache_metadata:
        cache_metadata(Path("metadata.json"))

    # Get a ComponentAccumulator setting up the standard components
    # needed to run an Athena job.
    # Setting temporarily needed for Run 3 code, to generate python
    # Configurable objects for deduplication
    from AthenaCommon.Configurable import ConfigurableRun3Behavior
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    with ConfigurableRun3Behavior():
        cfg = MainServicesCfg(ConfigFlags)

        from EventBookkeeperTools.EventBookkeeperToolsConfig import (
            CutFlowSvcCfg,
            BookkeeperToolCfg,
        )

        # output_name = "CutBookkeepers"
        # Needed for filtering, Athena only for now
        # from EventBookkeeperTools.CutFlowHelpers import CreateCutFlowSvc
        # Create CutFlowSvc otherwise the default CutFlowSvc that has only
        # one CutflowBookkeeper object, and can't deal with multiple weights
        cfg.merge(CutFlowSvcCfg(ConfigFlags))
        cfg.merge(BookkeeperToolCfg(ConfigFlags))
        # cfg.printConfig(withDetails=True, summariseProps=True)
        # Adjust the loop manager to announce the event number less frequently.
        # Makes a big difference if running over many events
        if ConfigFlags.Concurrency.NumThreads > 0:
            cfg.addService(
                CompFactory.AthenaHiveEventLoopMgr(EventPrintoutInterval=500)
            )
        else:
            cfg.addService(CompFactory.AthenaEventLoopMgr(EventPrintoutInterval=500))

        from AthenaRootComps.xAODEventSelectorConfig import xAODReadCfg

        cfg.merge(xAODReadCfg(ConfigFlags))

        dataType = ConfigFlags.Analysis.DataType

        log.info(
            f"Self-configured: dataType: '{dataType}', "
            f"is PHYSLITE? {is_physlite(ConfigFlags)}"
        )

        # Add our VariableDumper CA, calling the function defined above.
        from HH4bAnalysis.Config.TriggerLists import TriggerLists

        trigger_year_list = ConfigFlags.Analysis.trigger_year
        if trigger_year_list == "Auto":
            trigger_year_list = ConfigFlags.Analysis.Years
            log.info(
                "Self-configured trigger list for years: "
                f"{', '.join(str(year) for year in trigger_year_list) or None}"
            )

        trigger_chains = set()
        # Empty: set the HH4b analysis triggers
        trigger_groups = ConfigFlags.Analysis.trigger_list
        if trigger_groups == "Auto":
            log.info("No triggers specified, adding HH4b analysis triggers")
            trigger_groups = ["HH4bResolved", "HH4bBoosted"]
        try:
            for trigger_group in trigger_groups:
                for year in trigger_year_list:
                    trigger_chains |= set(TriggerLists[trigger_group][year])
        except KeyError as err:
            log.error(f"Trigger list for {trigger_group}, {year} not defined.")
            raise err

        trigger_chains = list(trigger_chains)
        if ConfigFlags.Analysis.disable_trigger_filtering:
            log.warning("Disabling trigger filtering, all events will pass!")

        from HH4bAnalysis.Config.GoodRunsLists import GoodRunsLists

        grl_runs = GoodRunsLists.keys()
        grl_files = []
        if dataType == "data":
            log.info(
                "Self-configured GRL for years: "
                f"{', '.join(str(year) for year in ConfigFlags.Analysis.Years) or None}"
            )
            grl_lists_by_year = {
                year: list
                for run in grl_runs
                for year, list in GoodRunsLists[run].items()
            }
            grl_files = [
                list
                for year in ConfigFlags.Analysis.Years
                for list in grl_lists_by_year[year]
            ]

        do_muons = not ConfigFlags.Analysis.cache_metadata

        do_PRW = _is_mc_phys(ConfigFlags)
        prw_files, lumicalc_files = [], []
        if do_PRW:
            try:
                prw_files, lumicalc_files = pileupConfigFiles(ConfigFlags)
            except LookupError as err:
                log.error(err)
                do_PRW = False

        log.info(f"Do PRW is {do_PRW}")

        cfg.addSequence(CompFactory.AthSequencer("HH4bSeq"), "AthAlgSeq")
        cfg.merge(
            AnalysisAlgsCfg(
                ConfigFlags,
                dataType,
                trigger_chains=trigger_chains,
                do_muons=do_muons,
                do_PRW=do_PRW,
                prw_files=prw_files,
                lumicalc_files=lumicalc_files,
                grl_files=grl_files,
            ),
            "HH4bSeq",
        )
        cfg.merge(
            MiniTupleCfg(
                ConfigFlags,
                trigger_chains=trigger_chains,
                do_muons=do_muons,
                do_PRW=do_PRW,
            ),
            "HH4bSeq",
        )

        if ConfigFlags.Analysis.write_h5_event:
            cfg.merge(getH5Cfg(ConfigFlags))

        # Print the full job configuration
        cfg.printConfig(summariseProps=False)

    # Execute the job defined in the ComponentAccumulator.
    # The number of events is specified by `args.evtMax`
    return_code = cfg.run(args.evtMax)
    if args.timeout:
        duration = time.process_time() - starttime
        if duration > args.timeout:
            raise RuntimeError(
                f'runtime ({duration:.1f}s) '
                f'exceed timeout ({args.timeout:.0f}s)')
    return return_code


# Execute the main function if this file was executed as a script
if __name__ == "__main__":
    code = main()
    sys.exit(0 if code.isSuccess() else 1)
