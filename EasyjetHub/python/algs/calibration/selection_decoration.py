# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory

from EasyjetHub.steering.utils.name_helper import drop_sys


def selection_decoration_sequence(flags):
    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    containers = []
    for objtype in ["muons", "electrons", "photons", "taus"]:
        if flags.Analysis[f"do_{objtype}"]:
            containers += [drop_sys(flags.Analysis.container_names.output[objtype])]
    if flags.Analysis.do_small_R_jets:
        containers += [drop_sys(flags.Analysis.container_names.output[
            flags.Analysis.Small_R_jet.jet_type])]
    if flags.Analysis.do_large_R_Topo_jets:
        containers += [drop_sys(flags.Analysis.container_names.output.reco10TopoJet)]
    if flags.Analysis.do_large_R_UFO_jets:
        containers += [drop_sys(flags.Analysis.container_names.output.reco10UFOJet)]

    configSeq += makeConfig('SelectionDecoration')
    configSeq.setOptionValue('.containers', containers)

    return configSeq
