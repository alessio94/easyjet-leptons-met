from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from EasyjetHub.steering.container_names import get_container_names
from EasyjetHub.algs.event_counter_config import event_counter_cfg
from bbbbAnalysis.config.boosted_config import boosted_cfg
from bbbbAnalysis.config.resolved_config import resolved_cfg


def dihiggs_analysis_cfg(
    flags,
):
    cfg = ComponentAccumulator()

    out_keys = get_container_names(flags)["outputs"]
    smalljetkey = out_keys["reco4PFlowJet"]
    if flags.Analysis.do_resolved_dihiggs_analysis:
        cfg.merge(
            resolved_cfg(
                flags,
                smalljetkey=smalljetkey.replace("%SYS%","NOSYS"),
            )
        )
        cfg.merge(event_counter_cfg("n_resolved"))

    if flags.Analysis.do_boosted_dihiggs_analysis:
        cfg.merge(
            boosted_cfg(
                flags,
                largejetkey=out_keys["reco10TopoJet"].replace("%SYS%", "NOSYS"),
            )
        )
        cfg.merge(event_counter_cfg("n_merged"))

    if out_path := flags.Analysis.output_hists:
        output = CompFactory.H5FileSvc(path=str(out_path))
        cfg.addService(output)
        cfg.addEventAlgo(
            CompFactory.HH4B.JetBoostHistogramsAlg(
                name="JetBoostHistogramsAlg",
                jetsIn=smalljetkey,
                output=output,
            )
        )

    return cfg
