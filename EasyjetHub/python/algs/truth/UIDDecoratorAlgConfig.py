from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def UIDDecoratorAlgCfg(flags):
    cfg = ComponentAccumulator()

    truthPartCollections = ["TruthBosonsWithDecayParticles",
                            "TruthBSMWithDecayParticles",
                            "TruthTop", "TruthBoson",
                            "TruthTaus", "TruthMuons", "TruthElectrons",
                            "TruthNeutrinos",
                            "TruthForwardProtons"
                            ]
    if not flags.Input.isPHYSLITE:
        truthPartCollections += ["TruthBottom", "TruthCharm",
                                 "TruthHFWithDecayParticles"]

    for coll in truthPartCollections:
        cfg.addEventAlgo(CompFactory.Easyjet.UIDDecoratorAlg(
            "UIDDecoratorAlg_" + coll,
            TruthParticleKey=coll))

    return cfg
