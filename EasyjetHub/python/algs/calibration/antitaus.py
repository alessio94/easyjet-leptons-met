# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock

from EasyjetHub.steering.utils.name_helper import drop_sys

# Define ConfigBlock for AntiTauDecoratorAlg


class HHbbttAntiTauDecoratorBlock(ConfigBlock):

    def __init__(self):
        super(HHbbttAntiTauDecoratorBlock, self).__init__()

        self.addOption('taus', '', type=str)
        self.addOption('muons', '', type=str)
        self.addOption('electrons', '', type=str)
        self.addOption('tauBaselineSelection', 'Baseline', type=str)
        self.addOption('tauIDSelection', '', type=str)
        self.addOption('antiTauScoreThreshold', 0, type=float)

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
        alg.antiTauScoreThreshold = self.antiTauScoreThreshold

        config.addOutputVar(drop_sys(self.taus), 'antiTauEventCategory',
                            'antiTauEventCategory')
        config.addSelection(self.taus, 'isIDTau', alg.IDTauSelection)
        config.addSelection(self.taus, 'isAntiTau', alg.AntiTauSelection)
