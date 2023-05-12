from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from HH4bAnalysis.cpalgs.event import (
    event_selection_sequence_cfg,
    generator_sequence_cfg,
    pileup_sequence_cfg,
    trigger_sequence_cfg,
)
from HH4bAnalysis.cpalgs.jets import (
    jet_sequence_cfg,
    vr_jet_sequence_cfg,
    lr_jet_sequence_cfg,
    lr_jet_ghost_vr_jet_association_cfg,
)
from HH4bAnalysis.cpalgs.muons import muon_sequence_cfg
from HH4bAnalysis.cpalgs.electrons import electron_sequence_cfg
from HH4bAnalysis.cpalgs.photons import photon_sequence_cfg
from HH4bAnalysis.cpalgs.overlap_removal import overlap_sequence_cfg
from HH4bAnalysis.config.container_names import get_container_names
from HH4bAnalysis.utils.log_helper import log
from HH4bAnalysis.utils.systematics_helper import consolidate_systematics_regex

from HH4bAnalysis.config.event_counter_config import event_counter_cfg

# Map object types to sequence configurators
analysis_seqs = {
    "muons":        muon_sequence_cfg,
    "electrons":    electron_sequence_cfg,
    "photons":      photon_sequence_cfg,
    "small_R_jets": jet_sequence_cfg,
    "VR_jets":      vr_jet_sequence_cfg,
}


# Generate the algorithm to do the dumping.
# AthAlgSequence does not respect filter decisions,
# so we will need to add a new sequence to the CA
def cpalgs_cfg(
    flags,
    trigger_chains=[],
    do_PRW=False,
    prw_files=[],
    lumicalc_files=[],
    grl_files=[],
):

    log.debug(f"Containers available in dataset: {flags.Input.Collections}")

    cfg = ComponentAccumulator()
    cfg.merge(event_counter_cfg("n_input"))

    # Create SystematicsSvc explicitly:
    sysSvc = CompFactory.CP.SystematicsSvc("SystematicsSvc")
    cfg.addService(sysSvc)
    if flags.Analysis.do_CP_systematics:
        sysSvc.sigmaRecommended = 1
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

    log.info("Adding trigger analysis algs")
    # Removes events failing trigger and adds variable to EventInfo
    # if trigger passed or not, for example:
    # EventInfo.trigger_name
    cfg.merge(trigger_sequence_cfg(flags, trigger_chains))
    cfg.merge(event_counter_cfg("n_trigger"))

    log.info("Add DQ event filter sequence")
    # Remove events failing DQ criteria
    cfg.merge(
        event_selection_sequence_cfg(
            flags, grlfiles=grl_files, loose=flags.Analysis.loose_jet_cleaning
        )
    )
    cfg.merge(event_counter_cfg("n_data_quality"))

    containers = get_container_names(flags)

    if not flags.Analysis.disable_calib:
        if do_PRW:
            log.info("Adding PRW sequence")
            # Adds variable to EventInfo if for pileup weight, for example:
            # EventInfo.PileWeight_%SYS$
            cfg.merge(
                pileup_sequence_cfg(
                    flags,
                    prwfiles=prw_files,
                    lumicalcfiles=lumicalc_files,
                )
            )
            log.info("Adding generator analysis sequence")
            # Adds variable to EventInfo if for generator weight, for example:
            # EventInfo.generatorWeight_%SYS%
            cfg.merge(generator_sequence_cfg(flags))

        for objtype in [
            "electrons",
            "photons",
            "muons",
            "small_R_jets",
            "VR_jets"
        ]:
            if flags(f"Analysis.do_{objtype}"):
                log.info(f"Adding {objtype} seq")
                cfg.merge(
                    analysis_seqs[objtype](
                        flags,
                        containers,
                    )
                )
                if objtype == "small_R_jets":
                    cfg.merge(event_counter_cfg("n_small_r"))
                if objtype == "VR_jets":
                    cfg.merge(event_counter_cfg("n_vr"))

        if flags.Analysis.do_large_R_Topo_jets:
            log.info("Adding large-R jet seq")
            cfg.merge(
                lr_jet_sequence_cfg(
                    flags,
                    containers,
                    lr_jet_type="Topo",
                )
            )
            cfg.merge(event_counter_cfg("n_large_r_topo"))

            if flags.Analysis.do_VR_jets:
                cfg.merge(
                    lr_jet_ghost_vr_jet_association_cfg(
                        flags,
                        containers,
                        lr_jet_type="Topo",
                    )
                )

        if flags.Analysis.do_large_R_UFO_jets:
            log.info("Adding UFO large-R jet seq")
            cfg.merge(
                lr_jet_sequence_cfg(
                    flags,
                    containers,
                    lr_jet_type="UFO"
                )
            )
            cfg.merge(event_counter_cfg("n_large_r_ufo"))

            if flags.Analysis.do_VR_jets:
                cfg.merge(
                    lr_jet_ghost_vr_jet_association_cfg(
                        flags,
                        containers,
                        lr_jet_type="UFO",
                    )
                )

    ########################################################################
    # Begin postprocessing
    ########################################################################

    if flags.Analysis.do_overlap_removal:
        log.info("Adding Overlap Removal sequence")

        cfg.merge(
            overlap_sequence_cfg(
                flags,
                containers,
            )
        )
        cfg.merge(event_counter_cfg("n_overlap"))

    return cfg
