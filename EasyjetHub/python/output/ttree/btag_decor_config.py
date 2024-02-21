from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from EasyjetHub.steering.sample_metadata import get_valid_ami_tag


def btag_decor_cfg(flags):
    # Could make this configurable
    jetcoll = flags.Analysis.container_names.input[flags.Analysis.small_R_jet.jet_type]
    cfg = ComponentAccumulator()
    btag_vars = [
        "DL1dv01_pb",
        "DL1dv01_pc",
        "DL1dv01_pu",
    ]

    split_tags = flags.Input.AMITag.split("_")
    gn2_valid_ptag = get_valid_ami_tag(split_tags, "p", "p5855")
    if gn2_valid_ptag:
        btag_vars += [
            "GN2v00_pb",
            "GN2v00_pc",
            "GN2v00_pu"
        ]

    cfg.addEventAlgo(
        CompFactory.Easyjet.BTaggingDecoratorAlg(
            f"BTagDecor_{jetcoll}",
            jetsIn=jetcoll,
            floatVars=btag_vars,
            expVars=flags.Analysis.small_R_jet.doBtagExpVars
        )
    )

    return cfg
