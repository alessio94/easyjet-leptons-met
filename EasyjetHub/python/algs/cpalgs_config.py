from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AnalysisAlgorithmsConfig.ConfigAccumulator import ConfigAccumulator
from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence

from AthenaConfiguration.ComponentFactory import CompFactory
from EasyjetHub.algs.calibration.jets import (
    jet_sequence,
    lr_jet_sequence,
)
from EasyjetHub.output.ttree.GN2X_decor_config import GN2X_decor_cfg
from EasyjetHub.output.ttree.wtag_decor_config import wtag_decor_cfg
from EasyjetHub.output.ttree.tau_decor_config import tau_decor_cfg
from EasyjetHub.output.ttree.jet_decor_config import jet_decor_cfg
from EasyjetHub.output.ttree.electron_decor_config import electron_decor_config

from EasyjetHub.algs.calibration.muons import muon_sequence
from EasyjetHub.algs.calibration.electrons import electron_sequence
from EasyjetHub.algs.calibration.photons import photon_sequence
from EasyjetHub.algs.calibration.taus import tau_sequence
from EasyjetHub.algs.calibration.met import met_sequence
from EasyjetHub.algs.calibration.selection_decoration import (
    selection_decoration_sequence)
from EasyjetHub.algs.postprocessing.overlap_removal import overlap_sequence
from EasyjetHub.algs.calibration.triggerSF import triggerSF_sequence
from EasyjetHub.steering.utils.log_helper import log
from EasyjetHub.steering.utils.systematics_helper import consolidate_systematics_regex

from EasyjetHub.algs.event_counter_config import event_counter_cfg
from EasyjetHub.algs.event_info_global_alg_config import event_info_global_alg_cfg

import pathlib
import os
import yaml

# Map object types to sequence configurators
analysis_seqs = {
    "muons": muon_sequence,
    "electrons": electron_sequence,
    "photons": photon_sequence,
    "taus": tau_sequence,
    "small_R_jets": jet_sequence,
}


def cpalgs_cfg(flags):

    log.debug(f"Containers available in dataset: {flags.Input.Collections}")

    cfg = ComponentAccumulator()
    if not flags.Analysis.suppress_metadata_json:
        cfg.merge(event_counter_cfg("n_input"))

    # Create SystematicsSvc explicitly:
    sysSvc = CompFactory.CP.SystematicsSvc("SystematicsSvc")
    cfg.addService(sysSvc)

    sys_weight_name = get_sys_weight_name(flags)

    if flags.Analysis.do_CP_systematics or (bool)(sys_weight_name):
        sysSvc.sigmaRecommended = 1

        if not flags.Analysis.do_CP_systematics and (bool)(sys_weight_name):
            # Convert sys weight name to be consolidated
            sys_weight_name = [f".*{sys_weight_name}"]
            systs = consolidate_systematics_regex(sys_weight_name)
        else:
            systs = consolidate_systematics_regex(flags.Analysis.systematics_regex)

        log.info("Systematics regex:")
        log.info(systs)
        sysSvc.systematicsRegex = systs

        syslistalg = CompFactory.CP.SysListDumperAlg(
            "SysList",
            histogramName="systematics",
            systematicsService=sysSvc,
        )
        cfg.addEventAlgo(syslistalg)

    # Create SelectionNameSvc explicitly
    selectionSvc = CompFactory.CP.SelectionNameSvc("SelectionNameSvc")
    cfg.addService(selectionSvc)

    # Extra decoration algorithms

    # Comput global EventInfo decoration, in particular data-taking year
    # Needed for MC20a = 2015+2016
    cfg.merge(event_info_global_alg_cfg(flags))

    if flags.Analysis.do_small_R_jets:
        # Schedule the alg to decorate jets with extra info
        cfg.merge(jet_decor_cfg(flags))

    if flags.Analysis.do_taus:
        # Schedule the alg to decorate taus with extra info
        cfg.merge(tau_decor_cfg(flags))

    if flags.Analysis.do_electrons:
        # Schedule the alg to decorate electrons with extra info
        cfg.merge(electron_decor_config(flags))

    if flags.Analysis.Large_R_jet.wtag_type and flags.Analysis.Large_R_jet.wtag_wp:
        cfg.merge(wtag_decor_cfg(flags))

    if flags.Analysis.Large_R_jet.GN2X_hbb_wps:
        cfg.merge(GN2X_decor_cfg(flags))

    # Aggregate the configured CP algs in one ConfigSequence,
    # which will handle the container names, copying etc
    configSeq = ConfigSequence()

    # Define the sequence holding all the calibration
    # Activate the full configuration, which stitches together
    # the ConfigBlocks with interstitial container names etc
    calibSeq = CompFactory.AthSequencer('CPAlgSequence')
    configAccumulator = ConfigAccumulator(
        calibSeq,
        autoconfigFromFlags=flags,
    )

    if not flags.Analysis.disable_calib:

        for objtype in [
            "electrons",
            "photons",
            "muons",
            "taus",
            "small_R_jets"
        ]:
            if flags.Analysis[f"do_{objtype}"]:
                log.info(f"Adding {objtype} seq")

                # Append the configured CP calibration sequence for
                # the given object type
                # Pass the configAccumulator because we may need to
                # provide some container name info
                configSeq += analysis_seqs[objtype](flags, configAccumulator)

        if flags.Analysis.do_large_R_Topo_jets:
            log.info("Adding large-R jet seq")
            configSeq += lr_jet_sequence(
                flags,
                lr_jet_type="Topo",
                configAcc=configAccumulator,
            )

        if flags.Analysis.do_large_R_UFO_jets:
            log.info("Adding UFO large-R jet seq")
            configSeq += lr_jet_sequence(
                flags,
                lr_jet_type="UFO",
                configAcc=configAccumulator,
            )

        if flags.Analysis.do_met:
            log.info("Adding MET seq")
            configSeq += met_sequence(flags, configAccumulator)

    ########################################################################
    # Begin postprocessing
    ########################################################################

    if flags.Analysis.do_overlap_removal:
        log.info("Adding Overlap Removal sequence")

        configSeq += overlap_sequence(flags)

    if flags.Input.isMC and flags.Analysis.Trigger.scale_factor.doSF:
        configSeq += triggerSF_sequence(flags)

    configSeq += selection_decoration_sequence(flags)

    configSeq.fullConfigure(configAccumulator)

    cfg.merge(configAccumulator.CA)

    return cfg


def get_sys_weight_name(input):
    # Determine if input is flags or an integer DSID
    if isinstance(input, int):
        dsid = input
    else:
        dsid = int(input.Input.MCChannelNumber)

    def FullPath(rawpath):
        fpath = pathlib.Path(rawpath)
        for dirpath in [""] + os.environ["DATAPATH"].split(":"):
            fullpath = dirpath / fpath
            if fullpath.exists():
                return fullpath

    # file name hard-coded
    with open(FullPath("EasyjetHub/SpecialWeightIndices.yaml"), 'r') as file_in:
        try:
            content = yaml.safe_load(file_in)
        except yaml.YAMLError as exc:
            raise ValueError(f"Error in configuration file: {exc}")

        for entry in content:
            if entry['DSID'] == dsid:
                sys_weight_name = entry.get("sysWeightName", "")

                # Exception in case I forgot to map the DSID with
                # the sys weight name for a MC sample.
                if sys_weight_name is None:
                    raise ValueError(
                        f"DSID {dsid} must have sysWeightName"
                    )

                return sys_weight_name if sys_weight_name else ""

    return ""  # in case no matching DSID is found
