# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock

from EasyjetHub.steering.utils.name_helper import drop_sys


def HHbbttTriggerDecoratorCfg(flags, **kwargs):

    cfg = ComponentAccumulator()

    from EasyjetHub.algs.postprocessing.trigger_matching import TriggerMatchingToolCfg

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
            **kwargs
        )
    )

    return cfg


# Define ConfigBlock for AntiTauDecoratorAlg

class HHbbttAntiTauDecoratorBlock(ConfigBlock):

    def __init__(self, configName=''):
        super(HHbbttAntiTauDecoratorBlock, self).__init__()

        self.addOption('taus', '', type=str)
        self.addOption('muons', '', type=str)
        self.addOption('electrons', '', type=str)
        self.addOption('tauBaselineSelection', 'Baseline', type=str)
        self.addOption('tauIDSelection', '', type=str)

    def makeAlgs(self, config):

        alg = config.createAlgorithm('HHBBTT::AntiTauDecoratorAlg',
                                     'AntiTauDecor_' + self.taus)
        alg.taus, alg.tauBaselineSelection = config.readNameAndSelection(
            self.taus + '.' + self.tauBaselineSelection)
        alg.muons, alg.muonSelection = config.readNameAndSelection(self.muons)
        alg.electrons, alg.electronSelection = config.readNameAndSelection(
            self.electrons)
        alg.tauIDWP = self.tauIDSelection
        alg.IDTauSelection = 'isIDTau,as_char'
        alg.AntiTauSelection = 'isAntiTau,as_char'

        config.addOutputVar(drop_sys(self.taus), 'antiTauEventCategory',
                            'antiTauEventCategory')
        config.addSelection(self.taus, 'isIDTau', alg.IDTauSelection)
        config.addSelection(self.taus, 'isAntiTau', alg.AntiTauSelection)
