from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
from EventBookkeeperTools.EventBookkeeperToolsConfig import (
    CutFlowSvcCfg,
    BookkeeperToolCfg,
)
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

from AnalysisAlgorithmsConfig.ConfigAccumulator import ConfigAccumulator
from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence

from EasyjetHub.algs.cpalgs_config import cpalgs_cfg
from EasyjetHub.algs.event_counter_config import (
    event_counter_cfg,
    makeEventCounterConfig,
)
from EasyjetHub.algs.preselection.preselection_config import (
    event_selection_sequence,
    trigger_sequence,
)
from EasyjetHub.algs.truth.truth_config import truth_info_cfg
from EasyjetHub.output.ttree.minituple_config import minituple_cfg
from EasyjetHub.output.h5.h5_config import get_h5_cfg
from EasyjetHub.output.xaod import get_xaod_cfg
from EasyjetHub.steering.utils.log_helper import log


def default_sequence_cfg(flags, seqname):
    cfg = core_services_cfg(flags)
    cfg.merge(preselection_cfg(flags, seqname))
    cfg.merge(metadata_cfg(flags), seqname)
    cfg.merge(event_building_cfg(flags, seqname))

    return cfg


# Set up the basic event reading infrastructure
# Retrieve this as the starting CA to merge others into
# This should not be merged into a bare CA, as this
# can mess up the app configuration
def core_services_cfg(flags):
    # Get a ComponentAccumulator setting up the standard components
    # needed to run an Athena job.
    cfg = MainServicesCfg(flags)

    if flags.PerfMon.doFullMonMT:
        cfg.merge(PerfMonMTSvcCfg(flags))

    # Needed for filtering, Athena only for now
    # Create CutFlowSvc otherwise the default CutFlowSvc that has only
    # one CutflowBookkeeper object, and can't deal with multiple weights
    cfg.merge(CutFlowSvcCfg(flags))
    cfg.merge(BookkeeperToolCfg(flags))
    # Adjust the loop manager to announce the event number less frequently.
    # Makes a big difference if running over many events
    if flags.Concurrency.NumThreads > 0:
        cfg.addService(
            CompFactory.AthenaHiveEventLoopMgr(EventPrintoutInterval=500)
        )
    else:
        cfg.addService(CompFactory.AthenaEventLoopMgr(EventPrintoutInterval=500))

    from AthenaRootComps.xAODEventSelectorConfig import xAODReadCfg

    # We need to use a pool file reader if we write out an xAOD
    if flags.Output.AODFileName:
        cfg.merge(PoolReadCfg(flags))
    else:
        cfg.merge(xAODReadCfg(flags))

    return cfg


# Select events early with trigger and data quality requirements
def preselection_cfg(flags, seqname):
    # Aggregate the configured CP algs in one ConfigSequence,
    # which will handle the container names, copying etc
    configSeq = ConfigSequence()

    if not flags.Analysis.do_trigger_filtering:
        log.warning("Disabling trigger filtering, all events will pass!")

    log.info("Adding trigger analysis algs")
    # Removes events failing trigger and adds variable to EventInfo
    # if trigger passed or not, for example:
    # EventInfo.trigger_name
    configSeq += trigger_sequence(flags)
    if not flags.Analysis.suppress_metadata_json:
        makeEventCounterConfig(configSeq, "n_trigger")

    log.info("Add DQ event filter sequence")
    # Remove events failing DQ criteria
    configSeq += event_selection_sequence(flags)
    if not flags.Analysis.suppress_metadata_json:
        makeEventCounterConfig(configSeq, "n_data_quality")

    # Create the output CA to set the sequence correctly
    cfg = ComponentAccumulator()
    cfg.addSequence(CompFactory.AthSequencer(seqname))

    if flags.Analysis.do_bbyy_analysis and flags.Input.isMC:
        from bbyyAnalysis.bbyy_config import contain_dalitz, get_weight_index
        if contain_dalitz(flags):
            from bbyyAnalysis.bbyy_config import bbyy_filter_dalitz_cfg
            cfg.merge(bbyy_filter_dalitz_cfg(flags), seqname)

        if contain_dalitz(flags) or get_weight_index(flags):
            from EasyjetHub.algs.truth.truth_config import sumofweightsalg_cfg
            cfg.merge(sumofweightsalg_cfg(flags), seqname)

    # Define the sequence holding all the calibration
    preselSeq = CompFactory.AthSequencer('PreselectionSequence')

    # Activate the full configuration, which stitches together
    # the ConfigBlocks with interstitial container names etc
    configAccumulator = ConfigAccumulator(
        preselSeq,
        autoconfigFromFlags=flags,
    )
    configSeq.fullConfigure(configAccumulator)

    cfg.merge(configAccumulator.CA, seqname)
    return cfg


# Populate StoreGate with calibrated objects and other
# analysis inputs, including truth information
def event_building_cfg(flags, seqname):

    log.info(f"Do PRW is {flags.Analysis.doPRW}")

    # Create the output CA to set the sequence correctly
    cfg = ComponentAccumulator()
    cfg.addSequence(CompFactory.AthSequencer(seqname))

    cfg.merge(cpalgs_cfg(flags), seqname)

    if flags.Input.isMC:
        cfg.merge(truth_info_cfg(flags), seqname)

    return cfg


# Configure output file writing
def output_cfg(flags, seqname):

    cfg = ComponentAccumulator()
    cfg.addSequence(CompFactory.AthSequencer(seqname), "AthAlgSeq")

    # Configure however many TTree outputs are configured.
    # This config only handles one output file, as it comes from the
    # command line arguments. In principle we could set up multiple
    # output files, but that needs more custom config hooks
    # If additional branches need to be configured dynamically,
    # i.e. via python, then minituple_cfg should be called explicitly
    if flags.Analysis.out_file:
        for tree_flags in flags.Analysis.ttree_output:
            cfg.merge(
                minituple_cfg(
                    flags, tree_flags,
                    flags.Analysis.out_file,
                ),
                seqname,
            )

    if flags.Analysis.h5_output:
        cfg.merge(
            get_h5_cfg(flags),
            seqname,
        )

    if flags.Output.AODFileName:
        cfg.merge(
            get_xaod_cfg(flags, seqname),
            seqname
        )

    if not flags.Analysis.suppress_metadata_json:
        cfg.merge(event_counter_cfg("n_events"), seqname)

    return cfg


def metadata_cfg(flags):
    cfg = ComponentAccumulator()

    if flags.Input.isMC:
        if flags.Sim.ISF.Simulator.usesFastCaloSim():
            dataType = "fastsim"
        else:
            dataType = "fullsim"
    else:
        dataType = "data"

    cfg.addEventAlgo(
        CompFactory.Easyjet.MetadataHistAlg(
            "MetadataHistAlg",
            dataType=dataType,
            mcCampaign=str(flags.Input.MCCampaign),
            mcChannelNumber=str(flags.Input.MCChannelNumber),
        )
    )
    return cfg
