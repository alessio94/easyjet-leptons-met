from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def tau_decor_cfg(flags, **kwargs):
    # Could make this configurable
    muoncoll = flags.Analysis.container_names.input.muons
    elecoll = flags.Analysis.container_names.input.electrons
    taucoll = flags.Analysis.container_names.input.taus

    cfg = ComponentAccumulator()

    # Run early trigger matching for bbtt analysis
    # Needed for anti-tau decoration
    if flags.Analysis.do_bbtt_analysis:
        from bbttAnalysis.TriggerDecoratorConfig import (
            HHbbttTriggerDecoratorCfg)
        cfg.merge(HHbbttTriggerDecoratorCfg(flags))

    cfg.addEventAlgo(
        CompFactory.Easyjet.TauDecoratorAlg(
            f"TauDecor_{taucoll}",
            isMC=flags.Input.isMC,
            Years=flags.Analysis.Years,
            tauIDWP=flags.Analysis.Tau.ID,
            doAntiTauDecor=flags.Analysis.do_bbtt_analysis,
            muonsIn=muoncoll,
            elesIn=elecoll,
            tausIn=taucoll,
            **kwargs
        )
    )

    return cfg
