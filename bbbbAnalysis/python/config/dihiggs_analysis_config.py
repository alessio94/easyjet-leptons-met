from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from EasyjetHub.algs.event_counter_config import event_counter_cfg
from bbbbAnalysis.config.boosted_config import boosted_cfg
from bbbbAnalysis.config.resolved_config import resolved_cfg
from bbbbAnalysis.config.boost_histograms import histograms_cfg


def dihiggs_analysis_cfg(
    flags,
):
    cfg = ComponentAccumulator()

    smalljetkey = flags.Analysis.container_names.output.reco4PFlowJet
    if flags.Analysis.do_resolved_dihiggs_analysis:
        cfg.merge(
            resolved_cfg(
                flags,
                smalljetkey=smalljetkey.replace("%SYS%","NOSYS"),
            )
        )
        cfg.merge(event_counter_cfg("n_resolved"))

    if flags.Analysis.do_boosted_dihiggs_analysis:
        lr10jets_sys = flags.Analysis.container_names.output.reco10TopoJet
        cfg.merge(
            boosted_cfg(
                flags,
                largejetkey=lr10jets_sys.replace("%SYS%", "NOSYS"),
            )
        )
        cfg.merge(event_counter_cfg("n_merged"))

    cfg.merge(histograms_cfg(flags))

    return cfg
