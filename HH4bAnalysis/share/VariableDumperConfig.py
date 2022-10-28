#!/bin/env python

#
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

#
# VariableDumperConfig.py
# A simple CA file to create a tree of variables
#

import sys

from AthenaConfiguration.AutoConfigFlags import GetFileMD
from AthenaConfiguration.ComponentFactory import CompFactory
from HH4bAnalysis.Config.AnalysisAlgsConfig import AnalysisAlgsCfg
from HH4bAnalysis.Config.Base import ConfigFlagsAdder, getRunYears, pileupConfigFiles
from HH4bAnalysis.Config.MiniTupleConfig import MiniTupleCfg
from HH4bAnalysis.utils.inputsHelper import get_dataType, is_physlite
from HH4bAnalysis.utils.logHelper import log


def defineArgs(ConfigFlags):
    # Generate a parser and add an output file argument, then retrieve the args
    parser = ConfigFlags.getArgumentParser()
    parser.add_argument(
        "--runConfig",
        type=str,
        required=True,
        help="Run config file path",
    )
    parser.add_argument(
        "--outFile",
        type=str,
        help="Output file name",
    )
    parser.add_argument(
        "--disable-trigger-filtering",
        action="store_true",
        help=(
            "Disable trigger filtering (to get all events to pass). "
            "Has no effect on data."
        ),
    )
    parser.add_argument(
        "-c",
        "--meta-cache",
        action="store_true",
        help="use metadata cache file, defaults to %(const)s",
    )
    parser.add_argument(
        "--disable-calib",
        action="store_true",
        help=(
            "disable CP Algs for calibration "
            "(can be used for plain PHYSLITE processing)"
        ),
    )
    parser.add_argument(
        "--allow-no-ptag",
        action="store_true",
        help=(
            "disable ptag detection for CI tests"
            "(avoids CBK failure on test files)"
        ),
    )
    return parser


def _is_mc_phys(flags):
    return flags.Input.isMC and not is_physlite(flags)


# CA modules are intended to be executable, to facilitate easy testing.
# We define a "main function" that will run a test job if the module
# is executed rather than imported.
def main():
    # Import the job configuration flags, some of which will be autoconfigured.
    # These are used for steering the job, and include e.g. the input file (list).
    from AthenaConfiguration.AllConfigFlags import ConfigFlags

    parser = defineArgs(ConfigFlags)
    args = ConfigFlags.fillFromArgs([], parser)
    # Write user options to ConfigFlags
    ConfigFlags = ConfigFlagsAdder(args, ConfigFlags)

    # Arg checks
    assert not (
        ConfigFlags.Analysis.disable_calib and not is_physlite(ConfigFlags)
    ), "Disabling calibrations is not safe except on PHYSLITE!"

    # Workaround for buggy glob, needed prior
    # to https://gitlab.cern.ch/atlas/athena/-/merge_requests/55561
    if ConfigFlags.Input.Files[0] == "_ATHENA_GENERIC_INPUTFILE_NAME_":
        ConfigFlags.Input.Files = ConfigFlags.Input.Files[1:]
    log.info(f"Operating on input files {ConfigFlags.Input.Files}")

    fileMD = GetFileMD(ConfigFlags.Input.Files)
    ConfigFlags.addFlag("Input.AMITag", fileMD.get("AMITag", ""))
    ConfigFlags.addFlag("Input.SimulationFlavour", fileMD.get("SimulationFlavour", ""))

    # Lock the flags so that the configuration of job subcomponents cannot
    # modify them silently/unpredictably.
    ConfigFlags.lock()

    # Get a ComponentAccumulator setting up the standard components
    # needed to run an Athena job.
    # Setting temporarily needed for Run 3 code, to generate python
    # Configurable objects for deduplication
    from AthenaCommon.Configurable import ConfigurableRun3Behavior
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    with ConfigurableRun3Behavior():
        cfg = MainServicesCfg(ConfigFlags)

        from EventBookkeeperTools.EventBookkeeperToolsConfig import CutFlowSvcCfg

        # Create CutFlowSvc otherwise the default CutFlowSvc that has only
        # one CutflowBookkeeper object, and can't deal with multiple weights
        cfg.merge(CutFlowSvcCfg(ConfigFlags))

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

        dataType = get_dataType(ConfigFlags)

        log.info(
            f"Self-configured: dataType: '{dataType}', "
            f"is PHYSLITE? {is_physlite(ConfigFlags)}"
        )

        self_configured_run_years = getRunYears(ConfigFlags, dataType)

        # Add our VariableDumper CA, calling the function defined above.
        from HH4bAnalysis.Config.TriggerLists import TriggerLists

        trigger_year_list = ConfigFlags.Analysis.trigger_year
        if trigger_year_list == "Auto":
            trigger_year_list = self_configured_run_years
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
        if ConfigFlags.Analysis.disable_trigger_filtering and dataType != "data":
            log.warning("Disabling trigger filtering, all events will pass!")
            trigger_chains.insert(0, "L1_RD0_FILLED")

        from HH4bAnalysis.Config.GoodRunsLists import GoodRunsLists

        grl_runs = GoodRunsLists.keys()
        grl_files = []
        if dataType == "data":
            log.info(
                "Self-configured GRL for years: "
                f"{', '.join(str(year) for year in self_configured_run_years) or None}"
            )
            grl_lists_by_year = {
                year: list
                for run in grl_runs
                for year, list in GoodRunsLists[run].items()
            }
            grl_files = [
                list
                for year in self_configured_run_years
                for list in grl_lists_by_year[year]
            ]

        do_muons = not ConfigFlags.Analysis.meta_cache

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

        # Print the full job configuration
        cfg.printConfig(summariseProps=False)

    # Execute the job defined in the ComponentAccumulator.
    # The number of events is specified by `args.evtMax`
    return cfg.run(args.evtMax)


# Execute the main function if this file was executed as a script
if __name__ == "__main__":
    code = main()
    sys.exit(0 if code.isSuccess() else 1)
