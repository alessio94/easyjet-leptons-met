from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def tau_decor_cfg(flags):
    # Could make this configurable
    taucoll = flags.Analysis.container_names.input.taus

    cfg = ComponentAccumulator()
    cfg.addEventAlgo(
        CompFactory.Easyjet.TauDecoratorAlg(
            f"TagDecor_{taucoll}",
            tausIn=taucoll
        )
    )

    return cfg
