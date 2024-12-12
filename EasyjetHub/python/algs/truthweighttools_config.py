from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TruthWeightToolsCfg(flags, **kwargs):
    ca = ComponentAccumulator()

    ca.setPrivateTools(CompFactory.TruthWeightTools.HiggsWeightTool(**kwargs))
    return ca
