from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def jet_decor_cfg(flags, **kwargs):
    cfg = ComponentAccumulator()

    jetcoll = flags.Analysis.container_names.input[
        flags.Analysis.Small_R_jet.jet_type
    ]

    kwargs.setdefault("isMC", flags.Input.isMC)

    if (flags.Analysis.Small_R_jet.doHLTMatching
            or flags.Analysis.Small_R_jet.doL1Matching):
        kwargs.setdefault("doHLTMatching", flags.Analysis.Small_R_jet.doHLTMatching)
        kwargs.setdefault("doL1Matching", flags.Analysis.Small_R_jet.doL1Matching)
        kwargs.setdefault("triggerList", flags.Analysis.TriggerChains)
        from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
        kwargs.setdefault("TrigDecisionTool", cfg.getPrimaryAndMerge(
            TrigDecisionToolCfg(flags)))

    cfg.addEventAlgo(
        CompFactory.Easyjet.JetDecoratorAlg(
            f"JetDecor_{jetcoll}",
            jetsIn=jetcoll,
            **kwargs
        )
    )

    return cfg
