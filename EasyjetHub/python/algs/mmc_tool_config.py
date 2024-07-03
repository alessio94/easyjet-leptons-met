from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MissingMassToolV2Cfg(flags, **kwargs):
    ca = ComponentAccumulator()

    kwargs.setdefault("Decorate", False)
    kwargs.setdefault("FloatStoppingCrit", False)
    kwargs.setdefault("CalibSet", "2019")
    kwargs.setdefault("BeamEnergy", flags.Beam.Energy)

    ca.setPrivateTools(CompFactory.DiTauMassTools.MissingMassToolV2(**kwargs))
    return ca
