from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from HH4bAnalysis.config.boosted_config import boosted_cfg
from HH4bAnalysis.cpalgs.electrons import electron_sequence_cfg
from HH4bAnalysis.cpalgs.event import (
    event_selection_sequence_cfg,
    generator_sequence_cfg,
    pileup_sequence_cfg,
    trigger_sequence_cfg,
)
from HH4bAnalysis.cpalgs.jets import (
    lr_jet_sequence_cfg,
    jet_sequence_cfg,
    lr_jet_ghost_vr_jet_association_cfg,
    vr_jet_sequence_cfg,
)
from HH4bAnalysis.cpalgs.muons import muon_sequence_cfg
from HH4bAnalysis.cpalgs.photons import photon_sequence_cfg
from HH4bAnalysis.cpalgs.overlap_removal import overlap_sequence_cfg
from HH4bAnalysis.config.resolved_config import resolved_cfg
from HH4bAnalysis.config.truth_particle_info_config import truth_particle_info_cfg
from HH4bAnalysis.config.container_names import get_container_names
from HH4bAnalysis.utils.log_helper import log
from HH4bAnalysis.utils.systematics_helper import consolidate_systematics_regex

from HH4bAnalysis.config.event_counter_config import event_counter_cfg
from HH4bAnalysis.config.jet_parent_decorator_config import jet_parent_decorator_cfg


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

    # truth record seems to be broken in physlite
    if flags.Input.isMC and not flags.Input.isPHYSLITE:
        cfg.merge(jet_parent_decorator_cfg(
            flags,
            jet_collection=containers["inputs"]["reco4Jet"],
            name_prefix="smallR",
            match_dr=0.3
        ))
        cfg.merge(jet_parent_decorator_cfg(
            flags,
            jet_collection=containers["inputs"]["reco10UFOJet"],
            name_prefix="largeR",
            match_dr=0.9,
        ))

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

        if flags.Analysis.do_electrons:
            log.info("Adding electron seq")
            cfg.merge(
                electron_sequence_cfg(
                    flags,
                    containers,
                )
            )

        if flags.Analysis.do_photons:
            log.info("Adding photon seq")
            cfg.merge(
                photon_sequence_cfg(
                    flags,
                    containers,
                )
            )

        if flags.Analysis.do_muons:
            log.info("Adding muon seq")
            cfg.merge(
                muon_sequence_cfg(
                    flags,
                    containers,
                )
            )

        if flags.Analysis.do_small_R_jets:
            log.info("Adding small-R jet seq")
            cfg.merge(
                jet_sequence_cfg(
                    flags,
                    containers,
                )
            )
            cfg.merge(event_counter_cfg("n_small_r"))

        if flags.Analysis.do_large_R_Topo_jets:
            log.info("Adding large-R jet seq")
            cfg.merge(
                lr_jet_sequence_cfg(
                    flags,
                    containers,
                    lr_jet_type="Topo",
                )
            )
            cfg.merge(event_counter_cfg("n_large_r"))

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
            log.info("Adding VR jet seq")
            cfg.merge(
                vr_jet_sequence_cfg(
                    flags,
                    containers,
                )
            )
            cfg.merge(event_counter_cfg("n_vr"))

            if flags.Analysis.do_large_R_Topo_jets:
                cfg.merge(
                    lr_jet_ghost_vr_jet_association_cfg(
                        flags,
                        containers,
                        lr_jet_type="Topo",
                    )
                )

            if flags.Analysis.do_large_R_UFO_jets:
                cfg.merge(
                    lr_jet_ghost_vr_jet_association_cfg(
                        flags,
                        containers,
                        lr_jet_type="UFO",
                    )
                )

        if flags.Input.isMC:
            log.info("Adding truth particle info seq")
            cfg.merge(
                truth_particle_info_cfg(
                    flags,
                    containers,
                )
            )
            cfg.merge(event_counter_cfg("n_truth_particle"))

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

    if flags.Analysis.do_resolved_dihiggs_analysis and not flags.Analysis.disable_calib:
        cfg.merge(
            resolved_cfg(
                flags,
                smalljetkey=containers["outputs"]["reco4Jet"].replace("%SYS%", "NOSYS"),
            )
        )
        cfg.merge(event_counter_cfg("n_resolved"))
    if flags.Analysis.do_boosted_dihiggs_analysis and not flags.Analysis.disable_calib:
        cfg.merge(
            boosted_cfg(
                flags,
                largejetkey=containers["outputs"]["reco10TopoJet"].replace(
                    "%SYS%", "NOSYS"
                ),
            )
        )
        cfg.merge(event_counter_cfg("n_merged"))

    return cfg
