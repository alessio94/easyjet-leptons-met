from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def HHbbttTriggerDecoratorCfg(flags, **kwargs):

    cfg = ComponentAccumulator()

    from EasyjetHub.algs.postprocessing.trigger_matching import TriggerMatchingToolCfg

    trigger_branches = [
        f"trigPassed_{c.replace('-', '_').replace('.', 'p')}"
        for c in flags.Analysis.TriggerChainsDeco
    ]

    kwargs.setdefault("isMC", flags.Input.isMC)
    kwargs.setdefault("muons", flags.Analysis.container_names.input.muons)
    kwargs.setdefault("electrons", flags.Analysis.container_names.input.electrons)
    kwargs.setdefault("taus", flags.Analysis.container_names.input.taus)
    kwargs.setdefault("triggerLists", trigger_branches)
    kwargs.setdefault("trigMatchingTool",
                      cfg.popToolsAndMerge(TriggerMatchingToolCfg(flags)))

    if flags.Analysis.do_bbtt_analysis:
        kwargs.setdefault("jets", flags.Analysis.container_names.input.reco4PFlowJet)
        cfg.addEventAlgo(
            CompFactory.HHBBTT.TriggerDecoratorAlg(
                "HHbbttTriggerDecoratorAlg",
                **kwargs
            )
        )

    if flags.Analysis.do_bbbbtt_analysis:
        cfg.addEventAlgo(
            CompFactory.HHHBBBBTT.TriggerDecoratorAlg(
                "HHHbbbbttTriggerDecoratorAlg",
                **kwargs
            )
        )

    return cfg


def tau_decor_cfg(flags, **kwargs):
    taucoll = flags.Analysis.container_names.input.taus

    cfg = ComponentAccumulator()

    cfg.addEventAlgo(
        CompFactory.Easyjet.TauDecoratorAlg(
            f"TauDecor_{taucoll}",
            isMC=flags.Input.isMC,
            tausIn=taucoll,
            **kwargs
        )
    )

    if flags.Analysis.do_bbtt_analysis or flags.Analysis.do_bbbbtt_analysis:
        cfg.merge(HHbbttTriggerDecoratorCfg(flags))

    return cfg
