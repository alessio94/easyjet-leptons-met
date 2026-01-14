# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod


def TriggerMatchingToolCfg(flags, name="TriggerMatchingTool", **kwargs):
    result = ComponentAccumulator()

    if flags.GeoModel.Run == LHCPeriod.Run3:
        from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
        kwargs.setdefault("TrigDecisionTool", result.getPrimaryAndMerge(
            TrigDecisionToolCfg(flags)))
        matching_tool = CompFactory.Trig.R3MatchingTool(name, **kwargs)
    else:
        if flags.Input.isPHYSLITE:
            kwargs.setdefault("InputPrefix", "AnalysisTrigMatch_")
        matching_tool = CompFactory.Trig.MatchFromCompositeTool(name, **kwargs)

    result.setPrivateTools(matching_tool)
    return result
