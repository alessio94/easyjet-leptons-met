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
                    datatype=datatype,
                    prwfiles=prw_files,
                    lumicalcfiles=lumicalc_files,
                )
            )
            log.info("Adding generator analysis sequence")
            # Adds variable to EventInfo if for generator weight, for example:
            # EventInfo.generatorWeight_%SYS%
            cfg.merge(generator_sequence_cfg(flags, datatype))

        if flags.Analysis.do_electrons:
            log.info("Adding electron seq")
            cfg.merge(
                electron_sequence_cfg(
                    flags,
                    datatype=datatype,
                    inname=containers["inputs"]["electrons"],
                    outname=containers["outputs"]["electrons"],
                )
            )

        if flags.Analysis.do_photons:
            log.info("Adding photon seq")
            cfg.merge(
                photon_sequence_cfg(
                    flags,
                    datatype=datatype,
                    inname=containers["inputs"]["photons"],
                    outname=containers["outputs"]["photons"],
                )
            )

        if flags.Analysis.do_muons:
            log.info("Adding muon seq")
            cfg.merge(
                muon_sequence_cfg(
                    flags,
                    datatype=datatype,
                    inname=containers["inputs"]["muons"],
                    outname=containers["outputs"]["muons"],
                )
            )

        muoncont = containers["outputs"]["muons"]
        if not flags.Analysis.do_muons:
            muoncont = containers["inputs"]["muons"]

        if flags.Analysis.do_small_R_jets:
            log.info("Adding small-R jet seq")
            cfg.merge(
                jet_sequence_cfg(
                    flags,
                    datatype=datatype,
                    inname=containers["inputs"]["reco4Jet"],
                    outname=containers["outputs"]["reco4Jet"],
                    # Need muons for b-jet pt correction
                    muonname=muoncont,
                    do_bjet_ptcalib=flags.Analysis.do_muons,
                    is_daod_physlite=is_daod_physlite,
                )
            )
            cfg.merge(event_counter_cfg("n_small_r"))

        if flags.Analysis.do_large_R_Topo_jets:
            log.info("Adding large-R jet seq")
            cfg.merge(
                lr_jet_sequence_cfg(
                    flags,
                    datatype=datatype,
                    inname=containers["inputs"]["reco10Jet"],
                    outname=containers["outputs"]["reco10Jet"],
                )
            )
            cfg.merge(event_counter_cfg("n_large_r"))

        if flags.Analysis.do_large_R_UFO_jets:
            log.info("Adding UFO large-R jet seq")
            cfg.merge(
                lr_ufo_jet_sequence_cfg(
                    flags,
                    datatype=datatype,
                    inname=containers["inputs"]["reco10UFOJet"],
                    outname=containers["outputs"]["reco10UFOJet"],
                )
            )
            cfg.merge(event_counter_cfg("n_large_r_ufo"))

        if flags.Analysis.do_VR_jets:
            log.info("Adding VR jet seq")
            cfg.merge(
                vr_jet_sequence_cfg(
                    flags,
                    datatype=datatype,
                    inname=containers["inputs"]["vrJet"],
                    outname=containers["outputs"]["vrJet"],
                )
            )
            cfg.merge(event_counter_cfg("n_vr"))

            if flags.Analysis.do_large_R_Topo_jets:
                cfg.merge(
                    lr_jet_ghost_vr_jet_association_cfg(
                        flags,
                        inlrjet_name=containers["outputs"]["reco10Jet"].replace(
                            "%SYS%", "NOSYS"
                        ),
                    )
                )

            if flags.Analysis.do_large_R_UFO_jets:
                cfg.merge(
                    lr_ufo_jet_ghost_vr_jet_association_cfg(
                        flags,
                        inlrufojet_name=containers["outputs"][
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
        overlapInputNames = {}
        for objtype in ["muons", "electrons", "photons"]:
            if flags(f"Analysis.do_{objtype}"):
                overlapInputNames[objtype] = containers["outputs"][objtype]

        if flags.Analysis.do_small_R_jets:
            overlapInputNames["jets"] = containers["outputs"]["reco4Jet"]

        do_fatJet_OR = False
        if flags.Analysis.write_large_R_Topo_jets:
            overlapInputNames["fatJets"] = containers["outputs"]["reco10Jet"]
            do_fatJet_OR = True
        if flags.Analysis.write_large_R_UFO_jets:
            overlapInputNames["fatJets"] = containers["outputs"]["reco10UFOJet"]
            do_fatJet_OR = True

        overlapOutputNames = {k: f"{v}_OR" for k, v in overlapInputNames.items()}

        cfg.merge(
            overlap_sequence_cfg(
                flags,
                datatype=datatype,
                inputnames=overlapInputNames,
                outputnames=overlapOutputNames,
                doJets=flags.Analysis.do_small_R_jets,
                doFatJets=do_fatJet_OR,
                doMuons=flags.Analysis.do_muons,
                doElectrons=flags.Analysis.do_electrons,
                doPhotons=flags.Analysis.do_photons,
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
