from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from EasyjetHub.algs.event_counter_config import event_counter_cfg
from bbbbAnalysis.config.boosted_config import boosted_cfg
from bbbbAnalysis.config.resolved_config import resolved_cfg


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
