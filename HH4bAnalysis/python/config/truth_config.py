from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from HH4bAnalysis.config.container_names import get_container_names
from HH4bAnalysis.utils.log_helper import log
from HH4bAnalysis.config.event_counter_config import event_counter_cfg
from HH4bAnalysis.config.jet_parent_decorator_config import jet_parent_decorator_cfg


def truth_info_cfg(
    flags,
):
    cfg = ComponentAccumulator()

    containers = get_container_names(flags)

    # truth record seems to be broken in physlite
    if flags.Analysis.do_small_R_jets and not flags.Input.isPHYSLITE:
        cfg.merge(jet_parent_decorator_cfg(
            flags,
            jet_collection=containers["inputs"]["reco4PFlowJet"],
            name_prefix="smallR",
            match_dr=0.3
        ))

    if flags.Analysis.do_large_R_UFO_jets and not flags.Input.isPHYSLITE:
        cfg.merge(jet_parent_decorator_cfg(
            flags,
            jet_collection=containers["inputs"]["reco10UFOJet"],
            name_prefix="largeR",
            match_dr=1.0,
        ))

    log.info("Adding truth particle info seq")
    cfg.merge(
        truth_particle_info_cfg(
            flags,
            containers,
        )
    )
    cfg.merge(event_counter_cfg("n_truth_particle"))

    return cfg


def truth_particle_info_cfg(
    flags,
    containers,
):

    cfg = ComponentAccumulator()
    cfg.addEventAlgo(
        CompFactory.HH4B.TruthParticleInformationAlg(
            "TruthParticleInformationAlg",
            EventInfoKey="EventInfo",
            TruthParticleSMInKey=containers["inputs"]["truthSMParticles"],
            TruthParticleBSMInKey=containers["inputs"]["truthBSMParticles"],
            TruthParticleInformationOutKey=containers["outputs"]["truthHHParticles"],
        )
    )

    return cfg
