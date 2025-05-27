from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from EasyjetHub.steering.sample_metadata import STXS_info
from EasyjetHub.algs.truthweighttools_config import TruthWeightToolsCfg


def truth_particle_info_cfg(
    flags,
):
    cfg = ComponentAccumulator()
    if flags.Input.MCChannelNumber in flags.Analysis.Truth.DSID_nWLep_samples:
        cfg.addEventAlgo(
            CompFactory.Easyjet.TruthWBosonInformationAlg(
                "TruthWBosonInformationAlg",
                EventInfoKey="EventInfo",
                TruthBosonParticleInKey="TruthBosonsWithDecayParticles",
            )
        )

    if flags.Analysis.Truth.do_STXS:
        ProdMode, has_STXS, has_STXS_unc = STXS_info(flags.Input.MCChannelNumber)
        if has_STXS:
            twTools = None
            if has_STXS_unc:
                RequireFinite, WeightCutOff = 0, 10000000
                if ProdMode == "ggF":
                    RequireFinite, WeightCutOff = 1, 100
                twTools = cfg.popToolsAndMerge(
                    TruthWeightToolsCfg(
                        flags,
                        ProdMode=ProdMode,
                        RequireFinite=RequireFinite,
                        WeightCutOff=WeightCutOff
                    )
                )

            cfg.addEventAlgo(
                CompFactory.Easyjet.STXSAlg(
                    twTools=twTools
                )
            )

    cfg.addEventAlgo(
        CompFactory.Easyjet.TruthParticleInformationAlg(
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
            decayModes=flags.Analysis.Truth.decayModes,
            nHiggses=flags.Analysis.Truth.nHiggses,
        )
    )

    return cfg
