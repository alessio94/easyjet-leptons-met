from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def truth_particle_info_cfg(
    flags,
    sm_containerinkey,
    bsm_containerinkey,
    containeroutkey,
):
    cfg = ComponentAccumulator()
    cfg.addEventAlgo(
        CompFactory.HH4B.TruthParticleInformationAlg(
            "TruthParticleInformationAlg",
            EventInfoKey="EventInfo",
            TruthParticleSMInKey=sm_containerinkey,
            TruthParticleBSMInKey=bsm_containerinkey,
            TruthParticleInformationOutKey=containeroutkey,
        )
    )

    return cfg
