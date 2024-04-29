# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def HHbbttTriggerDecoratorCfg(flags, **kwargs):

    cfg = ComponentAccumulator()

    from EasyjetHub.algs.postprocessing.trigger_matching import TriggerMatchingToolCfg
    from AthenaConfiguration.Enums import LHCPeriod

    # Selection
    trigger_branches = [
        f"trigPassed_{c.replace('-', '_').replace('.', 'p')}"
        for c in flags.Analysis.TriggerChains
    ]

    cfg.addEventAlgo(
        CompFactory.HHBBTT.TriggerDecoratorAlg(
            "HHbbttTriggerDecoratorAlg",
            isMC=flags.Input.isMC,
            muons=flags.Analysis.container_names.input.muons,
            electrons=flags.Analysis.container_names.input.electrons,
            taus=flags.Analysis.container_names.input.taus,
            triggerLists=trigger_branches,
            trigMatchingTool=cfg.popToolsAndMerge(TriggerMatchingToolCfg(flags)),
            # Not available in current Run2 PHYSLITE
            diTauTrigMatch=not (flags.Input.isPHYSLITE and \
                                flags.GeoModel.Run == LHCPeriod.Run2),
            **kwargs
        )
    )

    return cfg


def HHbbttAntiTauDecoratorCfg(flags, **kwargs):
    taucoll = flags.Analysis.container_names.input.taus
    muoncoll = flags.Analysis.container_names.input.muons
    elecoll = flags.Analysis.container_names.input.electrons

    cfg = HHbbttTriggerDecoratorCfg(flags)

    cfg.addEventAlgo(
        CompFactory.HHBBTT.AntiTauDecoratorAlg(
            f"AntiTauDecor_{taucoll}",
            isMC=flags.Input.isMC,
            tauIDWP=flags.Analysis.Tau.ID,
            muonsIn=muoncoll,
            elesIn=elecoll,
            tausIn=taucoll,
            **kwargs
        )
    )

    return cfg
