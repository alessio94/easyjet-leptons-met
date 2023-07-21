from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from EasyjetHub.steering.container_names import get_container_names
from EasyjetHub.steering.utils.log_helper import log
from EasyjetHub.algs.event_counter_config import event_counter_cfg
from EasyjetHub.algs.truth.jet_parent_decorator_config import jet_parent_decorator_cfg
from bbbbAnalysis.config.truth_particle_info_config import truth_particle_info_cfg


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

    if flags.Analysis.do_large_R_Topo_jets and not flags.Input.isPHYSLITE:
        cfg.merge(jet_parent_decorator_cfg(
            flags,
            jet_collection=containers["inputs"]["reco10TopoJet"],
            name_prefix="largeRTopo",
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
