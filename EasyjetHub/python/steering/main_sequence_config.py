from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from EventBookkeeperTools.EventBookkeeperToolsConfig import (
    CutFlowSvcCfg,
    BookkeeperToolCfg,
)
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from AthenaRootComps.xAODEventSelectorConfig import xAODReadCfg

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
    overlap_VGammaOR,
)
from EasyjetHub.algs.truth.truth_config import truth_info_cfg
from EasyjetHub.output.ttree.minituple_config import minituple_cfg
from EasyjetHub.output.h5.h5_config import get_h5_cfg
from EasyjetHub.output.xaod import get_xaod_cfg
from EasyjetHub.steering.utils.log_helper import log

from EasyjetHub.algs.calibration.event_weights import (
    generator_sequence,
    pileup_sequence,
)


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

    # Needed for filtering, Athena only for now
    # Create CutFlowSvc otherwise the default CutFlowSvc that has only
    # one CutflowBookkeeper object, and can't deal with multiple weights
    cfg.merge(CutFlowSvcCfg(flags))
    cfg.merge(BookkeeperToolCfg(flags))

    # We need to use a pool file reader if we write out an xAOD
    if flags.Output.AODFileName:
        cfg.merge(PoolReadCfg(flags))
    else:
        cfg.merge(xAODReadCfg(flags))

    return cfg


# Select events early with trigger and data quality requirements
def preselection_cfg(flags, seqname):
    # Create the output CA to set the sequence correctly
    cfg = ComponentAccumulator()
    cfg.addSequence(CompFactory.AthSequencer(seqname))

    # adding weight seq (originally in cpalg)
    # Some decoration algorithms require the RandomRunNumber from PRW
    # Make sure this is available by adding first the relevant configs
    weightConfigSeq = ConfigSequence()

    weightSeq = CompFactory.AthSequencer('WeightSequence')
    weightConfigAccumulator = ConfigAccumulator(
        weightSeq,
        autoconfigFromFlags=flags,
    )

    if not flags.Analysis.disable_calib:
        if flags.Analysis.doPRW:
            log.info("Adding PRW sequence")
            # Adds variable to EventInfo if for pileup weight, for example:
            # EventInfo.PileWeight_%SYS$
            weightConfigSeq += pileup_sequence(flags)

        if flags.Input.isMC:
            log.info("Adding generator analysis sequence")
            # Adds variable to EventInfo if for generator weight, for example:
            # EventInfo.generatorWeight_%SYS%
            weightConfigSeq += generator_sequence(flags)

    weightConfigSeq.fullConfigure(weightConfigAccumulator)
    cfg.merge(weightConfigAccumulator.CA, seqname)

    if flags.Analysis.do_bbyy_analysis and flags.Input.isMC:
        from EasyjetHub.algs.truth.truth_config import (
            contain_dalitz, bbyy_filter_dalitz_cfg, sumofweightsalg_cfg)
        if contain_dalitz(flags):
            cfg.merge(bbyy_filter_dalitz_cfg(flags), seqname)
            cfg.merge(sumofweightsalg_cfg(flags), seqname)

    if flags.Input.MCChannelNumber in flags.Analysis.Truth.DSID_HSTP_samples:
        cfg.addEventAlgo(
            CompFactory.Easyjet.DijetHSTPFilter(),
            seqname
        )

    # Aggregate the configured CP algs in one ConfigSequence,
    # which will handle the container names, copying etc
    configSeq = ConfigSequence()
    # Remove overlap between Z+jet & Vgamma samples
    # Events in Z+jets are removed or a falg is added w/ the OR info

    if flags.Analysis.do_VgammaOR:
        if flags.Analysis.bypass_VGammaOR:
            log.warning("Disabling VgammaOR filtering, all events will pass!")
        log.info("Adding VgammaOR analysis algs")
        configSeq += overlap_VGammaOR(flags)

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
        tree_flags = flags.Analysis.ttree_output
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


def metadata_cfg(flags, **kwargs):
    cfg = ComponentAccumulator()

    if flags.Input.isMC:
        campaign = str(flags.Input.MCCampaign).replace("Campaign.MC", "mc")
        if flags.Sim.ISF.Simulator.usesFastCaloSim():
            dataType = "fastsim"
        else:
            dataType = "fullsim"
    else:
        dataType = "data"
        if 'Delayed' in flags.Input.TriggerStream:
            campaign = str(flags.Input.DataYear).replace("20", "delayeddata")
        else:
            campaign = str(flags.Input.DataYear).replace("20", "data")

    kwargs.setdefault("dataType", dataType)
    kwargs.setdefault("campaign", campaign)
    kwargs.setdefault("mcChannelNumber", str(flags.Input.MCChannelNumber))
    if hasattr(flags.Analysis, "ttree_output"):
        kwargs.setdefault("RootStreamName", 'CBK' if flags.Analysis.splitCBK else
                          flags.Analysis.ttree_output.stream_name)

    cfg.addEventAlgo(
        CompFactory.Easyjet.MetadataHistAlg(
            "MetadataHistAlg", **kwargs
        )
    )
    return cfg
