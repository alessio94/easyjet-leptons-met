from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def muon_parent_decorator_cfg(
        flags,
        muon_collection,
        name_prefix="",
        match_dr=0.3,
):
    sm_particles = flags.Analysis.container_names.input.truthSMParticles

    # add in leptonic tau decay via a W to neutrino and other lepton+nu
    muon_decay_pdgIds = [24, 11, 12, 13, 14, 15, 16]
    muon_decay_pdgIds_all = [-x for x in muon_decay_pdgIds] + muon_decay_pdgIds

    muon_common = dict(
        targetContainer=muon_collection,
        cascades=[
            "TruthBoson",
            "TruthBosonsWithDecayParticles",
            "TruthTaus",
            "TruthMuons",
            "TruthElectrons",
            "TruthNeutrinos"
        ],
        cascadePdgIds=muon_decay_pdgIds_all,
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
            **muon_common,
        )
    )
    cfg.addEventAlgo(
        CompFactory.TruthParentDecoratorAlg(
            name=f"{name_prefix}ParentZDecorator",
            parents=sm_particles,
            decoratorPrefix="parentZ",
            parentPdgIds=[23],
            **muon_common
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
            targetContainer=muon_collection,
            matchDeltaR=match_dr
        )
    )
    return cfg
