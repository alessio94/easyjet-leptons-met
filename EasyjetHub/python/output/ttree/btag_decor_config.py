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
    gn2v00_valid_ptag = (get_valid_ami_tag(split_tags, "p", "p5855")
                         and not get_valid_ami_tag(split_tags, "p", "p6187"))
    if gn2v00_valid_ptag:
        btag_vars += [
            "GN2v00_pb",
            "GN2v00_pc",
            "GN2v00_pu"
        ]

    gn2v01_valid_ptag = (
        (get_valid_ami_tag(split_tags, "p", "p6026") and not flags.Input.isPHYSLITE)
        or get_valid_ami_tag(split_tags, "p", "p6266"))
    if gn2v01_valid_ptag:
        btag_vars += [
            "GN2v01_pb",
            "GN2v01_pc",
            "GN2v01_pu",
            "GN2v01_ptau",
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
