from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from EasyjetHub.steering.container_names import get_container_names


def btag_decor_cfg(flags):
    # Could make this configurable
    jetcoll = get_container_names(flags)["inputs"]["reco4PFlowJet"]
    if flags.Analysis.write_small_R_gn2_branches:
        cfg = ComponentAccumulator()
        cfg.addEventAlgo(
            CompFactory.Easyjet.BTaggingDecoratorAlg(
                f"BTagDecor_{jetcoll}",
                jetsIn=jetcoll,
                floatVars=[
                    "GN2v00_pb",
                    "GN2v00_pc",
                    "GN2v00_pu",
                    "DL1dv01_pb",
                    "DL1dv01_pc",
                    "DL1dv01_pu",
                    "DL1r_pb",
                    "DL1r_pc",
                    "DL1r_pu",
                ],
            )
        )

    return cfg
