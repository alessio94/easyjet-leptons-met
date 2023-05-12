from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from HH4bAnalysis.config.container_names import get_container_names
from HH4bAnalysis.config.event_counter_config import event_counter_cfg
from HH4bAnalysis.config.boosted_config import boosted_cfg
from HH4bAnalysis.config.resolved_config import resolved_cfg


def dihiggs_analysis_cfg(
    flags,
):
    cfg = ComponentAccumulator()

    containers = get_container_names(flags)

    if flags.Analysis.do_resolved_dihiggs_analysis:
        cfg.merge(
            resolved_cfg(
                flags,
                smalljetkey=containers["outputs"]["reco4Jet"].replace("%SYS%", "NOSYS"),
            )
        )
        cfg.merge(event_counter_cfg("n_resolved"))

    if flags.Analysis.do_boosted_dihiggs_analysis:
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
