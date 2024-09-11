from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TruthClassificationToolConfig(flags, name="TruthClassificationTool", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("separateChargeFlipElectrons", True)
    kwargs.setdefault("separateChargeFlipMuons", True)
    result.setPrivateTools(CompFactory.TruthClassificationTool(name, **kwargs))
    return result
