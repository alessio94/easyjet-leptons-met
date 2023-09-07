from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def tau_decor_cfg(flags, **kwargs):
    # Could make this configurable
    taucoll = flags.Analysis.container_names.input.taus

    kwargs.setdefault("tauIDWP", "Loose")
    kwargs.setdefault("channel", flags.Analysis.channel)

    cfg = ComponentAccumulator()
    cfg.addEventAlgo(
        CompFactory.Easyjet.TauDecoratorAlg(
            f"TagDecor_{taucoll}",
            tausIn=taucoll,
            **kwargs
        )
    )

    return cfg
