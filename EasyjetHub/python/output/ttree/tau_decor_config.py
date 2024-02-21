from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def tau_decor_cfg(flags, **kwargs):
    # Could make this configurable
    muoncoll = flags.Analysis.container_names.input.muons
    elecoll = flags.Analysis.container_names.input.electrons
    taucoll = flags.Analysis.container_names.input.taus

    cfg = ComponentAccumulator()
    cfg.addEventAlgo(
        CompFactory.Easyjet.TauDecoratorAlg(
            f"TagDecor_{taucoll}",
            isMC=flags.Input.isMC,
            tauIDWP=flags.Analysis.Tau.ID,
            muonsIn=muoncoll,
            elesIn=elecoll,
            tausIn=taucoll,
            **kwargs
        )
    )

    return cfg
