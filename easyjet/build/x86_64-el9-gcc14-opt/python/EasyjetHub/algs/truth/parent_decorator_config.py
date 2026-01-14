from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
import ParticleJetTools.ParentDecoratorConfig as pdc


def parent_decorator_cfg(flags, prefix="", **kwargs):
    cfg = ComponentAccumulator()
    cfg.merge(pdc.HiggsParentDecoratorCfg(
        flags, name=prefix + "HiggsParentDecoratorAlg", **kwargs))
    cfg.merge(pdc.ZParentDecoratorCfg(
        flags, name=prefix + "ZParentDecoratorAlg", **kwargs))
    cfg.merge(pdc.ScalarParentDecoratorCfg(
        flags, name=prefix + "ScalarParentDecoratorAlg", **kwargs))
    cfg.merge(pdc.TopParentDecoratorCfg(
        flags, name=prefix + "TopParentDecoratorAlg", **kwargs))
    return cfg
