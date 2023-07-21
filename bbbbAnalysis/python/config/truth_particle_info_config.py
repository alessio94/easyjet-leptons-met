from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def truth_particle_info_cfg(
    flags,
    containers,
):

    cfg = ComponentAccumulator()
    cfg.addEventAlgo(
        CompFactory.HH4B.TruthParticleInformationAlg(
            "TruthParticleInformationAlg",
            EventInfoKey="EventInfo",
            TruthParticleSMInKey=containers["inputs"]["truthSMParticles"],
            TruthParticleBSMInKey=containers["inputs"]["truthBSMParticles"],
            TruthParticleInformationOutKey=containers["outputs"]["truthHHParticles"],
        )
    )

    return cfg
