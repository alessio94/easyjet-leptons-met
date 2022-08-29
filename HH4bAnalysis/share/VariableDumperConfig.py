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

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AutoConfigFlags import GetFileMD

from HH4bAnalysis.Config.Base import pileupConfigFiles, getRunYears
from HH4bAnalysis.Config.AnalysisAlgsConfig import AnalysisAlgsCfg
from HH4bAnalysis.Config.MiniTupleConfig import MiniTupleCfg
from HH4bAnalysis.utils.inputsHelper import is_physlite
from HH4bAnalysis.utils.logHelper import log


def defineArgs(ConfigFlags):
    # Generate a parser and add an output file argument, then retrieve the args
    parser = ConfigFlags.getArgumentParser()
    parser.add_argument(
        "--outFile",
        type=str,
        default="analysis-variables.root",
        help="Output file name",
    )
    parser.add_argument(
        "--btag-wps",
        type=str,
        nargs="*",
        default=[
            "DL1dv00_FixedCutBEff_70",
            "DL1dv00_FixedCutBEff_77",
            "DL1dv00_FixedCutBEff_85",
        ],
        help="Btag working points default %(default)s",
    )
    parser.add_argument(
        "--vr-btag-wps",
        type=str,
        nargs="*",
        default=[
            "DL1r_FixedCutBEff_77",
            "DL1r_FixedCutBEff_85",
        ],
        help="VR Jets btag working points default %(default)s",
    )
    parser.add_argument(
        "--trigger-list",
        action="extend",
        nargs="+",
        type=str,
        default=[],
        help=(
            "Trigger list to use, default: %(default)s. "
            "Will use run number to set trigger list."
        ),
    )
    parser.add_argument(
        "--trigger-year",
        action="extend",
        nargs="+",
        type=int,
        default=[],
        help=(
            "Years used to define the trigger list. "
            "Default empty list will auto-configure from file metadata."
        ),
    )
    parser.add_argument(
        "-c",
        "--meta-cache",
        type=Path,
        default=None,
        nargs="?",
        const=Path("metadata.json"),
        help="use metadata cache file, defaults to %(const)s",
    )
    parser.add_argument(
        "-o",
        "--loose",
        action="store_true",
        help="use loose event cleaning (to get something to pass)",
    )
    parser.add_argument(
        "--do_dihiggs_analysis",
        action="store_true",
        help="run DiHiggs anaylysis",
    )
    parser.add_argument(
        "--do_resolved_analysis",
        action="store_true",
        default=True,
        help="activate resolved analysis",
    )
    parser.add_argument(
        "--do_boosted_analysis",
        action="store_true",
        default=True,
        help="activate boosted",
    )
    parser.add_argument(
        "--disable-calib",
        action="store_true",
        help=(
            "disable CP Algs for calibration "
            "(can be used for plain PHYSLITE processing)"
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

    # Get the arguments, defined at the top for easy browsing
    parser = defineArgs(ConfigFlags)
    args = ConfigFlags.fillFromArgs([], parser)

    # Arg checks
    assert not (
        args.disable_calib and not is_physlite(ConfigFlags)
    ), "Disabling calibrations is not safe except on PHYSLITE!"

    # Lock the flags so that the configuration of job subcomponents cannot
    # modify them silently/unpredictably.
    # Workaround for buggy glob, needed prior
    # to https://gitlab.cern.ch/atlas/athena/-/merge_requests/55561
    if ConfigFlags.Input.Files[0] == "_ATHENA_GENERIC_INPUTFILE_NAME_":
        ConfigFlags.Input.Files = ConfigFlags.Input.Files[1:]
    log.info(f"Operating on input files {ConfigFlags.Input.Files}")

    fileMD = GetFileMD(ConfigFlags.Input.Files)
    ConfigFlags.addFlag("Input.AMITag", fileMD.get("AMITag", ""))
    ConfigFlags.addFlag("Input.SimulationFlavour", fileMD.get("SimulationFlavour", ""))
    ConfigFlags.addFlag("do_resolved_analysis", args.do_resolved_analysis)
    ConfigFlags.addFlag("do_boosted_analysis", args.do_boosted_analysis)

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

        from HH4bAnalysis.Config.xAODEventSelectorConfig import xAODReadCfg

        cfg.merge(xAODReadCfg(ConfigFlags))

        # Add our VariableDumper CA, calling the function defined above.
        from HH4bAnalysis.Config.TriggerLists import TriggerLists

        trigger_year_list = args.trigger_year
        if not trigger_year_list:
            trigger_year_list = getRunYears(ConfigFlags)
        log.info(
            "Self-configured trigger list for years: "
            f"{', '.join(str(year) for year in trigger_year_list) or None}"
        )

        trigger_chains = set()
        # Empty: set the HH4b analysis triggers
        trigger_groups = args.trigger_list
        if not trigger_groups:
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

        from HH4bAnalysis.Config.GoodRunsLists import GoodRunsLists

        grl_runs = GoodRunsLists.keys()
        grl_files = []
        if not ConfigFlags.Input.isMC:
            run_years = getRunYears(ConfigFlags)
            log.info(
                "Self-configured GRL for years: "
                f"{', '.join(str(year) for year in run_years) or None}"
            )
            grl_lists_by_year = {
                year: list
                for run in grl_runs
                for year, list in GoodRunsLists[run].items()
            }
            grl_files = [list for year in run_years for list in grl_lists_by_year[year]]

        do_muons = not args.meta_cache

        do_PRW = _is_mc_phys(ConfigFlags)
        prw_files, lumicalc_files = [], []
        if do_PRW:
            try:
                prw_files, lumicalc_files = pileupConfigFiles(ConfigFlags)
            except LookupError as err:
                log.error(err)
                do_PRW = False

        cfg.addSequence(CompFactory.AthSequencer("HH4bSeq"), "AthAlgSeq")
        cfg.merge(
            AnalysisAlgsCfg(
                ConfigFlags,
                btag_wps=args.btag_wps,
                vr_btag_wps=args.vr_btag_wps,
                disable_calib=args.disable_calib,
                trigger_chains=trigger_chains,
                metadata_cache=args.meta_cache,
                do_muons=do_muons,
                do_loose=args.loose,
                do_PRW=do_PRW,
                prw_files=prw_files,
                lumicalc_files=lumicalc_files,
                grl_files=grl_files,
                do_dihiggs_analysis=args.do_dihiggs_analysis,
            ),
            "HH4bSeq",
        )
        cfg.merge(
            MiniTupleCfg(
                ConfigFlags,
                outfname=args.outFile,
                trigger_chains=trigger_chains,
                working_points={"ak4": args.btag_wps, "vr": args.vr_btag_wps},
                do_muons=do_muons,
                do_PRW=do_PRW,
                do_dihiggs_analysis=args.do_dihiggs_analysis,
                disable_calib=args.disable_calib,
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
