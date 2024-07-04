from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


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
        from bbttAnalysis.AntiTauDecoratorConfig import (
            HHbbttTriggerDecoratorCfg)
        cfg.merge(HHbbttTriggerDecoratorCfg(flags))

    return cfg
