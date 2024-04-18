from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def jet_decor_cfg(flags, **kwargs):
    jetcoll = flags.Analysis.container_names.input[
        flags.Analysis.small_R_jet.jet_type
    ]

    cfg = ComponentAccumulator()
    cfg.addEventAlgo(
        CompFactory.Easyjet.JetDecoratorAlg(
            f"JetDecor_{jetcoll}",
            jetsIn=jetcoll,
            **kwargs
        )
    )

    return cfg
