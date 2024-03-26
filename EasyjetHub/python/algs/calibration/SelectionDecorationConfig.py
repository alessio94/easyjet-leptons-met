# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence

from EasyjetHub.steering.utils.name_helper import drop_sys


class SelectionDecorationBlock (ConfigBlock):
    """the ConfigBlock to add selection decoration to a container"""

    def __init__(self, containers):
        super(SelectionDecorationBlock, self).__init__()
        self.containers = containers

    def makeAlgs(self, config):
        for container in self.containers:
            originContainerName = config.getOutputContainerOrigin(container)
            selectionNames = config.getSelectionNames(originContainerName)
            for selectionName in selectionNames:
                # skip default selection
                if selectionName == '':
                    continue
                alg = config.createAlgorithm(
                    'CP::AsgSelectionAlg',
                    f'SelectionDecoration_{originContainerName}_{selectionName}')
                selectionDecoration = f'baselineSelection_{selectionName}_%SYS%'
                alg.selectionDecoration = f'{selectionDecoration},as_char'
                alg.particles = config.readName(originContainerName)
                alg.preselection = config.getFullSelection(originContainerName,
                                                           selectionName)
                config.addOutputVar(originContainerName, selectionDecoration,
                                    selectionName)


def makeSelectionDecorationConfig(seq, containers):
    config = SelectionDecorationBlock(containers)
    seq.append(config)


def selection_decoration_sequence(flags):
    configSeq = ConfigSequence()

    containers = []
    for objtype in ["muons", "electrons", "photons", "taus"]:
        if flags.Analysis[f"do_{objtype}"]:
            containers += [drop_sys(flags.Analysis.container_names.output[objtype])]
    if flags.Analysis.do_small_R_jets:
        containers += [drop_sys(flags.Analysis.container_names.output[
            flags.Analysis.small_R_jet.jet_type])]
    if flags.Analysis.do_large_R_Topo_jets:
        containers += [drop_sys(flags.Analysis.container_names.output.reco10TopoJet)]
    if flags.Analysis.do_large_R_UFO_jets:
        containers += [drop_sys(flags.Analysis.container_names.output.reco10UFOJet)]

    makeSelectionDecorationConfig(configSeq, containers)

    return configSeq
