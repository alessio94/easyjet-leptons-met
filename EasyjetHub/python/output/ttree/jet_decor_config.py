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

        if flags.GeoModel.Run not in [LHCPeriod.Run2, LHCPeriod.Run3]:
            raise RuntimeError(
                f"Jet trigger matching not supported in LHCPeriod \
                    {flags.LHCPeriod.Run}. Please switch off \
                        doL1Matching and doHLTMatching.")

        kwargs.setdefault("doHLTMatching", flags.Analysis.Small_R_jet.doHLTMatching)
        kwargs.setdefault("doL1Matching", flags.Analysis.Small_R_jet.doL1Matching)
        kwargs.setdefault("triggerList", flags.Analysis.TriggerChainsDeco)

        from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
        kwargs.setdefault("TrigDecisionTool", cfg.getPrimaryAndMerge(
            TrigDecisionToolCfg(flags)))

        if flags.Analysis.Small_R_jet.doHLTMatching:
            # if flags.GeoModel.Run is LHCPeriod.Run2:
            # Disabled for now
            if False:
                kwargs.setdefault("useEmulationTool", True)

                from TrigBtagEmulationTool.TrigBtagEmulationToolConfig import (
                    TrigBtagEmulationToolCfg)
                kwargs.setdefault("trigBtagEmulationTool", cfg.popToolsAndMerge(
                    TrigBtagEmulationToolCfg(
                        flags,
                        toBeEmulatedTriggers=list(flags.Analysis.TriggerChainsDeco))))

    cfg.addEventAlgo(
        CompFactory.Easyjet.JetDecoratorAlg(
            f"JetDecor_{jetcoll}",
            jetsIn=jetcoll,
            **kwargs
        )
    )

    return cfg
