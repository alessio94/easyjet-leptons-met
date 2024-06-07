from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigFactory import ConfigFactory

from EasyjetHub.steering.utils.name_helper import drop_sys


def muon_sequence(flags, configAcc):

    wps = [(flags.Analysis.Muon.ID, flags.Analysis.Muon.Iso)]
    if 'extra_wps' in flags.Analysis.Muon:
        for wp in flags.Analysis.Muon.extra_wps:
            wps.append((wp[0], wp[1]))

    configSeq = ConfigSequence()
    config = ConfigFactory()
    makeConfig = config.makeConfig

    # Temporary hack, we should do this in a more systematic way
    # The config sequence will deal with the systematics suffix
    output_name = drop_sys(flags.Analysis.container_names.output.muons)
    configSeq += makeConfig('Muons', containerName=output_name)
    configSeq.setOptionValue('.maxEta', flags.Analysis.Muon.max_eta)

    # PID configuration
    for id, iso in wps:
        configSeq += makeConfig('Muons.WorkingPoint', containerName=output_name,
                                selectionName=id + '_' + iso)
        configSeq.setOptionValue('.quality', id)
        configSeq.setOptionValue('.isolation', iso)
        configSeq.setOptionValue('.trackSelection',
                                 flags.Analysis.Muon.trackSelection)
        configSeq.setOptionValue('.writeTrackD0Z0', True)
        configSeq.setOptionValue('.maxD0Significance',
                                 flags.Analysis.Muon.maxD0Significance)
        configSeq.setOptionValue('.maxDeltaZ0SinTheta',
                                 flags.Analysis.Muon.maxDeltaZ0SinTheta)

    # Kinematic selection
    configSeq += makeConfig('Muons.PtEtaSelection', containerName=output_name,
                            selectionName='selectPtEta')
    configSeq.setOptionValue('.selectionDecoration', 'selectPtEta')
    configSeq.setOptionValue('.minPt', flags.Analysis.Muon.min_pT)
    configSeq.setOptionValue('.maxEta', flags.Analysis.Muon.max_eta)

    # Add systematic object links
    configSeq += makeConfig('SystObjectLink', containerName=output_name)

    configSeq += makeConfig('Thinning', containerName=output_name)
    configSeq.setOptionValue('.selectionName', 'selectPtEta')

    for id, iso in wps:
        label = id + '_' + iso
        configSeq += makeConfig('Thinning', containerName=output_name,
                                configName=f'Thinning_{label}')
        configSeq.setOptionValue('.selectionName', label)
        configSeq.setOptionValue('.outputName', label + output_name)
        configSeq.setOptionValue('.postfix', label)

    return configSeq
