from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def btag_decor_cfg(flags):
    # Could make this configurable
    jetcoll = flags.Analysis.container_names.input[flags.Analysis.small_R_jet.jet_type]
    cfg = ComponentAccumulator()
    btag_vars = [
        "DL1dv01_pb",
        "DL1dv01_pc",
        "DL1dv01_pu",
        "GN2v00_pb",
        "GN2v00_pc",
        "GN2v00_pu"
    ]

    cfg.addEventAlgo(
        CompFactory.Easyjet.BTaggingDecoratorAlg(
            f"BTagDecor_{jetcoll}",
            jetsIn=jetcoll,
            floatVars=btag_vars,
        )
    )

    return cfg
