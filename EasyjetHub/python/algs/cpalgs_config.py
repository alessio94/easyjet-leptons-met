from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from EasyjetHub.algs.calibration.event_weights import (
    generator_sequence_cfg,
    pileup_sequence_cfg,
)
from EasyjetHub.algs.calibration.jets import (
    jet_sequence_cfg,
    vr_jet_sequence_cfg,
    lr_jet_sequence_cfg,
    lr_jet_ghost_vr_jet_association_cfg,
)
from EasyjetHub.algs.calibration.muons import muon_sequence_cfg
from EasyjetHub.algs.calibration.electrons import electron_sequence_cfg
from EasyjetHub.algs.calibration.photons import photon_sequence_cfg
from EasyjetHub.algs.calibration.taus import tau_sequence_cfg
from EasyjetHub.algs.postprocessing.overlap_removal import overlap_sequence_cfg
from EasyjetHub.steering.container_names import get_container_names
from EasyjetHub.steering.utils.log_helper import log
from EasyjetHub.steering.utils.systematics_helper import consolidate_systematics_regex

from EasyjetHub.algs.event_counter_config import event_counter_cfg

# Map object types to sequence configurators
analysis_seqs = {
    "muons":        muon_sequence_cfg,
    "electrons":    electron_sequence_cfg,
    "photons":      photon_sequence_cfg,
    "taus":         tau_sequence_cfg,
    "small_R_jets": jet_sequence_cfg,
    "VR_jets":      vr_jet_sequence_cfg,
}


# Generate the algorithm to do the dumping.
# AthAlgSequence does not respect filter decisions,
# so we will need to add a new sequence to the CA
def cpalgs_cfg(
    flags,
    prw_files=[],
    lumicalc_files=[],
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

    containers = get_container_names(flags)

    if not flags.Analysis.disable_calib:
        if flags.Analysis.doPRW:
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
            "taus",
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
