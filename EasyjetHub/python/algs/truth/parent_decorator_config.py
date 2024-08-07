from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TruthParentDecoratorCfg(flags, name="TruthParentDecoratorAlg", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("parents", flags.Analysis.container_names.input.truthSMParticles)
    kwargs.setdefault("vetoSoftLeptonCascade", True)
    kwargs.setdefault("vetoSoftCharmCascade", True)
    kwargs.setdefault("matchDeltaR", 0.3)

    # Jets
    if "AntiKt" in kwargs.get("targetContainer", ""):
        kwargs.setdefault("cascades", ["TruthBottom", "TruthCharm",
                                       "TruthHFWithDecayParticles"])
        kwargs.setdefault("cascadePdgIds", [-5, 5])

        kwargs.setdefault("addBsToCascade", True)
        kwargs.setdefault("addCsToCascade", True)
    # Other objects
    else:
        kwargs.setdefault("cascades", ["TruthBoson", "TruthBosonsWithDecayParticles",
                                       "TruthTaus", "TruthMuons", "TruthElectrons",
                                       "TruthNeutrinos"])
        # add in leptonic tau decay via a W to neutrino and other lepton+nu
        decay_pdgIds = [24, 11, 12, 13, 14, 15, 16]
        kwargs.setdefault("cascadePdgIds", [-x for x in decay_pdgIds] + decay_pdgIds)

    cfg.addEventAlgo(CompFactory.TruthParentDecoratorAlg(name, **kwargs))
    return cfg


def HiggsParentDecoratorCfg(flags, name="HiggsParentDecoratorAlg", **kwargs):
    kwargs.setdefault("decoratorPrefix", "parentHiggs")
    kwargs.setdefault("parentPdgIds", [25])
    return TruthParentDecoratorCfg(flags, name, **kwargs)


def ZParentDecoratorCfg(flags, name="ZParentDecoratorAlg", **kwargs):
    kwargs.setdefault("decoratorPrefix", "parentZ")
    kwargs.setdefault("parentPdgIds", [23])
    return TruthParentDecoratorCfg(flags, name, **kwargs)


def ScalarParentDecoratorCfg(flags, name="ScalarParentDecoratorAlg", **kwargs):
    kwargs.setdefault("parents", flags.Analysis.container_names.input.truthBSMParticles)
    kwargs.setdefault("decoratorPrefix", "parentScalar")
    kwargs.setdefault("parentPdgIds", [35])
    return TruthParentDecoratorCfg(flags, name, **kwargs)


def TopParentDecoratorCfg(flags, name="TopParentDecoratorAlg", **kwargs):
    kwargs.setdefault("parents", "TruthTop")
    kwargs.setdefault("decoratorPrefix", "parentTop")
    kwargs.setdefault("parentPdgIds", [-6, 6])

    kwargs.setdefault("cascades", ["TruthBoson", "TruthBosonsWithDecayParticles",
                                   "TruthBottom", "TruthCharm",
                                   "TruthTaus", "TruthElectrons",
                                   "TruthForwardProtons", "TruthHFWithDecayParticles"])

    # Jets
    if "AntiKt" in kwargs.get("targetContainer", ""):
        # we want to label any jet coming via a W to a quark, tau, or electron
        decay_pdgids = [24, 5, 4, 3, 2, 1, 11, 15]
        kwargs.setdefault("cascadePdgIds", [-x for x in decay_pdgids] + decay_pdgids)

        kwargs.setdefault("countChildrenInCascadeWithPdgIds",
                          {"nTopToWChildren": [-24, 24],
                           "nTopToBChildren": [-5, 5]})
    else:
        kwargs.setdefault("countChildrenInCascadeWithPdgIds",
                          {"nTopToWChildren": [-24, 24]})
        kwargs.setdefault("vetoSoftLeptonCascade", False)
        kwargs.setdefault("vetoSoftCharmCascade", False)

    return TruthParentDecoratorCfg(flags, name, **kwargs)


def parent_decorator_cfg(flags, prefix="", **kwargs):
    cfg = ComponentAccumulator()
    cfg.merge(HiggsParentDecoratorCfg(
        flags, name=prefix + "HiggsParentDecoratorAlg", **kwargs))
    cfg.merge(ZParentDecoratorCfg(
        flags, name=prefix + "ZParentDecoratorAlg", **kwargs))
    cfg.merge(ScalarParentDecoratorCfg(
        flags, name=prefix + "ScalarParentDecoratorAlg", **kwargs))
    cfg.merge(TopParentDecoratorCfg(
        flags, name=prefix + "TopParentDecoratorAlg", **kwargs))
    return cfg
