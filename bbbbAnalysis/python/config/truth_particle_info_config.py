from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def truth_particle_info_cfg(
    flags,
):

    cfg = ComponentAccumulator()
    cfg.addEventAlgo(
        CompFactory.HH4B.TruthParticleInformationAlg(
            "TruthParticleInformationAlg",
            EventInfoKey="EventInfo",
            TruthParticleSMInKey=(
                flags.Analysis.container_names.input.truthSMParticles
            ),
            TruthParticleBSMInKey=(
                flags.Analysis.container_names.input.truthBSMParticles
            ),
            TruthParticleInformationOutKey=(
                flags.Analysis.container_names.output.truthHHParticles
            ),
        )
    )

    return cfg
