from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from EasyjetHub.steering.utils.log_helper import log
from EasyjetHub.algs.truth.jet_parent_decorator_config import jet_parent_decorator_cfg
from EasyjetHub.algs.truth_particle_info_config import truth_particle_info_cfg


def truth_info_cfg(
    flags,
):
    cfg = ComponentAccumulator()

    # truth record seems to be broken in physlite
    if flags.Analysis.do_small_R_jets and not flags.Input.isPHYSLITE:
        cfg.merge(jet_parent_decorator_cfg(
            flags,
            jet_collection=flags.Analysis.container_names.input[
                flags.Analysis.small_R_jet.jet_type
            ],
            name_prefix="smallR",
            match_dr=0.3
        ))

    if flags.Analysis.do_large_R_UFO_jets and not flags.Input.isPHYSLITE:
        cfg.merge(jet_parent_decorator_cfg(
            flags,
            jet_collection=flags.Analysis.container_names.input.reco10UFOJet,
            name_prefix="largeR",
            match_dr=1.0,
        ))

    if flags.Analysis.do_large_R_Topo_jets and not flags.Input.isPHYSLITE:
        cfg.merge(jet_parent_decorator_cfg(
            flags,
            jet_collection=flags.Analysis.container_names.input.reco10TopoJet,
            name_prefix="largeRTopo",
            match_dr=1.0,
        ))

    log.info("Adding truth particle info seq")
    cfg.merge(
        truth_particle_info_cfg(flags)
    )

    return cfg
