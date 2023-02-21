from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TruthParticleInformationAlgCfg(
    flags,
    inputSMContainerName,
    inputBSMContainerName,
    outputContainerName,
):
    cfg = ComponentAccumulator()
    cfg.addEventAlgo(
        CompFactory.HH4B.TruthParticleInformationAlg(
            "TruthParticleInformationAlg",
            EventInfoKey="EventInfo",
            TruthParticleSMInKey=inputSMContainerName,
            TruthParticleBSMInKey=inputBSMContainerName,
            TruthParticleInformationOutKey=outputContainerName,
        )
    )

    return cfg
