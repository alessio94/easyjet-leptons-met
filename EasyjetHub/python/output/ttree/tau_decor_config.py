from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from EasyjetHub.steering.container_names import get_container_names


def tau_decor_cfg(flags):
    # Could make this configurable
    taucoll = get_container_names(flags)["inputs"]["taus"]

    cfg = ComponentAccumulator()
    cfg.addEventAlgo(
        CompFactory.Easyjet.TauDecoratorAlg(
            f"TagDecor_{taucoll}",
            tausIn=taucoll
        )
    )

    return cfg
