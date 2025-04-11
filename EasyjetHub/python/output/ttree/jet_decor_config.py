from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod


def jet_decor_cfg(flags, **kwargs):
    cfg = ComponentAccumulator()

    jetcoll = flags.Analysis.container_names.input[
        flags.Analysis.Small_R_jet.jet_type
    ]

    kwargs.setdefault("isMC", flags.Input.isMC)

    if (flags.Analysis.Small_R_jet.doHLTMatching
            or flags.Analysis.Small_R_jet.doL1Matching):

        if flags.Analysis.Small_R_jet.doHLTMatching:
            if flags.GeoModel.Run is LHCPeriod.Run2:
                print("WARNING! HLT jet matching is not yet supported for Run 2. ")
                print("You should get in touch with the Trigger Core Software group ")
                print("to contribute to the necessary developments")
            else:
                kwargs.setdefault("doHLTMatching", True)

        kwargs.setdefault("doL1Matching", flags.Analysis.Small_R_jet.doL1Matching)
        kwargs.setdefault("triggerList", flags.Analysis.TriggerChainsDeco)
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
