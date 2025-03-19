from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def HHbbttTriggerDecoratorCfg(flags, **kwargs):

    cfg = ComponentAccumulator()

    from EasyjetHub.algs.postprocessing.trigger_matching import TriggerMatchingToolCfg

    trigger_branches = [
        f"trigPassed_{c.replace('-', '_').replace('.', 'p')}"
        for c in flags.Analysis.TriggerChainsDeco
    ]

    cfg.addEventAlgo(
        CompFactory.HHBBTT.TriggerDecoratorAlg(
            "HHbbttTriggerDecoratorAlg",
            isMC=flags.Input.isMC,
            muons=flags.Analysis.container_names.input.muons,
            electrons=flags.Analysis.container_names.input.electrons,
            taus=flags.Analysis.container_names.input.taus,
            jets=flags.Analysis.container_names.input.reco4PFlowJet,
            triggerLists=trigger_branches,
            trigMatchingTool=cfg.popToolsAndMerge(TriggerMatchingToolCfg(flags)),
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

    if flags.Analysis.do_bbtt_analysis:
        cfg.merge(HHbbttTriggerDecoratorCfg(flags))

    return cfg
