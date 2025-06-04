from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory

from EasyjetHub.steering.utils.name_helper import drop_sys


def ditau_sequence(flags, configAcc):

    wps = [flags.Analysis.DiTau.ID]
    if 'extra_wps' in flags.Analysis.DiTau:
        for wp in flags.Analysis.DiTau.extra_wps:
            wps.append(wp)

    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    output_name = drop_sys(flags.Analysis.container_names.output.ditaus)

    configSeq += makeConfig('DiTauJets',)
    configSeq.setOptionValue('.containerName', output_name)

    for quality in wps:
        configSeq += makeConfig('DiTauJets.WorkingPoint')
        configSeq.setOptionValue('.containerName', output_name)
        configSeq.setOptionValue('.selectionName', quality)
        configSeq.setOptionValue('.quality', quality)

    # Kinematic selection
    configSeq += makeConfig('DiTauJets.PtEtaSelection', containerName=output_name,
                            selectionName='selectPtEta')
    configSeq.setOptionValue('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue('.minPt', 20e3)
    configSeq.setOptionValue('.maxEta', 2.5)

    # Add systematic object links
    configSeq += makeConfig('SystObjectLink', containerName=output_name)

    configSeq += makeConfig('Thinning', containerName=output_name)
    configSeq.setOptionValue('.selectionName', 'selectPtEta')

    return configSeq
