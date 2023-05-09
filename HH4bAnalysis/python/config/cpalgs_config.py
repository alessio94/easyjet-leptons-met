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
    lr_ufo_jet_sequence_cfg,
    jet_sequence_cfg,
    lr_jet_ghost_vr_jet_association_cfg,
    lr_ufo_jet_ghost_vr_jet_association_cfg,
    vr_jet_sequence_cfg,
)
from HH4bAnalysis.cpalgs.muons import muon_sequence_cfg
from HH4bAnalysis.cpalgs.photons import photon_sequence_cfg
from HH4bAnalysis.cpalgs.postprocessing import overlap_sequence_cfg
from HH4bAnalysis.config.resolved_config import resolved_cfg
from HH4bAnalysis.config.truth_particle_info_config import truth_particle_info_cfg
from HH4bAnalysis.config.container_names import get_container_names
from HH4bAnalysis.utils.inputs_helper import is_physlite
from HH4bAnalysis.utils.log_helper import log
from HH4bAnalysis.utils.systematics_helper import consolidate_systematics_regex

from HH4bAnalysis.config.event_counter_config import event_counter_cfg
from HH4bAnalysis.config.jet_parent_decorator_config import jet_parent_decorator_cfg


# Generate the algorithm to do the dumping.
# AthAlgSequence does not respect filter decisions,
# so we will need to add a new sequence to the CA
def cpalgs_cfg(
    flags,
    datatype,
    trigger_chains=[],
    do_PRW=False,
    prw_files=[],
    lumicalc_files=[],
    grl_files=[],
):
    is_daod_physlite = is_physlite(flags)

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
    cfg.merge(trigger_sequence_cfg(flags, datatype, trigger_chains))
    cfg.merge(event_counter_cfg("n_trigger"))

    log.info("Add DQ event filter sequence")
    # Remove events failing DQ criteria
    cfg.merge(
        event_selection_sequence_cfg(
            flags, datatype, grlfiles=grl_files, loose=flags.Analysis.loose_jet_cleaning
        )
    )
    cfg.merge(event_counter_cfg("n_data_quality"))

    containers = get_container_names(flags)

    # truth record seems to be broken in physlite
    if flags.Input.isMC and not is_physlite(flags):
        cfg.merge(jet_parent_decorator_cfg(flags))

    if not flags.Analysis.disable_calib:
        if do_PRW:
            log.info("Adding PRW sequence")
            # Adds variable to EventInfo if for pileup weight, for example:
            # EventInfo.PileWeight_%SYS$
            cfg.merge(
                pileup_sequence_cfg(
                    flags,
                    datatype=datatype,
                    prwfiles=prw_files,
                    lumicalcfiles=lumicalc_files,
                )
            )
            log.info("Adding generator analysis sequence")
            # Adds variable to EventInfo if for generator weight, for example:
            # EventInfo.generatorWeight_%SYS%
            cfg.merge(generator_sequence_cfg(flags, datatype))

        log.info("Adding electron seq")
        cfg.merge(
            electron_sequence_cfg(
                flags,
                datatype=datatype,
                incontainername=containers["inputs"]["electrons"],
                outcontainername=containers["outputs"]["electrons"],
            )
        )

        log.info("Adding photon seq")
        cfg.merge(
            photon_sequence_cfg(
                flags,
                datatype=datatype,
                incontainername=containers["inputs"]["photons"],
                outcontainername=containers["outputs"]["photons"],
            )
        )

        if flags.Analysis.do_muons:
            log.info("Adding muon seq")
            cfg.merge(
                muon_sequence_cfg(
                    flags,
                    datatype=datatype,
                    incontainername=containers["inputs"]["muons"],
                    outcontainername=containers["outputs"]["muons"],
                )
            )

        log.info("Adding small-R jet seq")
        muoncont = containers["outputs"]["muons"]
        if not flags.Analysis.do_muons:
            muoncont = containers["inputs"]["muons"]

        cfg.merge(
            jet_sequence_cfg(
                flags,
                datatype=datatype,
                incontainername=containers["inputs"]["reco4Jet"],
                outcontainername=containers["outputs"]["reco4Jet"],
                # Need muons for b-jet pt correction
                muoncontainername=muoncont,
                do_bjet_ptcalib=flags.Analysis.do_muons,
                is_daod_physlite=is_daod_physlite,
            )
        )
        cfg.merge(event_counter_cfg("n_small_r"))

        if is_daod_physlite:
            log.warning("On PHYSLITE, skip large-R jet sequence for now")
        else:
            log.info("Adding large-R jet seq")
            cfg.merge(
                lr_jet_sequence_cfg(
                    flags,
                    datatype=datatype,
                    incontainername=containers["inputs"]["reco10Jet"],
                    outcontainername=containers["outputs"]["reco10Jet"],
                )
            )
            cfg.merge(event_counter_cfg("n_large_r"))

        if is_daod_physlite:
            log.warning("On PHYSLITE, skip  UFO large-R jet sequence for now")
        else:
            log.info("Adding UFO large-R jet seq")
            cfg.merge(
                lr_ufo_jet_sequence_cfg(
                    flags,
                    datatype=datatype,
                    incontainername=containers["inputs"]["reco10UFOJet"],
                    outcontainername=containers["outputs"]["reco10UFOJet"],
                )
            )
            cfg.merge(event_counter_cfg("n_large_r_ufo"))

        if is_daod_physlite:
            log.warning("On PHYSLITE, skip VR jet sequence for now")
        else:
            log.info("Adding VR jet seq")
            cfg.merge(
                vr_jet_sequence_cfg(
                    flags,
                    datatype=datatype,
                    incontainername=containers["inputs"]["vrJet"],
                    outcontainername=containers["outputs"]["vrJet"],
                )
            )
            cfg.merge(event_counter_cfg("n_vr"))

        if is_daod_physlite:
            log.warning("On PHYSLITE, skip ghost assocciation VR jet sequence for now")
        else:
            cfg.merge(
                lr_jet_ghost_vr_jet_association_cfg(
                    flags,
                    inlrjet_containername=containers["outputs"]["reco10Jet"].replace(
                        "%SYS%", "NOSYS"
                    ),
                )
            )

        if is_daod_physlite:
            log.warning("On PHYSLITE, skip ghost assocciation VR jet sequence for now")
        else:
            cfg.merge(
                lr_ufo_jet_ghost_vr_jet_association_cfg(
                    flags,
                    inlrufojet_containername=containers["outputs"][
                        "reco10UFOJet"
                    ].replace("%SYS%", "NOSYS"),
                )
            )

        if flags.Input.isMC:
            log.info("Adding truth particle info seq")
            cfg.merge(
                truth_particle_info_cfg(
                    flags,
                    sm_containerinkey=containers["inputs"]["truthSMParticles"],
                    bsm_containerinkey=containers["inputs"]["truthBSMParticles"],
                    containeroutkey=containers["outputs"]["truthParticles"],
                )
            )
            cfg.merge(event_counter_cfg("n_truth_particle"))

    ########################################################################
    # Begin postprocessing
    ########################################################################

    if flags.Analysis.do_overlap_removal:
        log.info("Adding Overlap Removal sequence")
        if (
            flags.Analysis.write_large_R_Topo_jets
            and flags.Analysis.write_large_R_UFO_jets
        ):
            raise ValueError("Overlap removal only works with one Large R collection")
        overlapInputNames = {
            "electrons": containers["outputs"]["electrons"],
            "photons": containers["outputs"]["photons"],
            "jets": containers["outputs"]["reco4Jet"],
            "muons": containers["outputs"]["muons"],
        }

        if not is_daod_physlite:
            if flags.Analysis.write_large_R_Topo_jets:
                overlapInputNames["fatJets"] = containers["outputs"]["reco10Jet"]
            if flags.Analysis.write_large_R_UFO_jets:
                overlapInputNames["fatJets"] = containers["outputs"]["reco10UFOJet"]

        overlapOutputNames = {k: f"{v}_OR" for k, v in overlapInputNames.items()}

        cfg.merge(
            overlap_sequence_cfg(
                flags,
                datatype=datatype,
                inputnames=overlapInputNames,
                outputnames=overlapOutputNames,
                doFatJets=not is_daod_physlite,
                doMuons=flags.Analysis.do_muons,
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
                largejetkey=containers["outputs"]["reco10Jet"].replace(
                    "%SYS%", "NOSYS"
                ),
            )
        )
        cfg.merge(event_counter_cfg("n_merged"))

    return cfg
