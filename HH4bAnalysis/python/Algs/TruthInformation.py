from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TruthParticleInformationAlgCfg(
    flags,
    inputContainerName,
    outputContainerName,
):
    cfg = ComponentAccumulator()
    cfg.addEventAlgo(
        CompFactory.HH4B.TruthParticleInformationAlg(
            "TruthParticleInformationAlg",
            EventInfoKey="EventInfo",
            TruthParticleInformationKey=inputContainerName,
            TruthParticleInformationOutKey=outputContainerName,
        )
    )

    return cfg
