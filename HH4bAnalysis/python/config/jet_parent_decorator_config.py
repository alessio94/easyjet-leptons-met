from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from HH4bAnalysis.config.container_names import get_container_names


def jet_parent_decorator_cfg(flags, name_prefix=""):
    containers = get_container_names(flags)
    jet_collection = containers["inputs"]["reco4Jet"]
    sm_particles = containers["inputs"]["truthSMParticles"]
    bsm_particles = containers["inputs"]["truthBSMParticles"]
    common = dict(
        targetContainer=jet_collection,
        cascades=["TruthBottom", "TruthCharm", "TruthHFWithDecayParticles"],
    )
    cfg = ComponentAccumulator()
    cfg.addEventAlgo(
        CompFactory.TruthParentDecoratorAlg(
            name=f"{name_prefix}JetParentHiggsDecorator",
            parents=sm_particles,
            decoratorPrefix="parentHiggs",
            parentPdgIds=[25],
            **common,
        )
    )
    cfg.addEventAlgo(
        CompFactory.TruthParentDecoratorAlg(
            name=f"{name_prefix}JetParentScalarDecorator",
            parents=bsm_particles,
            decoratorPrefix="parentScalar",
            parentPdgIds=[35],
            **common,
        )
    )
    # we want to label any jet coming via a W to a quark, tau, or
    # electron
    top_decay_pdgids = [24, 5, 4, 3, 2, 1, 11, 15]
    top_decay_pdgids_all = [-x for x in top_decay_pdgids] + top_decay_pdgids
    cfg.addEventAlgo(
        CompFactory.TruthParentDecoratorAlg(
            name=f"{name_prefix}JetParentTopDecorator",
            parents="TruthTop",
            decoratorPrefix="parentTop",
            parentPdgIds=[-6, 6],
            cascadePdgIds=top_decay_pdgids_all,
            cascades=[
                "TruthBoson",
                "TruthBosonsWithDecayParticles",
                "TruthBottom",
                "TruthCharm",
                "TruthElectrons",
                "TruthForwardProtons",
                "TruthHFWithDecayParticles",
                "TruthPileupParticles",
                "TruthTaus",
            ],
            targetContainer=jet_collection,
        )
    )
    return cfg
