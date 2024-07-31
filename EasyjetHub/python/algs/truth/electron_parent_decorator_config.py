from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def electron_parent_decorator_cfg(
        flags,
        electron_collection,
        name_prefix="",
        match_dr=0.3,
):
    sm_particles = flags.Analysis.container_names.input.truthSMParticles

    # add in leptonic tau decay via a W to neutrino and other lepton+nu
    el_decay_pdgIds = [24, 11, 12, 13, 14, 15, 16]
    electron_decay_pdgIds_all = [-x for x in el_decay_pdgIds] + el_decay_pdgIds

    electron_common = dict(
        targetContainer=electron_collection,
        cascades=[
            "TruthBoson",
            "TruthBosonsWithDecayParticles",
            "TruthTaus",
            "TruthMuons",
            "TruthElectrons",
            "TruthNeutrinos"
        ],
        cascadePdgIds=electron_decay_pdgIds_all,
        addBsToCascade=False,
        addCsToCascade=False,
        vetoSoftLeptonCascade=True,
        vetoSoftCharmCascade=True,
        matchDeltaR=match_dr,
    )

    cfg = ComponentAccumulator()
    cfg.addEventAlgo(
        CompFactory.TruthParentDecoratorAlg(
            name=f"{name_prefix}ParentHiggsDecorator",
            parents=sm_particles,
            decoratorPrefix="parentHiggs",
            parentPdgIds=[25],
            **electron_common,
        )
    )
    cfg.addEventAlgo(
        CompFactory.TruthParentDecoratorAlg(
            name=f"{name_prefix}ParentZDecorator",
            parents=sm_particles,
            decoratorPrefix="parentZ",
            parentPdgIds=[23],
            **electron_common
        )
    )
    # we want to label any lepton or neutrino coming via a W
    top_decay_pdgids = [24, 11, 12, 13, 14, 15, 16]
    top_decay_pdgids_all = [-x for x in top_decay_pdgids] + top_decay_pdgids
    cfg.addEventAlgo(
        CompFactory.TruthParentDecoratorAlg(
            name=f"{name_prefix}ParentTopDecorator",
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
                "TruthTaus",
            ],
            countChildrenInCascadeWithPdgIds={
                "nTopToWChildren": [-24, 24],
            },
            targetContainer=electron_collection,
            matchDeltaR=match_dr
        )
    )
    return cfg
